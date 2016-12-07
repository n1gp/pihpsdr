/* Copyright (C)
* 2015 - John Melton, G0ORX/N6LYT
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

#ifndef _BAND_H
#define _BAND_H

#include <gtk/gtk.h>
#include "bandstack.h"

#ifndef RTLSDR
#define band160 0
#define band80 1
#define band60 2
#define band40 3
#define band30 4
#define band20 5
#define band17 6
#define band15 7
#define band12 8
#define band10 9
#define band6 10
#endif
#if defined RTLSDR
#define band160 -1
#define band80 -1
#define band60 -1
#define band40 -1
#define band30 -1
#define band20 -1
#define band17 -1
#define band15 -1
#define bandGen -1
#define bandWWV -1
#define band136 -1
#define band472 -1
#define band12 0
#define band10 1
#define band6 2
#define band70 3
#define band144 4
#define band220 5
#define band430 6
#define band902 7
#define bandAIR 8
#define bandWX 9
#define BANDS 10
#elif defined LIMESDR
#define band70 11
#define band144 12
#define band220 13
#define band430 14
#define band902 15
#define band1240 16
#define band2300 17
#define band3400 18
#define bandAIR 19
#define bandWX 20
#define bandGen 21
#define bandWWV 22
#define band136 23
#define band472 24
#define BANDS 25
#else
#define bandGen 11
#define bandWWV 12
#define band136 13
#define band472 14
#define BANDS 15
#endif

#define XVTRS 8

/* --------------------------------------------------------------------------*/
/**
* @brief Band definition
*/
struct _BAND {
    char title[16];
    BANDSTACK *bandstack;
    unsigned char OCrx;
    unsigned char OCtx;
    int preamp;
    int alexRxAntenna;
    int alexTxAntenna;
    int alexAttenuation;
    double pa_calibration;
    long long frequencyMin;
    long long frequencyMax;
    long long frequencyLO;
    int disablePA;
};

typedef struct _BAND BAND;

int band;
gboolean displayHF;

int band_get_current();
BAND *band_get_current_band();
BAND *band_get_band(int b);
BAND *band_set_current(int b);

BANDSTACK_ENTRY *bandstack_entry_next();
BANDSTACK_ENTRY *bandstack_entry_previous();
BANDSTACK_ENTRY *bandstack_entry_get_current();

#endif
