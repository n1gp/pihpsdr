#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wdsp.h>

#include "audio.h"
#include "channel.h"
#include "discovered.h"
#include "rtl_protocol.h"
#include "radio.h"
#include "SoapySDR/Constants.h"
#include "SoapySDR/Device.h"

static double bandwidth=0.0;

static size_t receiver;
static SoapySDRDevice *rtl_device;
static SoapySDRStream *stream;
static int display_width;
static int buffer_size=BUFFER_SIZE;
static int outputsamples;
static int fft_size=4096;
static int dspRate=48000;
static int outputRate=48000;
static float *buffer;
static int max_samples;

static long long saved_frequency=0LL;
static int saved_antenna=-1;

static double iqinputbuffer[BUFFER_SIZE*2];
static double audiooutputbuffer[BUFFER_SIZE*2];
static int samples=0;

static pthread_t receive_thread_id;
static void *receive_thread(void* arg);

static void *resampler;
static int actual_rate;
static double resamples[1024*16];
static double resampled[1024*16];

static int running;

#define RTL_RATE 1536000

void rtl_protocol_init(int rx,int pixels) {
    SoapySDRKwargs args;
    int rc;

    fprintf(stderr,"rtl_protocol_init: receiver=%d pixels=%d\n",rx,pixels);

    receiver=(size_t)rx;
    display_width=pixels;

    switch(sample_rate) {
    case 48000:
        outputsamples=BUFFER_SIZE;
        break;
    case 96000:
        outputsamples=BUFFER_SIZE/2;
        break;
    case 192000:
        outputsamples=BUFFER_SIZE/4;
        break;
    case 384000:
        outputsamples=BUFFER_SIZE/8;
        break;
    case 768000:
        outputsamples=BUFFER_SIZE/16;
        break;
    case 1536000:
        outputsamples=BUFFER_SIZE/32;
        break;
    }

    args.size=0;

    // initialize the radio
    fprintf(stderr,"rtl_protocol: receive_thread: SoapySDRDevice_make\n");
    rtl_device=SoapySDRDevice_make(discovered->info.soapy.args);
    if(rtl_device==NULL) {
        fprintf(stderr,"rtl_protocol: SoapySDRDevice_make failed: %s\n",SoapySDRDevice_lastError());
        _exit(-1);
    }

    fprintf(stderr,"rtl_protocol: setting samplerate=%f\n",(double)RTL_RATE);
    rc=SoapySDRDevice_setSampleRate(rtl_device,SOAPY_SDR_RX,receiver,(double)RTL_RATE);
    if(rc!=0) {
        fprintf(stderr,"rtl_protocol: SoapySDRDevice_setSampleRate(%f) failed: %s\n",(double)RTL_RATE,SoapySDRDevice_lastError());
    }

    actual_rate=(int)SoapySDRDevice_getSampleRate(rtl_device, SOAPY_SDR_RX, receiver);
    fprintf(stderr,"rtl_protocol: actual samplerate= %d\n",actual_rate);

    fprintf(stderr,"rtl_protocol: setting bandwidth =%f\n",bandwidth);
    rc=SoapySDRDevice_setBandwidth(rtl_device,SOAPY_SDR_RX,receiver,bandwidth);
    if(rc!=0) {
        fprintf(stderr,"rtl_protocol: SoapySDRDevice_setBandwidth(%f) failed: %s\n",bandwidth,SoapySDRDevice_lastError());
    }

    if(saved_frequency!=0LL) {
        fprintf(stderr,"rtl_protocol: setting save_frequency: %lld\n",saved_frequency);
        rtl_protocol_set_frequency(saved_frequency);
    }

    fprintf(stderr,"setting Gain LNA=30.0\n");
    rc=SoapySDRDevice_setGainElement(rtl_device,SOAPY_SDR_RX,receiver,"LNA",30.0);
    if(rc!=0) {
        fprintf(stderr,"rtl_protocol: SoapySDRDevice_setGain LNA failed: %s\n",SoapySDRDevice_lastError());
    }
    fprintf(stderr,"setting Gain PGA=19.0\n");
    rc=SoapySDRDevice_setGainElement(rtl_device,SOAPY_SDR_RX,receiver,"PGA",19.0);
    if(rc!=0) {
        fprintf(stderr,"rtl_protocol: SoapySDRDevice_setGain PGA failed: %s\n",SoapySDRDevice_lastError());
    }
    fprintf(stderr,"setting Gain TIA=12.0\n");
    rc=SoapySDRDevice_setGainElement(rtl_device,SOAPY_SDR_RX,receiver,"TIA",12.0);
    if(rc!=0) {
        fprintf(stderr,"rtl_protocol: SoapySDRDevice_setGain TIA failed: %s\n",SoapySDRDevice_lastError());
    }

    fprintf(stderr,"rtl_protocol: receive_thread: SoapySDRDevice_setupStream\n");
    size_t channels=(size_t)receiver;
    rc=SoapySDRDevice_setupStream(rtl_device,&stream,SOAPY_SDR_RX,"CF32",&channels,1,&args);
    if(rc!=0) {
        fprintf(stderr,"rtl_protocol: SoapySDRDevice_setupStream failed: %s\n",SoapySDRDevice_lastError());
        _exit(-1);
    }

    max_samples=SoapySDRDevice_getStreamMTU(rtl_device,stream);
    fprintf(stderr,"max_samples=%d\n",max_samples);

    buffer=(float *)malloc(max_samples*sizeof(float)*2);

    if(actual_rate!=sample_rate) {
        fprintf(stderr,"rtl_protocol: creating resampler from %d to %d\n",actual_rate,sample_rate);
        resampler=create_resample (1, max_samples, resamples, resampled, actual_rate, sample_rate, 0.0, 0, 1.0);
    }

    rc=SoapySDRDevice_activateStream(rtl_device, stream, 0, 0LL, 0);
    if(rc!=0) {
        fprintf(stderr,"rtl_protocol: SoapySDRDevice_activateStream failed: %s\n",SoapySDRDevice_lastError());
        _exit(-1);
    }


    if(saved_frequency!=0LL) {
        fprintf(stderr,"rtl_protocol: setting save_frequency: %lld\n",saved_frequency);
        rtl_protocol_set_frequency(saved_frequency);
    }

    fprintf(stderr,"rtl_protocol_init: audio_open_output\n");
    if(audio_open_output()!=0) {
        local_audio=false;
        fprintf(stderr,"audio_open_output failed\n");
    }

    fprintf(stderr,"rtl_protocol_init: create receive_thread\n");
    rc=pthread_create(&receive_thread_id,NULL,receive_thread,NULL);
    if(rc != 0) {
        fprintf(stderr,"rtl_protocol: pthread_create failed on receive_thread: rc=%d\n", rc);
        _exit(-1);
    }
}

static void *receive_thread(void *arg) {
    float isample;
    float qsample;
    int outsamples;
    int elements;
    int flags=0;
    long long timeNs=0;
    long timeoutUs=10000L;
    int i,j;
    int leftaudiosample;
    int rightaudiosample;

    running=1;
    fprintf(stderr,"rtl_protocol: receive_thread\n");
    while(running) {
        elements=SoapySDRDevice_readStream(rtl_device,stream,(void *)&buffer,max_samples,&flags,&timeNs,timeoutUs);
//fprintf(stderr,"read %d elements\n",elements);
        if(actual_rate!=sample_rate) {
            for(i=0; i<elements; i++) {
                resamples[i*2]=(double)buffer[i*2];
                resamples[(i*2)+1]=(double)buffer[(i*2)+1];
            }

            outsamples=xresample(resampler);

            for(i=0; i<outsamples; i++) {
                iqinputbuffer[samples*2]=(double)resampled[i*2];
                iqinputbuffer[(samples*2)+1]=(double)resampled[(i*2)+1];
                samples++;
                if(samples==buffer_size) {
                    int error;
                    fexchange0(CHANNEL_RX0, iqinputbuffer, audiooutputbuffer, &error);
                    if(error!=0) {
                        fprintf(stderr,"fexchange0 (CHANNEL_RX0) returned error: %d\n", error);
                    }

                    if(local_audio) {
                        for(j=0; j<outputsamples; j++) {
                            leftaudiosample=(short)(audiooutputbuffer[j]*32767.0*volume);
                            rightaudiosample=(short)(audiooutputbuffer[j+1]*32767.0*volume);
                            audio_write(leftaudiosample, rightaudiosample);
                        }
                    }
                    Spectrum0(1, CHANNEL_RX0, 0, 0, iqinputbuffer);
                    samples=0;
                }
            }
        } else {
            for(i=0; i<elements; i++) {
                iqinputbuffer[samples*2]=(double)buffer[i*2];
                iqinputbuffer[(samples*2)+1]=(double)buffer[(i*2)+1];
                samples++;
                if(samples==buffer_size) {
                    int error;
                    fexchange0(CHANNEL_RX0, iqinputbuffer, audiooutputbuffer, &error);
                    if(error!=0) {
                        fprintf(stderr,"fexchange0 (CHANNEL_RX0) returned error: %d\n", error);
                    }

                    if(local_audio) {
                        for(j=0; j<outputsamples; j++) {
                            leftaudiosample=(short)(audiooutputbuffer[j]*32767.0*volume);
                            rightaudiosample=(short)(audiooutputbuffer[j+1]*32767.0*volume);
                            audio_write(leftaudiosample, rightaudiosample);
                        }
                    }
                    Spectrum0(1, CHANNEL_RX0, 0, 0, iqinputbuffer);
                    samples=0;
                }
            }
        }
    }

    SoapySDRDevice_deactivateStream(rtl_device, stream, 0, 0);
    fprintf(stderr,"rtl_protocol: receive_thread: SoapySDRDevice_closeStream\n");
    SoapySDRDevice_closeStream(rtl_device,stream);
    fprintf(stderr,"rtl_protocol: receive_thread: SoapySDRDevice_unmake\n");
    SoapySDRDevice_unmake(rtl_device);

}


void rtl_protocol_stop() {
    running=0;
    sleep(1);
    free(buffer);
    audio_close_output();
}

void rtl_protocol_set_frequency(long long f) {
    int rc;
    char *ant;

    if(rtl_device!=NULL) {
        SoapySDRKwargs args;
        args.size=0;
        fprintf(stderr,"rtl_protocol: setFrequency: %lld\n",f);
        //rc=SoapySDRDevice_setFrequencyComponent(rtl_device,SOAPY_SDR_RX,receiver,"RF",(double)f,&args);
        rc=SoapySDRDevice_setFrequency(rtl_device,SOAPY_SDR_RX,receiver,(double)f,&args);
        if(rc!=0) {
            fprintf(stderr,"rtl_protocol: SoapySDRDevice_setFrequency() failed: %s\n",SoapySDRDevice_lastError());
        }
    } else {
        fprintf(stderr,"rtl_protocol: setFrequency: %lld device is NULL\n",f);
        saved_frequency=f;
    }
}

void rtl_protocol_set_attenuation(int attenuation) {
    int rc;
    fprintf(stderr,"setting Gain LNA=%f\n",30.0-(double)attenuation);
    rc=SoapySDRDevice_setGainElement(rtl_device,SOAPY_SDR_RX,receiver,"LNA",30.0-(double)attenuation);
    if(rc!=0) {
        fprintf(stderr,"rtl_protocol: SoapySDRDevice_setGain LNA failed: %s\n",SoapySDRDevice_lastError());
    }
}
