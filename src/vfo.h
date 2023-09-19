/* Copyright (C)
* 2015 - John Melton, G0ORX/N6LYT
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

#ifndef _VFO_H
#define _VFO_H

#include "mode.h"

enum {
  VFO_A = 0,
  VFO_B,
  MAX_VFOS
};

struct _vfo {
  int band;
  int bandstack;
  long long frequency;
  int mode;
  int filter;
  int cwAudioPeakFilter;  // on/off

  int ctun;
  long long ctun_frequency;

  int rit_enabled;
  long long rit;

  long long lo;
  long long offset;

  char entered_frequency[16];  // need 13, but rounded up to next multiple of 4

};

extern struct _vfo vfo[MAX_VFOS];

//
// Store filter and NR settings on a per-mode basis
// all elements are "on/off"
//
struct _mode_settings {
  int filter;               // actual filter used
  int nb;                   // Noise blanker (0..2)
  int nr;                   // Noise reduction (0..2 or 0..4)
  int anf;                  // Automatic notch filter
  int snb;                  // Spectral noise blanker
  int en_txeq;              // TX equalizer on/off
  int en_rxeq;              // RX equalizer on/off
  int txeq[4];              // TX equalizer settings
  int rxeq[4];              // RX equalizer settings
  long long step;           // VFO step size
  int compressor;           // TX compressor on/off
  double compressor_level;  // TX compressor level
};

extern struct _mode_settings mode_settings[];

#define STEPS 15
extern char *step_labels[];

extern GtkWidget* vfo_init(int width, int height);
extern int  vfo_get_stepindex(void);
extern void vfo_set_step_from_index(int index);
extern void vfo_set_stepsize(int newstep);
extern int  vfo_get_step_from_index(int index);
extern void vfo_step(int steps);
extern void vfo_id_step(int id, int steps);
extern void vfo_move(long long hz, int round);
extern void vfo_id_move(int id, long long hz, int round);
extern void vfo_move_to(long long hz);
extern void vfo_update(void);

extern void vfoSaveState(void);
extern void vfoRestoreState(void);

extern void vfo_band_changed(int id, int b);
extern void vfo_bandstack_changed(int b);
extern void vfo_mode_changed(int m);
extern void vfo_filter_changed(int f);
extern void vfo_a_to_b(void);
extern void vfo_b_to_a(void);
extern void vfo_a_swap_b(void);

extern int get_tx_vfo(void);
extern int get_tx_mode(void);
extern long long get_tx_freq(void);

extern void vfo_xvtr_changed(void);

extern void vfo_rit_update(int rx);
extern void vfo_rit_clear(int rx);
extern void vfo_rit(int rx, int i);
extern void vfo_set_frequency(int vfo, long long f);

extern void vfo_ctun_update(int id, int state);

extern void num_pad(int val, int vfo);

#endif
