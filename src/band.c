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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bandstack.h"
#include "band.h"
#include "filter.h"
#include "mode.h"
#include "alex.h"
#include "property.h"
#include "radio.h"
#include "vfo.h"

#define LINESDR

static int current_band = band20;
int xvtr_band = BANDS;

char* outOfBand = "Out of band";
int info_band;

/* --------------------------------------------------------------------------*/
/**
* @brief bandstack
*/
/* ----------------------------------------------------------------------------*/

//
// The "CTUN" flag and the ctun frequency have been added to all band-stacks.
// By default, ctun is off and the ctun_frequency is zero.
//
// The var1/2 low/high frequencies were removed because they were nowhere used.
//
BANDSTACK_ENTRY bandstack_entries136[] = {
  {135800LL, 0, 0LL, modeCWL, filterF6, 0, 0},
  {137100LL, 0, 0LL, modeCWL, filterF6, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries472[] = {
  {472100LL, 0, 0LL, modeCWL, filterF6, 0, 0},
  {475100LL, 0, 0LL, modeCWL, filterF6, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries160[] = {
  {1810000LL, 0, 0LL, modeCWL, filterF6, 0, 0},
  {1835000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {1845000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries80[] = {
  {3501000LL, 0, 0LL, modeCWL, filterF6, 0, 0},
  {3751000LL, 0, 0LL, modeLSB, filterF5, 0, 0},
  {3850000LL, 0, 0LL, modeLSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries60_WRC15[] = {
  {5352750LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {5357000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {5363000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries60_OTHER[] = {
  {5332000LL, 0, 0LL, modeUSB, filterF5, 0, 0}, // default channels for
  {5348000LL, 0, 0LL, modeUSB, filterF5, 0, 0}, // 60m band, US regulations
  {5358500LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {5373000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {5405000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries60_UK[] = {
  {5261250LL, 0, 0LL, modeUSB, filterF5, 0, 0}, // default channels for
  {5280000LL, 0, 0LL, modeUSB, filterF5, 0, 0}, // 60m band, UK regulations
  {5290250LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {5302500LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {5318000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {5335500LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {5356000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {5368250LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {5380000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {5398250LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {5405000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries40[] = {
  {7001000LL, 0, 0LL, modeCWL, filterF6, 0, 0},
  {7152000LL, 0, 0LL, modeLSB, filterF5, 0, 0},
  {7255000LL, 0, 0LL, modeLSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries30[] = {
  {10120000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {10130000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {10140000LL, 0, 0LL, modeCWU, filterF6, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries20[] = {
  {14010000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {14150000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {14230000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {14336000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries17[] = {
  {18068600LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {18125000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {18140000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries15[] = {
  {21001000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {21255000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {21300000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries12[] = {
  {24895000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {24900000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {24910000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries10[] = {
  {28010000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {28300000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {28400000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries6[] = {
  {50010000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {50125000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {50200000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries70[] = {
  {70010000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {70200000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {70250000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries144[] = {
  {144010000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {144200000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {144250000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {145600000LL, 0, 0LL, modeFMN, filterF1, 0, 0},
  {145725000LL, 0, 0LL, modeFMN, filterF1, 0, 0},
  {145900000LL, 0, 0LL, modeFMN, filterF1, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries220[] = {
  {220010000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {220200000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {220250000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries430[] = {
  {430010000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {432100000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {432300000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries902[] = {
  {902010000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {902100000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {902300000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries1240[] = {
  {1240010000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {1240100000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {1240300000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries2300[] = {
  {2300010000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {2300100000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {2300300000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entries3400[] = {
  {3400010000LL, 0, 0LL, modeCWU, filterF6, 0, 0},
  {3400100000LL, 0, 0LL, modeUSB, filterF5, 0, 0},
  {3400300000LL, 0, 0LL, modeUSB, filterF5, 0, 0}
};

BANDSTACK_ENTRY bandstack_entriesAIR[] = {
  {118800000LL, 0, 0LL, modeAM, filterF3, 0, 0},
  {120000000LL, 0, 0LL, modeAM, filterF3, 0, 0},
  {121700000LL, 0, 0LL, modeAM, filterF3, 0, 0},
  {124100000LL, 0, 0LL, modeAM, filterF3, 0, 0},
  {126600000LL, 0, 0LL, modeAM, filterF3, 0, 0},
  {136500000LL, 0, 0LL, modeAM, filterF3, 0, 0}
};

BANDSTACK_ENTRY bandstack_entriesGEN[] = {
  {909000LL,    0, 0LL, modeAM, filterF3, 0, 0},
  {5975000LL,   0, 0LL, modeAM, filterF3, 0, 0},
  {13845000LL,  0, 0LL, modeAM, filterF3, 0, 0}
};

BANDSTACK_ENTRY bandstack_entriesWWV[] = {
  {2500000LL,   0, 0LL, modeSAM, filterF3, 0, 0},
  {5000000LL,   0, 0LL, modeSAM, filterF3, 0, 0},
  {10000000LL,  0, 0LL, modeSAM, filterF3, 0, 0},
  {15000000LL,  0, 0LL, modeSAM, filterF3, 0, 0},
  {20000000LL,  0, 0LL, modeSAM, filterF3, 0, 0}
};

BANDSTACK bandstack160 = {3, 1, bandstack_entries160};
BANDSTACK bandstack80 = {3, 1, bandstack_entries80};
BANDSTACK bandstack60 = {5, 1, bandstack_entries60_OTHER};
BANDSTACK bandstack40 = {3, 1, bandstack_entries40};
BANDSTACK bandstack30 = {3, 1, bandstack_entries30};
BANDSTACK bandstack20 = {4, 1, bandstack_entries20};
BANDSTACK bandstack17 = {3, 1, bandstack_entries17};
BANDSTACK bandstack15 = {3, 1, bandstack_entries15};
BANDSTACK bandstack12 = {3, 1, bandstack_entries12};
BANDSTACK bandstack10 = {3, 1, bandstack_entries10};
BANDSTACK bandstack6 = {3, 1, bandstack_entries6};
BANDSTACK bandstack70 = {3, 1, bandstack_entries70};
BANDSTACK bandstack144 = {6, 1, bandstack_entries144};
BANDSTACK bandstack220 = {3, 1, bandstack_entries220};
BANDSTACK bandstack430 = {3, 1, bandstack_entries430};
BANDSTACK bandstack902 = {3, 1, bandstack_entries902};
BANDSTACK bandstack1240 = {3, 1, bandstack_entries1240};
BANDSTACK bandstack2300 = {3, 1, bandstack_entries2300};
BANDSTACK bandstack3400 = {3, 1, bandstack_entries3400};
BANDSTACK bandstackAIR = {6, 1, bandstack_entriesAIR};
BANDSTACK bandstackGEN = {3, 1, bandstack_entriesGEN};
BANDSTACK bandstackWWV = {5, 1, bandstack_entriesWWV};
BANDSTACK bandstack136 = {2, 0, bandstack_entries136};
BANDSTACK bandstack472 = {2, 0, bandstack_entries472};

/* --------------------------------------------------------------------------*/
/**
* @brief xvtr
*/
/* ----------------------------------------------------------------------------*/

BANDSTACK_ENTRY bandstack_entries_xvtr_0[] = {
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0}
};
BANDSTACK_ENTRY bandstack_entries_xvtr_1[] = {
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0}
};
BANDSTACK_ENTRY bandstack_entries_xvtr_2[] = {
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0}
};
BANDSTACK_ENTRY bandstack_entries_xvtr_3[] = {
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0}
};
BANDSTACK_ENTRY bandstack_entries_xvtr_4[] = {
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0}
};
BANDSTACK_ENTRY bandstack_entries_xvtr_5[] = {
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0}
};
BANDSTACK_ENTRY bandstack_entries_xvtr_6[] = {
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0}
};
BANDSTACK_ENTRY bandstack_entries_xvtr_7[] = {
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0},
  {0LL, 0, 0LL, modeUSB, filterF6, 0, 0}
};

BANDSTACK bandstack_xvtr_0 = {3, 0, bandstack_entries_xvtr_0};
BANDSTACK bandstack_xvtr_1 = {3, 0, bandstack_entries_xvtr_1};
BANDSTACK bandstack_xvtr_2 = {3, 0, bandstack_entries_xvtr_2};
BANDSTACK bandstack_xvtr_3 = {3, 0, bandstack_entries_xvtr_3};
BANDSTACK bandstack_xvtr_4 = {3, 0, bandstack_entries_xvtr_4};
BANDSTACK bandstack_xvtr_5 = {3, 0, bandstack_entries_xvtr_5};
BANDSTACK bandstack_xvtr_6 = {3, 0, bandstack_entries_xvtr_6};
BANDSTACK bandstack_xvtr_7 = {3, 0, bandstack_entries_xvtr_7};



BAND bands[BANDS + XVTRS] = {
  {"136kHz", &bandstack136, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 135700LL, 137800LL, 0LL, 0LL, 0},
  {"472kHz", &bandstack472, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 472000LL, 479000LL, 0LL, 0LL, 0},
  {"160", &bandstack160, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 1800000LL, 2000000LL, 0LL, 0LL, 0},
  {"80", &bandstack80, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 3500000LL, 4000000LL, 0LL, 0LL, 0},
  {"60", &bandstack60, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 5250000LL, 5450000LL, 0LL, 0LL, 0},
  {"40", &bandstack40, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 7000000LL, 7300000LL, 0LL, 0LL, 0},
  {"30", &bandstack30, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 10100000LL, 10150000LL, 0LL, 0LL, 0},
  {"20", &bandstack20, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 14000000LL, 14350000LL, 0LL, 0LL, 0},
  {"17", &bandstack17, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 18068000LL, 18168000LL, 0LL, 0LL, 0},
  {"15", &bandstack15, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 21000000LL, 21450000LL, 0LL, 0LL, 0},
  {"12", &bandstack12, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 24890000LL, 24990000LL, 0LL, 0LL, 0},
  {"10", &bandstack10, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 28000000LL, 29700000LL, 0LL, 0LL, 0},
  {"6", &bandstack6, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 50000000LL, 54000000LL, 0LL, 0LL, 0},
  {"4", &bandstack70, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 70000000LL, 70500000LL, 0LL, 0LL, 0},
  {"144", &bandstack144, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 144000000LL, 148000000LL, 0LL, 0LL, 0},
  {"220", &bandstack220, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 222000000LL, 224980000LL, 0LL, 0LL, 0},
  {"430", &bandstack430, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 420000000LL, 450000000LL, 0LL, 0LL, 0},
  {"902", &bandstack902, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 902000000LL, 928000000LL, 0LL, 0LL, 0},
  {"1240", &bandstack1240, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 1240000000LL, 1300000000LL, 0LL, 0LL, 0},
  {"2300", &bandstack2300, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 2300000000LL, 2450000000LL, 0LL, 0LL, 0},
  {"3400", &bandstack3400, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 3400000000LL, 3410000000LL, 0LL, 0LL, 0},
  {"AIR", &bandstackAIR, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 108000000LL, 137000000LL, 0LL, 0LL, 0},
  {"WWV", &bandstackWWV, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 0LL, 0LL, 0LL, 0LL, 0},
  {"GEN", &bandstackGEN, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 0LL, 0LL, 0LL, 0LL, 0},
  // XVTRS
  {"", &bandstack_xvtr_0, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 0LL, 0LL, 0LL, 0LL, 1},
  {"", &bandstack_xvtr_1, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 0LL, 0LL, 0LL, 0LL, 1},
  {"", &bandstack_xvtr_2, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 0LL, 0LL, 0LL, 0LL, 1},
  {"", &bandstack_xvtr_3, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 0LL, 0LL, 0LL, 0LL, 1},
  {"", &bandstack_xvtr_4, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 0LL, 0LL, 0LL, 0LL, 1},
  {"", &bandstack_xvtr_5, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 0LL, 0LL, 0LL, 0LL, 1},
  {"", &bandstack_xvtr_6, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 0LL, 0LL, 0LL, 0LL, 1},
  {"", &bandstack_xvtr_7, 0, 0, 0, 0, 0, ALEX_ATTENUATION_0dB, 53.0, 0LL, 0LL, 0LL, 0LL, 1}
};

CHANNEL band_channels_60m_UK[UK_CHANNEL_ENTRIES] = {
  {5261250LL, 5500LL},
  {5280000LL, 8000LL},
  {5290250LL, 3500LL},
  {5302500LL, 9000LL},
  {5318000LL, 10000LL},
  {5335500LL, 5000LL},
  {5356000LL, 4000LL},
  {5368250LL, 12500LL},
  {5380000LL, 4000LL},
  {5398250LL, 6500LL},
  {5405000LL, 3000LL}
};

//
// Many countries have now allowed ham radio on the 60m
// band based on WRC15. There is a single 15 kHz wide "channel"
//
CHANNEL band_channels_60m_WRC15[WRC15_CHANNEL_ENTRIES] =
{{5359000LL, 15000LL}};

CHANNEL band_channels_60m_OTHER[OTHER_CHANNEL_ENTRIES] = {
  {5332000LL, 2800LL},
  {5348000LL, 2800LL},
  {5358500LL, 2800LL},
  {5373000LL, 2800LL},
  {5405000LL, 2800LL}
};

int channel_entries;
CHANNEL *band_channels_60m;

BANDSTACK *bandstack_get_bandstack(int band) {
  return bands[band].bandstack;
}

int band_get_current() {
  return current_band;
}

BAND *band_get_current_band() {
  BAND *b = &bands[current_band];
  return b;
}

BAND *band_get_band(int b) {
  BAND *band = &bands[b];
  return band;
}

BAND *band_set_current(int b) {
  current_band = b;
  return &bands[b];
}

void bandSaveState() {
  char name[128];
  char value[128];
  SetPropI0("band",                 current_band);

  for (int b = 0; b < BANDS + XVTRS; b++) {
    //
    // Skip non-assigned transverter bands
    //
    if (strlen(bands[b].title) == 0) { continue; }

    SetPropS1("band.%d.title", b,              bands[b].title);
    SetPropI1("band.%d.current", b,            bands[b].bandstack->current_entry);
    SetPropI1("band.%d.preamp", b,             bands[b].preamp);
    SetPropI1("band.%d.alexRxAntenna", b,      bands[b].alexRxAntenna);
    SetPropI1("band.%d.alexTxAntenna", b,      bands[b].alexTxAntenna);
    SetPropI1("band.%d.alexAttenuation", b,    bands[b].alexAttenuation);
    SetPropF1("band.%d.pa_calibration", b,     bands[b].pa_calibration);
    SetPropI1("band.%d.OCrx", b,               bands[b].OCrx);
    SetPropI1("band.%d.OCtx", b,               bands[b].OCtx);
    SetPropI1("band.%d.frequencyMin", b,       bands[b].frequencyMin);
    SetPropI1("band.%d.frequencyMax", b,       bands[b].frequencyMax);
    SetPropI1("band.%d.frequencyLO", b,        bands[b].frequencyLO);
    SetPropI1("band.%d.errorLO", b,            bands[b].errorLO);
    SetPropI1("band.%d.disablePA", b,          bands[b].disablePA);

    for (int stack = 0; stack < bands[b].bandstack->entries; stack++) {
      BANDSTACK_ENTRY *entry = bands[b].bandstack->entry;
      entry += stack;
      SetPropI2("band.%d.stack.%d.a", b, stack,              entry->frequency);
      SetPropI2("band.%d.stack.%d.mode", b,                  stack, entry->mode);
      SetPropI2("band.%d.stack.%d.filter", b,                stack, entry->filter);
      SetPropI2("band.%d.stack.%d.ctun", b, stack,           entry->ctun);
      SetPropI2("band.%d.stack.%d.c", b, stack,              entry->ctun_frequency);
      SetPropI2("band.%d.ctcss_enabled.%d.c", b, stack,      entry->ctcss_enabled);
      SetPropI2("band.%d.ctcss.%d.c", b, stack,              entry->ctcss);
    }
  }
}

void bandRestoreState() {
  char name[128];
  char* value;
  GetPropI0("band",                 current_band);

  for (int b = 0; b < BANDS + XVTRS; b++) {
    //
    // For the "normal" (non-XVTR) bands, do not overwrite
    // the compile-time constants title,fmin,fmax,LO,errorLO,disablePA
    // (This is to guard against starting from an out-dated props file)
    //
    if (b >= BANDS) {
      GetPropS1("band.%d.title", b,              bands[b].title);
      GetPropI1("band.%d.frequencyMin", b,       bands[b].frequencyMin);
      GetPropI1("band.%d.frequencyMax", b,       bands[b].frequencyMax);
      GetPropI1("band.%d.frequencyLO", b,        bands[b].frequencyLO);
      GetPropI1("band.%d.errorLO", b,            bands[b].errorLO);
      GetPropI1("band.%d.disablePA", b,          bands[b].disablePA);
    }

    GetPropI1("band.%d.current", b,            bands[b].bandstack->current_entry);
    GetPropI1("band.%d.preamp", b,             bands[b].preamp);
    GetPropI1("band.%d.alexRxAntenna", b,      bands[b].alexRxAntenna);
    GetPropI1("band.%d.alexTxAntenna", b,      bands[b].alexTxAntenna);
    GetPropI1("band.%d.alexAttenuation", b,    bands[b].alexAttenuation);
    GetPropF1("band.%d.pa_calibration", b,     bands[b].pa_calibration);
    GetPropI1("band.%d.OCrx", b,               bands[b].OCrx);
    GetPropI1("band.%d.OCtx", b,               bands[b].OCtx);

    for (int stack = 0; stack < bands[b].bandstack->entries; stack++) {
      BANDSTACK_ENTRY *entry = bands[b].bandstack->entry;
      entry += stack;
      GetPropI2("band.%d.stack.%d.a", b, stack,              entry->frequency);
      GetPropI2("band.%d.stack.%d.mode", b,                  stack, entry->mode);
      GetPropI2("band.%d.stack.%d.filter", b,                stack, entry->filter);
      GetPropI2("band.%d.stack.%d.ctun", b, stack,           entry->ctun);
      GetPropI2("band.%d.stack.%d.c", b, stack,              entry->ctun_frequency);
      GetPropI2("band.%d.ctcss_enabled.%d.c", b, stack,      entry->ctcss_enabled);
      GetPropI2("band.%d.ctcss.%d.c", b, stack,              entry->ctcss);
    }
  }

  //
  // Sanity checks
  //
  if (current_band >= BANDS + XVTRS) { current_band = band20; }

  for (int b = 0; b < BANDS + XVTRS; b++) {
    //
    // Some sanity checks
    //
    if (bands[b].bandstack->current_entry >= bands[b].bandstack->entries) {
      bands[b].bandstack->current_entry = 0;
    }

    if (bands[b].alexTxAntenna > 2 || bands[b].alexTxAntenna < 0) {
      bands[b].alexTxAntenna = 0;
    }

    if (bands[b].pa_calibration < 38.8) {
      bands[b].pa_calibration = 38.8;
    }

    if (bands[b].pa_calibration > 70.0) {
      bands[b].pa_calibration = 70.0;
    }
  }
}

int get_band_from_frequency(long long f) {
  int b;
  int found = -1;

  //
  // do not search non-xvtr bands if frequency not supported
  // by the radio
  //
  if (f >= radio->frequency_min && f <= radio->frequency_max) {
    for (b = 0; b < BANDS; b++) {
      const BAND *band = band_get_band(b);

      if (strlen(band->title) > 0) {
        if (f >= band->frequencyMin && f <= band->frequencyMax) {
          found = b;
          break;
        }
      }
    }
  }

  //
  // start a new search on the xvtr bands such that if a xvtr
  // band produces a match, it will take precedence
  //
  for (b = BANDS; b < BANDS + XVTRS; b++) {
    const BAND *band = band_get_band(b);

    if (strlen(band->title) > 0) {
      if (f >= band->frequencyMin && f <= band->frequencyMax) {
        found = b;
        break;
      }
    }
  }

  //
  // If the frequency is out of range, use bandGen
  //
  if (found < 0) { found = bandGen; }

  return found;
}

#if 0
char* getFrequencyInfo(long long frequency, int filter_low, int filter_high) {
  char* result = outOfBand;
  int i;
  long long flow = frequency + (long long)filter_low;
  long long fhigh = frequency + (long long)filter_high;
  int b;
  info_band = BANDS + XVTRS;

  for (b = 0; b < BANDS + XVTRS; b++) {
    BAND *band = band_get_band(b);

    if (strlen(band->title) > 0) {
      if (flow >= band->frequencyMin && fhigh <= band->frequencyMax) {
        if (b == band60) {
          for (i = 0; i < channel_entries; i++) {
            long long low_freq = band_channels_60m[i].frequency - (band_channels_60m[i].width / (long long)2);
            long long hi_freq = band_channels_60m[i].frequency + (band_channels_60m[i].width / (long long)2);

            if (flow >= low_freq && fhigh <= hi_freq) {
              info_band = b;
              result = band->title;
              break;
            }
          }
        } else {
          info_band = b;
          result = band->title;
          break;
        }
      }
    }
  }

  t_print("getFrequencyInfo %lld is %s\n", frequency, result);
  return result;
}
#endif

int TransmitAllowed() {
  int result;
  long long txfreq, flow, fhigh;
  int txb, txvfo, txmode;
  BAND *txband;

  //
  // If there is no transmitter, we cannot transmit
  //
  if (!can_transmit) { return 0; }

  //
  // quick return if out-of-band TX is enabled
  //
  if (tx_out_of_band) { return 1; }

  txvfo = get_tx_vfo();
  txb = vfo[txvfo].band;

  //
  // See if we have a band
  //
  if (info_band != bandGen && info_band != bandWWV && info_band != bandAIR) {
    result = TRUE;
  }

  if (txb == bandGen || txb  == bandWWV || txb  == bandAIR) { return 0; }

  //
  // Determine the edges of our band
  // and the edges of our TX signal
  //
  txband = band_get_band(vfo[txvfo].band);
  txfreq = get_tx_freq();
  txmode = get_tx_mode();

  switch (txmode) {
  case modeCWU:
    flow = fhigh = cw_is_on_vfo_freq ? txfreq : txfreq + cw_keyer_sidetone_frequency;
    break;

  case modeCWL:
    flow = fhigh = cw_is_on_vfo_freq ? txfreq : txfreq - cw_keyer_sidetone_frequency;
    break;

  default:
    flow = txfreq + transmitter->filter_low;
    fhigh = txfreq + transmitter->filter_high;
    break;
  }

  if (txb == band60) {
    //
    // For 60m band, ensure signal is within one of the "channels"
    //
    result = 0;

    for (int i = 0; i < channel_entries; i++) {
      long long low_freq = band_channels_60m[i].frequency - (band_channels_60m[i].width / (long long)2);
      long long hi_freq = band_channels_60m[i].frequency + (band_channels_60m[i].width / (long long)2);

      //t_print("TRY CHANNEL: low=%lld high=%lld SIGNAL: low=%lld high=%lld\n", low_freq, hi_freq, flow, fhigh);
      if (flow >= low_freq && fhigh <= hi_freq) {
        //t_print("60m channel OK: chan=%d flow=%lld fhigh=%lld\n", i, flow, fhigh);
        result = 1;
        break;
      }
    }

    //t_print("60m channel NOT FOUND: flow=%lld fhigh=%lld\n", flow, fhigh);
  } else {
    //
    // For other bands, return true if signal within band
    //
    result = flow >= txband->frequencyMin && fhigh <= txband->frequencyMax;
  }

  //t_print("CANTRANSMIT: low=%lld  high=%lld transmit=%d\n", flow, fhigh, result);
  return result;
}


void band_plus(int id) {
  long long frequency_min = radio->frequency_min;
  long long frequency_max = radio->frequency_max;
  int b = vfo[id].band;
  int found = 0;

  while (!found) {
    BAND *band;
    b++;

    if (b >= BANDS + XVTRS) { b = 0; }

    band = (BAND*)band_get_band(b);

    if (strlen(band->title) > 0) {
      if (b < BANDS) {
        if (!(band->frequencyMin == 0.0 && band->frequencyMax == 0.0)) {
          if (band->frequencyMin < frequency_min || band->frequencyMax > frequency_max) {
            continue;
          }
        }
      }

      vfo_band_changed(id, b);
      found = 1;
    }
  }
}

void band_minus(int id) {
  long long frequency_min = radio->frequency_min;
  long long frequency_max = radio->frequency_max;
  int b = vfo[id].band;
  int found = 0;

  while (!found) {
    BAND *band;
    b--;

    if (b < 0) { b = BANDS + XVTRS - 1; }

    band = (BAND*)band_get_band(b);

    if (strlen(band->title) > 0) {
      if (b < BANDS) {
        if (band->frequencyMin < frequency_min || band->frequencyMax > frequency_max) {
          continue;
        }
      }

      vfo_band_changed(id, b);
      found = 1;
    }
  }
}

