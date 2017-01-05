#ifndef _BEEP_H
#define _BEEP_H

extern double beep_freq;
extern int beep_mute;

void beep_vol(long volume);
void beep_init();
void beep_close();

#endif
