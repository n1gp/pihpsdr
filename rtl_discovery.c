#include <stdio.h>
#include <string.h>
#include <SoapySDR/Device.h>
#include "discovered.h"
#include "rtl_discovery.h"

void rtl_discovery() {
  SoapySDRKwargs args;
  size_t length;
  int i;
  args.size=0;
  SoapySDRKwargs *devs=SoapySDRDevice_enumerate(&args, &length);

fprintf(stderr,"rtl_discovery: length=%ld devs->size=%ld\n",length,devs->size);

  for(i=0;i<devs->size;i++) {
fprintf(stderr,"rtl_discovery:device key=%s val=%s\n",devs->keys[i], devs->vals[i]);
    if(strcmp(devs->keys[i],"driver")==0 && strstr("rtlsdr", devs->vals[i])) {
      discovered[devices].protocol=RTLSDR_PROTOCOL;
      discovered[devices].device=RTLSDR_USB_DEVICE;
      strcpy(discovered[devices].name,devs->vals[i]);
      discovered[devices].status=STATE_AVAILABLE;
      discovered[devices].info.soapy.args=devs;
      devices++;
    }
  }

  fprintf(stderr,"rtl_discovery found %d devices\n",(int)length);
}
