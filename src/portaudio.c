/* Copyright (C)
* 2019 - Christoph van Wüllen, DL1YCF
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*
*/

#ifdef PORTAUDIO
//
// Alternate "audio" module using PORTAUDIO instead of ALSA
// (e.g. on MacOS)
//
// If PortAudio is NOT used, this file is empty, and audio.c
// is used instead.
//

#include <gtk/gtk.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <semaphore.h>

#include "radio.h"
#include "receiver.h"
#include "mode.h"
#include "portaudio.h"
#include "audio.h"
#include "message.h"
#include "vfo.h"

static PaStream *record_handle = NULL;


int n_input_devices;
AUDIO_DEVICE input_devices[MAX_AUDIO_DEVICES];
int n_output_devices;
AUDIO_DEVICE output_devices[MAX_AUDIO_DEVICES];

GMutex audio_mutex;
int n_input_devices = 0;
int n_output_devices = 0;

//
// We now use callback functions to provide the "headphone" audio data,
// and therefore can control the latency.
// RX audio samples are put into a ring buffer and "fetched" therefreom
// by the portaudio "headphone" callback.
//
// We choose a ring buffer of 9600 (stereo) samples that is kept about half-full
// during RX (latency: 0.1 sec) which should be more than enough.
// If the buffer falls below 1800, half a buffer length of silence is
// inserted. This usually only happens after TX/RX transitions
//
// If we go TX in CW mode, cw_audio_write() is called. If it is called for
// the first time with a non-zero sidetone volume,
// the ring buffer is cleared and only 256 (stereo) samples of silence
// are put into it. During the TX phase, the buffer filling remains low
// which we need for small CW sidetone latencies. If we then go to RX again
// a "low water mark" condition is detected in the first call to audio_write()
// and half a buffer length of silence is inserted again.
//
// Experiments indicate that we can indeed keep the ring buffer about half full
// during RX and quite empty during CW-TX.
//
// If the sidetone volume is zero, the audio buffers are left unchanged
//

#define MY_AUDIO_BUFFER_SIZE 256
#define MY_RING_BUFFER_SIZE  9600
#define MY_RING_LOW_WATER    1000
#define MY_RING_HIGH_WATER   8600
#define MY_CW_LOW_WATER      512
#define MY_CW_HIGH_WATER     768
#define MY_CW_MID_WATER      640

//
// Ring buffer for "local microphone" samples stored locally here.
// NOTE: lead large buffer for some "loopback" devices which produce
//       samples in large chunks if fed from digimode programs.
//
float  *mic_ring_buffer = NULL;
int     mic_ring_outpt = 0;
int     mic_ring_inpt = 0;

//
// AUDIO_GET_CARDS
//
// This inits PortAudio and looks for suitable input and output channels
//
void audio_get_cards() {
  int numDevices;
  PaStreamParameters inputParameters, outputParameters;
  PaError err;
  g_mutex_init(&audio_mutex);
  err = Pa_Initialize();

  if ( err != paNoError ) {
    t_print("%s: init error %s\n", __FUNCTION__, Pa_GetErrorText(err));
    return;
  }

  numDevices = Pa_GetDeviceCount();

  if ( numDevices < 0 ) { return; }

  n_input_devices = 0;
  n_output_devices = 0;

  for (int  i = 0; i < numDevices; i++ ) {
    const PaDeviceInfo *deviceInfo = Pa_GetDeviceInfo( i );
    inputParameters.device = i;
    inputParameters.channelCount = 1;  // Microphone samples are mono
    inputParameters.sampleFormat = paFloat32;
    inputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
    inputParameters.hostApiSpecificStreamInfo = NULL;

    if (Pa_IsFormatSupported(&inputParameters, NULL, 48000.0) == paFormatIsSupported) {
      if (n_input_devices < MAX_AUDIO_DEVICES) {
        //
        // probably not necessary with portaudio, but to be on the safe side,
        // we copy the device name to local storage. This is referenced both
        // by the name and description element.
        //
        input_devices[n_input_devices].name = input_devices[n_input_devices].description = g_new0(char,
                                              strlen(deviceInfo->name) + 1);
        strcpy(input_devices[n_input_devices].name, deviceInfo->name);
        input_devices[n_input_devices].index = i;
        n_input_devices++;
      }

      t_print("%s: INPUT DEVICE, No=%d, Name=%s\n", __FUNCTION__, i, deviceInfo->name);
    }

    outputParameters.device = i;
    outputParameters.channelCount = 2;  // audio output samples are stereo
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = 0; /* ignored by Pa_IsFormatSupported() */
    outputParameters.hostApiSpecificStreamInfo = NULL;

    if (Pa_IsFormatSupported(NULL, &outputParameters, 48000.0) == paFormatIsSupported) {
      if (n_output_devices < MAX_AUDIO_DEVICES) {
        output_devices[n_output_devices].name = output_devices[n_output_devices].description = g_new0(char,
                                                strlen(deviceInfo->name) + 1);
        strcpy(output_devices[n_output_devices].name, deviceInfo->name);
        output_devices[n_output_devices].index = i;
        n_output_devices++;
      }

      t_print("%s: OUTPUT DEVICE, No=%d, Name=%s\n", __FUNCTION__, i, deviceInfo->name);
    }
  }
}

//
// AUDIO_OPEN_INPUT
//
// open a PA stream that connects to the TX microphone
// The PA callback function then sends the data to the transmitter
//

int pa_mic_cb(const void*, void*, unsigned long, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);
int pa_out_cb(const void*, void*, unsigned long, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void*);

int audio_open_input() {
  PaError err;
  PaStreamParameters inputParameters;
  int i;
  int padev;

  if (!can_transmit) {
    return -1;
  }

  if (transmitter->microphone_name == NULL) {
    transmitter->local_microphone = 0;
    return -1;
  }

  //
  // Look up device name and determine device ID
  //
  padev = -1;

  for (i = 0; i < n_input_devices; i++) {
    if (!strcmp(transmitter->microphone_name, input_devices[i].name)) {
      padev = input_devices[i].index;
      break;
    }
  }

  t_print("%s: name=%s PADEV=%d\n", __FUNCTION__, transmitter->microphone_name, padev);

  //
  // Device name possibly came from props file and device is no longer there
  //
  if (padev < 0) {
    return -1;
  }

  g_mutex_lock(&audio_mutex);
  bzero( &inputParameters, sizeof( inputParameters ) ); //not necessary if you are filling in all the fields
  inputParameters.channelCount = 1;   // MONO
  inputParameters.device = padev;
  inputParameters.hostApiSpecificStreamInfo = NULL;
  inputParameters.sampleFormat = paFloat32;
  inputParameters.suggestedLatency = Pa_GetDeviceInfo(padev)->defaultLowInputLatency ;
  inputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
  err = Pa_OpenStream(&record_handle, &inputParameters, NULL, 48000.0, MY_AUDIO_BUFFER_SIZE,
                      paNoFlag, pa_mic_cb, NULL);

  if (err != paNoError) {
    t_print("%s: open stream error %s\n", __FUNCTION__, Pa_GetErrorText(err));
    record_handle = NULL;
    g_mutex_unlock(&audio_mutex);
    return -1;
  }

  mic_ring_buffer = (float *) g_new(float, MY_RING_BUFFER_SIZE);
  mic_ring_outpt = mic_ring_inpt = 0;

  if (mic_ring_buffer == NULL) {
    Pa_CloseStream(record_handle);
    record_handle = NULL;
    t_print("%s: alloc buffer failed.\n", __FUNCTION__);
    g_mutex_unlock(&audio_mutex);
    return -1;
  }

  err = Pa_StartStream(record_handle);

  if (err != paNoError) {
    t_print("%s: start stream error %s\n", __FUNCTION__, Pa_GetErrorText(err));
    Pa_CloseStream(record_handle);
    record_handle = NULL;
    g_free(mic_ring_buffer);
    mic_ring_buffer = NULL;
    g_mutex_unlock(&audio_mutex);
    return -1;
  }

  //
  // Finished!
  //
  g_mutex_unlock(&audio_mutex);
  return 0;
}

//
// PortAudio call-back function for Audio output
//
int pa_out_cb(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
              const PaStreamCallbackTimeInfo* timeInfo,
              PaStreamCallbackFlags statusFlags,
              void *userdata) {
  float *out = (float *)outputBuffer;
  RECEIVER *rx = (RECEIVER *)userdata;

  if (out == NULL) {
    t_print("%s: bogus audio buffer in callback\n", __FUNCTION__);
    return paContinue;
  }

  g_mutex_lock(&rx->local_audio_mutex);

  if (rx->local_audio_buffer != NULL) {
    //
    // Mutex protection: if the buffer is non-NULL it cannot vanish
    // util callback is completed
    //
    int newpt = rx->local_audio_buffer_outpt;

    for (int i = 0; i < framesPerBuffer; i++) {
      if (rx->local_audio_buffer_inpt == newpt) {
        // Ring buffer empty, send zero sample
        *out++ = 0.0;
        *out++ = 0.0;
      } else {
        *out++ = rx->local_audio_buffer[2 * newpt];
        *out++ = rx->local_audio_buffer[2 * newpt + 1];
        newpt++;

        if (newpt >= MY_RING_BUFFER_SIZE) { newpt = 0; }

        rx->local_audio_buffer_outpt = newpt;
      }
    }
  }

  g_mutex_unlock(&rx->local_audio_mutex);
  return paContinue;
}

//
// PortAudio call-back function for Audio input
//
int pa_mic_cb(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
              const PaStreamCallbackTimeInfo* timeInfo,
              PaStreamCallbackFlags statusFlags,
              void *userdata) {
  const float *in = (float *)inputBuffer;

  if (in == NULL) {
    // This should not happen, so we do not send silence etc.
    t_print("%s: bogus audio buffer in callback\n", __FUNCTION__);
    return paContinue;
  }

  g_mutex_lock(&audio_mutex);

  if (mic_ring_buffer != NULL) {
    static int last_was_tx = 0;

    //
    // mutex protected: ring buffer cannot vanish
    //
    // Normally there is a slight mis-match between the 48kHz sample
    // rate of the "microphone device" and the 48kHz rate of the
    // HPSDR device. Thus, the mic buffer tends to either slowly
    // drain or slowly become full (which leads to large TX delays).
    //
    // The TX/RX transition seems to be the best moment to "reset"
    // the mic input buffer, and fill it with a little bit (20 msec)
    // of silence and the current batch of mic samples. During normal
    // RX operation, one cannot fiddle around with the mic samples since
    // VOX might be active.
    //
    // The (static) variable last_was_tx is used to "detect" the
    // TX/RX transition.
    //
    //
    if (!isTransmitting()) {
      if (last_was_tx) {
        last_was_tx = 0;
        mic_ring_outpt = 0;
        mic_ring_inpt  = 960;
        bzero(mic_ring_buffer, 960 * sizeof(float));
      }
    } else {
      last_was_tx = 1;
    }

    for (int i = 0; i < framesPerBuffer; i++) {
      //
      // put sample into ring buffer
      //
      int newpt = mic_ring_inpt + 1;

      if (newpt == MY_RING_BUFFER_SIZE) { newpt = 0; }

      if (newpt != mic_ring_outpt) {
        // buffer space available, do the write
        mic_ring_buffer[mic_ring_inpt] = in[i];
        // atomic update of mic_ring_inpt
        mic_ring_inpt = newpt;
      }
    }
  }

  // print mic input buffer water mark for debugging
  // i=mic_ring_inpt - mic_ring_outpt;
  // if (mic_ring_inpt < mic_ring_outpt) i +=MY_RING_BUFFER_SIZE;
  // t_print("MIC IN BUF=%d\n", i);
  g_mutex_unlock(&audio_mutex);
  return paContinue;
}

//
// Utility function for retrieving mic samples
// from ring buffer
//
float audio_get_next_mic_sample() {
  float sample;
  g_mutex_lock(&audio_mutex);

  //
  // mutex protected (for every single sample!):
  // ring buffer cannot vanish while being processed here
  //
  if ((mic_ring_buffer == NULL) || (mic_ring_outpt == mic_ring_inpt)) {
    // no buffer, or nothing in buffer: insert silence
    sample = 0.0;
  } else {
    int newpt = mic_ring_outpt + 1;

    if (newpt == MY_RING_BUFFER_SIZE) { newpt = 0; }

    sample = mic_ring_buffer[mic_ring_outpt];
    // atomic update of read pointer
    mic_ring_outpt = newpt;
  }

  g_mutex_unlock(&audio_mutex);
  return sample;
}

//
// AUDIO_OPEN_OUTPUT
//
// open a PA stream for data from one of the RX
//
int audio_open_output(RECEIVER *rx) {
  PaError err;
  PaStreamParameters outputParameters;
  int padev;
  int i;

  if (rx->audio_name == NULL) {
    rx->local_audio = 0;
    return -1;
  }

  //
  // Look up device name and determine device ID
  //
  padev = -1;

  for (i = 0; i < n_output_devices; i++) {
    if (!strcmp(rx->audio_name, output_devices[i].name)) {
      padev = output_devices[i].index;
      break;
    }
  }

  t_print("%s: name=%s PADEV=%d\n", __FUNCTION__, rx->audio_name, padev);

  //
  // Device name possibly came from props file and device is no longer there
  //
  if (padev < 0) {
    return -1;
  }

  g_mutex_lock(&rx->local_audio_mutex);
  bzero( &outputParameters, sizeof( outputParameters ) ); //not necessary if you are filling in all the fields
  outputParameters.channelCount = 2;   // audio output is stereo
  outputParameters.device = padev;
  outputParameters.hostApiSpecificStreamInfo = NULL;
  outputParameters.sampleFormat = paFloat32;
  // use a zero for the latency to get the minimum value
  outputParameters.suggestedLatency = 0.0; //Pa_GetDeviceInfo(padev)->defaultLowOutputLatency ;
  outputParameters.hostApiSpecificStreamInfo = NULL; //See you specific host's API docs for info on using this field
  err = Pa_OpenStream(&(rx->playstream), NULL, &outputParameters, 48000.0, MY_AUDIO_BUFFER_SIZE,
                      paNoFlag, pa_out_cb, rx);

  if (err != paNoError) {
    t_print("%s: open stream error %s\n", __FUNCTION__, Pa_GetErrorText(err));
    rx->playstream = NULL;
    g_mutex_unlock(&rx->local_audio_mutex);
    return -1;
  }

  //
  // This is now a ring buffer much larger than a single audio buffer
  //
  rx->local_audio_buffer = g_new(float, 2 * MY_RING_BUFFER_SIZE);
  rx->local_audio_buffer_inpt = 0;
  rx->local_audio_buffer_outpt = 0;

  if (rx->local_audio_buffer == NULL) {
    t_print("%s: allocate buffer failed\n", __FUNCTION__);
    Pa_CloseStream(rx->playstream);
    rx->playstream = NULL;
    g_mutex_unlock(&rx->local_audio_mutex);
    return -1;
  }

  err = Pa_StartStream(rx->playstream);

  if (err != paNoError) {
    t_print("%s: error starting stream:%s\n", __FUNCTION__, Pa_GetErrorText(err));
    Pa_CloseStream(rx->playstream);
    rx->playstream = NULL;
    g_free(rx->local_audio_buffer);
    rx->local_audio_buffer = NULL;
    g_mutex_unlock(&rx->local_audio_mutex);
    return -1;
  }

  //
  // Finished!
  //
  g_mutex_unlock(&rx->local_audio_mutex);
  return 0;
}

//
// AUDIO_CLOSE_INPUT
//
// close a TX microphone stream
//
void audio_close_input() {
  t_print("%s: micname=%s\n", __FUNCTION__, transmitter->microphone_name);
  g_mutex_lock(&audio_mutex);

  if (record_handle != NULL) {
    PaError err = Pa_StopStream(record_handle);

    if (err != paNoError) {
      t_print("%s: error stopping stream: %s\n", __FUNCTION__, Pa_GetErrorText(err));
    }

    err = Pa_CloseStream(record_handle);

    if (err != paNoError) {
      t_print("%s: %s\n", __FUNCTION__, Pa_GetErrorText(err));
    }

    record_handle = NULL;
  }

  if (mic_ring_buffer != NULL) {
    g_free(mic_ring_buffer);
  }

  g_mutex_unlock(&audio_mutex);
}

//
// AUDIO_CLOSE_OUTPUT
//
// shut down the stream connected with audio from one of the RX
//
void audio_close_output(RECEIVER *rx) {
  t_print("%s: device=%s\n", __FUNCTION__, rx->audio_name);
  g_mutex_lock(&rx->local_audio_mutex);

  if (rx->local_audio_buffer != NULL) {
    g_free(rx->local_audio_buffer);
    rx->local_audio_buffer = NULL;
  }

  if (rx->playstream != NULL) {
    PaError err = Pa_StopStream(rx->playstream);

    if (err != paNoError) {
      t_print("%s: stop stream error %s\n", __FUNCTION__, Pa_GetErrorText(err));
    }

    err = Pa_CloseStream(rx->playstream);

    if (err != paNoError) {
      t_print("%s: close stream error %s\n", __FUNCTION__, Pa_GetErrorText(err));
    }

    rx->playstream = NULL;
  }

  g_mutex_unlock(&rx->local_audio_mutex);
}

//
// AUDIO_WRITE
//
// send RX audio data to a PA output stream
// we have to store the data such that the PA callback function
// can access it.
//
// Note that the check on isTransmitting() takes care that "blocking"
// by the mutex can only occur in the moment of a RX/TX transition if
// both audio_write() and cw_audio_write() get a "go".
//
// So mutex locking/unlocking should only cost few CPU cycles in
// normal operation.
//
int audio_write (RECEIVER *rx, float left, float right) {
  int txmode = get_tx_mode();
  float *buffer = rx->local_audio_buffer;

  if (rx == active_receiver && isTransmitting() && (txmode == modeCWU || txmode == modeCWL)
      && cw_keyer_sidetone_volume > 0) {
    //
    // If a CW side tone may occur, quickly return
    //
    return 0;
  }

  g_mutex_lock(&rx->local_audio_mutex);

  if (rx->playstream != NULL && buffer != NULL) {
    int avail = rx->local_audio_buffer_inpt - rx->local_audio_buffer_outpt;

    if (avail < 0) { avail += MY_RING_BUFFER_SIZE; }

    if (avail <  MY_RING_LOW_WATER) {
      //
      // Running the RX-audio for a very long time
      // and with audio hardware whose "48000 Hz" are a little faster than the "48000 Hz" of
      // the SDR will very slowly drain the buffer. We recover from this by brutally
      // inserting half a buffer's length of silence.
      //
      // This is not always an "error" to be reported and necessarily happens in three cases:
      //  a) we come here for the first time
      //  b) we come from a TX/RX transition in non-CW mode, and no duplex
      //  c) we come from a TX/RX transition in CW mode
      //
      // In case a) and b) the buffer will be empty, in c) the buffer will contain "few" samples
      // because of the "CW audio low latency" strategy.
      //
      int oldpt = rx->local_audio_buffer_inpt;

      for (int i = 0; i < MY_RING_BUFFER_SIZE / 2 - avail; i++) {
        buffer[2 * oldpt] = 0.0;
        buffer[2 * oldpt + 1] = 0.0;
        oldpt++;

        if (oldpt >= MY_RING_BUFFER_SIZE) { oldpt = 0; }
      }

      rx->local_audio_buffer_inpt = oldpt;
      //t_print("%s: buffer was nearly empty, inserted silence.\n", __FUNCTION__);
    }

    if (avail > MY_RING_HIGH_WATER) {
      //
      // Running the RX-audio for a very long time
      // and with audio hardware whose "48000 Hz" are a little slower than the "48000 Hz" of
      // the SDR will very slowly fill the buffer. This should be the only situation where
      // this "buffer overrun" condition should occur. We recover from this by brutally
      // deleting half a buffer size of audio, such that the next overrun is in the distant
      // future.
      //
      int oldpt = rx->local_audio_buffer_inpt - avail + MY_RING_BUFFER_SIZE / 2;

      if (oldpt < 0) { oldpt += MY_RING_BUFFER_SIZE; }

      rx->local_audio_buffer_inpt = oldpt;
      t_print("%s: buffer was nearly full, deleted audio\n", __FUNCTION__);
    }

    //
    // put sample into ring buffer
    //
    int oldpt = rx->local_audio_buffer_inpt;
    int newpt = oldpt + 1;

    if (newpt == MY_RING_BUFFER_SIZE) { newpt = 0; }

    if (newpt != rx->local_audio_buffer_outpt) {
      //
      // buffer space available
      //
      buffer[2 * oldpt] = left;
      buffer[2 * oldpt + 1] = right;
      rx->local_audio_buffer_inpt = newpt;
    }
  }

  g_mutex_unlock(&rx->local_audio_mutex);
  return 0;
}

//
// During CW, between the elements the side tone contains "true" silence.
// We detect a sequence of 16 subsequent zero samples, and insert or delete
// a zero sample depending on the buffer water mark:
// If there are more than two portaudio buffers available, delete one sample,
// if it drops down to less than one portaudio buffer, insert one sample
//
// Thus we have an active latency management.
//
int cw_audio_write(RECEIVER *rx, float sample) {
  g_mutex_lock(&rx->local_audio_mutex);

  if (rx->playstream != NULL && rx->local_audio_buffer != NULL) {
    static int count = 0;
    int oldpt, newpt;
    int avail = rx->local_audio_buffer_inpt - rx->local_audio_buffer_outpt;
    int adjust = 0;

    if (avail < 0) { avail += MY_RING_BUFFER_SIZE; }

    if (avail >  MY_RING_LOW_WATER) {
      //
      // First time producing CW audio after RX/TX transition:
      // empty audio buffer and insert *a little bit of* silence
      //
      bzero(rx->local_audio_buffer, 2 * MY_CW_MID_WATER * sizeof(float));
      rx->local_audio_buffer_inpt = MY_CW_MID_WATER;
      rx->local_audio_buffer_outpt = 0;
      avail = MY_CW_MID_WATER;
      count = 0;
    }

    if (sample != 0.0) { count = 0; }

    if (++count >= 16) {
      count = 0;

      //
      // We arrive here if we have seen 16 zero samples in a row.
      // First look how many samples there are in the ring buffer
      //
      if (avail > MY_CW_HIGH_WATER) { adjust = 2; } // too full: skip one sample

      if (avail < MY_CW_LOW_WATER ) { adjust = 1; } // too empty: insert one sample
    }

    switch (adjust) {
    case 0:
      //
      // default case: put sample into ring buffer
      //
      oldpt = rx->local_audio_buffer_inpt;
      newpt = oldpt + 1;

      if (newpt == MY_RING_BUFFER_SIZE) { newpt = 0; }

      if (newpt != rx->local_audio_buffer_outpt) {
        //
        // buffer space available
        //
        rx->local_audio_buffer[2 * oldpt] = sample;
        rx->local_audio_buffer[2 * oldpt + 1] = -sample;
        rx->local_audio_buffer_inpt = newpt;
      }

      break;

    case 1:
      //
      // buffer becomes too empty, and we just saw 16 samples of silence:
      // insert two samples of silence. No check on "buffer full" necessary.
      //
      oldpt = rx->local_audio_buffer_inpt;
      rx->local_audio_buffer[2 * oldpt] = 0.0;
      rx->local_audio_buffer[2 * oldpt + 1] = 0.0;
      oldpt++;

      if (oldpt == MY_RING_BUFFER_SIZE) { oldpt = 0; }

      rx->local_audio_buffer[2 * oldpt] = 0.0;
      rx->local_audio_buffer[2 * oldpt + 1] = 0.0;
      oldpt++;

      if (oldpt == MY_RING_BUFFER_SIZE) { oldpt = 0; }

      rx->local_audio_buffer_inpt = oldpt;
      break;

    case 2:
      //
      // buffer becomes too full, and we just saw
      // 16 samples of silence: just skip the last "silent" sample
      //
      break;
    }
  }

  g_mutex_unlock(&rx->local_audio_mutex);
  return 0;
}

#endif
