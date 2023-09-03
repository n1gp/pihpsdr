/* Copyright (C)
* 2021 - John Melton, G0ORX/N6LYT
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

#include <gtk/gtk.h>
#include <math.h>
#include <wdsp.h>

#include "main.h"
#include "discovery.h"
#include "receiver.h"
#include "sliders.h"
#include "band_menu.h"
#include "diversity_menu.h"
#include "vfo.h"
#include "radio.h"
#include "radio_menu.h"
#include "new_menu.h"
#include "new_protocol.h"
#include "ps_menu.h"
#include "agc.h"
#include "filter.h"
#include "mode.h"
#include "band.h"
#include "bandstack.h"
#include "noise_menu.h"
#ifdef CLIENT_SERVER
  #include "client_server.h"
#endif
#include "ext.h"
#include "zoompan.h"
#include "actions.h"
#include "gpio.h"
#include "toolbar.h"
#include "iambic.h"
#include "message.h"

//
// The "short button text" (button_str) needs to be present in ALL cases, and must be different
// for each case. button_str is used to identify the action in the props files and therefore
// it should not contain white space. Apart from the props files, the button_str determines
// what is written on the buttons in the toolbar (but that's it).
// For finding an action in the "action_dialog", it is most convenient if these actions are
// (roughly) sorted by the first string, but keep "NONE" at the beginning
//
ACTION_TABLE ActionTable[] = {
  {NO_ACTION,           "None",                 "NONE",         TYPE_NONE},
  {A_SWAP_B,            "A<>B",                 "A<>B",         MIDI_KEY   | CONTROLLER_SWITCH},
  {B_TO_A,              "A<B",                  "A<B",          MIDI_KEY   | CONTROLLER_SWITCH},
  {A_TO_B,              "A>B",                  "A>B",          MIDI_KEY   | CONTROLLER_SWITCH},
  {AF_GAIN,             "AF Gain",              "AFGAIN",       MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {AF_GAIN_RX1,         "AF Gain\nRX1",         "AFGAIN1",      MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {AF_GAIN_RX2,         "AF Gain\nRX2",         "AFGAIN2",      MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {AGC,                 "AGC",                  "AGCT",         MIDI_KEY   | CONTROLLER_SWITCH},
  {AGC_GAIN,            "AGC Gain",             "AGCGain",      MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {AGC_GAIN_RX1,        "AGC Gain\nRX1",        "AGCGain1",     MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {AGC_GAIN_RX2,        "AGC Gain\nRX2",        "AGCGain2",     MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {MENU_AGC,            "AGC\nMenu",            "AGC",          MIDI_KEY   | CONTROLLER_SWITCH},
  {ANF,                 "ANF",                  "ANF",          MIDI_KEY   | CONTROLLER_SWITCH},
  {ATTENUATION,         "Atten",                "ATTEN",        MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {BAND_10,             "Band 10",              "10",           MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_12,             "Band 12",              "12",           MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_1240,           "Band 1240",            "1240",         MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_144,            "Band 144",             "144",          MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_15,             "Band 15",              "15",           MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_160,            "Band 160",             "160",          MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_17,             "Band 17",              "17",           MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_20,             "Band 20",              "20",           MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_220,            "Band 220",             "220",          MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_2300,           "Band 2300",            "2300",         MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_30,             "Band 30",              "30",           MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_3400,           "Band 3400",            "3400",         MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_40,             "Band 40",              "40",           MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_430,            "Band 430",             "430",          MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_6,              "Band 6",               "6",            MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_60,             "Band 60",              "60",           MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_70,             "Band 70",              "70",           MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_80,             "Band 80",              "80",           MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_902,            "Band 902",             "902",          MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_AIR,            "Band AIR",             "AIR",          MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_GEN,            "Band GEN",             "GEN",          MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_MINUS,          "Band -",               "BND-",         MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_PLUS,           "Band +",               "BND+",         MIDI_KEY   | CONTROLLER_SWITCH},
  {BAND_WWV,            "Band WWV",             "WWV",          MIDI_KEY   | CONTROLLER_SWITCH},
  {BANDSTACK_MINUS,     "BndStack -",           "BSTK-",        MIDI_KEY   | CONTROLLER_SWITCH},
  {BANDSTACK_PLUS,      "BndStack +",           "BSTK+",        MIDI_KEY   | CONTROLLER_SWITCH},
  {MENU_BAND,           "Band\nMenu",           "BAND",         MIDI_KEY   | CONTROLLER_SWITCH},
  {MENU_BANDSTACK,      "BndStack\nMenu",       "BSTK",         MIDI_KEY   | CONTROLLER_SWITCH},
  {COMP_ENABLE,         "Cmpr On/Off",          "COMP",         MIDI_KEY   | CONTROLLER_SWITCH},
  {COMPRESSION,         "Cmpr Level",           "COMPVAL",      MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {CTUN,                "CTUN",                 "CTUN",         MIDI_KEY   | CONTROLLER_SWITCH},
  {CW_AUDIOPEAKFILTER,  "CW Audio\nPeak Fltr",  "CW-APF",       MIDI_KEY   | CONTROLLER_SWITCH},
  {CW_FREQUENCY,        "CW Frequency",         "CWFREQ",       MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {CW_LEFT,             "CW Left",              "CWL",          MIDI_KEY   | CONTROLLER_SWITCH},
  {CW_RIGHT,            "CW Right",             "CWR",          MIDI_KEY   | CONTROLLER_SWITCH},
  {CW_SPEED,            "CW Speed",             "CWSPD",        MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {CW_KEYER_KEYDOWN,    "CW Key\n(keyer)",      "CWKy",         MIDI_KEY   | CONTROLLER_SWITCH},
  {CW_KEYER_PTT,        "PTT\n(CW keyer)",      "CWKyPTT",      MIDI_KEY   | CONTROLLER_SWITCH},
  {DIV,                 "DIV On/Off",           "DIVT",         MIDI_KEY   | CONTROLLER_SWITCH},
  {DIV_GAIN,            "DIV Gain",             "DIVG",         MIDI_WHEEL | CONTROLLER_ENCODER},
  {DIV_GAIN_COARSE,     "DIV Gain\nCoarse",     "DIVGC",        MIDI_WHEEL | CONTROLLER_ENCODER},
  {DIV_GAIN_FINE,       "DIV Gain\nFine",       "DIVGF",        MIDI_WHEEL | CONTROLLER_ENCODER},
  {DIV_PHASE,           "DIV Phase",            "DIVP",         MIDI_WHEEL | CONTROLLER_ENCODER},
  {DIV_PHASE_COARSE,    "DIV Phase\nCoarse",    "DIVPC",        MIDI_WHEEL | CONTROLLER_ENCODER},
  {DIV_PHASE_FINE,      "DIV Phase\nFine",      "DIVPF",        MIDI_WHEEL | CONTROLLER_ENCODER},
  {MENU_DIVERSITY,      "DIV\nMenu",            "DIV",          MIDI_KEY   | CONTROLLER_SWITCH},
  {DUPLEX,              "Duplex",               "DUP",          MIDI_KEY   | CONTROLLER_SWITCH},
  {FILTER_MINUS,        "Filter -",             "FL-",          MIDI_KEY   | CONTROLLER_SWITCH},
  {FILTER_PLUS,         "Filter +",             "FL+",          MIDI_KEY   | CONTROLLER_SWITCH},
  {FILTER_CUT_LOW,      "Filter Cut\nLow",      "FCUTL",        MIDI_WHEEL | CONTROLLER_ENCODER},
  {FILTER_CUT_HIGH,     "Filter Cut\nHigh",     "FCUTH",        MIDI_WHEEL | CONTROLLER_ENCODER},
  {FILTER_CUT_DEFAULT,  "Filter Cut\nDefault",  "FCUTDEF",      MIDI_KEY   | CONTROLLER_SWITCH},
  {MENU_FILTER,         "Filter\nMenu",           "FILT",         MIDI_KEY   | CONTROLLER_SWITCH},
  {FUNCTION,            "FUNC",                 "FUNC",         MIDI_KEY   | CONTROLLER_SWITCH},
  {IF_SHIFT,            "IF Shift",             "IFSHFT",       MIDI_WHEEL | CONTROLLER_ENCODER},
  {IF_SHIFT_RX1,        "IF Shift\nRX1",        "IFSHFT1",      MIDI_WHEEL | CONTROLLER_ENCODER},
  {IF_SHIFT_RX2,        "IF Shift\nRX2",        "IFSHFT2",      MIDI_WHEEL | CONTROLLER_ENCODER},
  {IF_WIDTH,            "IF Width",             "IFWIDTH",      MIDI_WHEEL | CONTROLLER_ENCODER},
  {IF_WIDTH_RX1,        "IF Width\nRX1",        "IFWIDTH1",     MIDI_WHEEL | CONTROLLER_ENCODER},
  {IF_WIDTH_RX2,        "IF Width\nRX2",        "IFWIDTH2",     MIDI_WHEEL | CONTROLLER_ENCODER},
  {LINEIN_GAIN,         "Linein\nGain",         "LIGAIN",       MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {LOCK,                "Lock",                 "LOCKM",        MIDI_KEY   | CONTROLLER_SWITCH},
  {MENU_MEMORY,         "Memory\nMenu",         "MEM",          MIDI_KEY   | CONTROLLER_SWITCH},
  {MIC_GAIN,            "Mic Gain",             "MICGAIN",      MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {MODE_MINUS,          "Mode -",               "MD-",          MIDI_KEY   | CONTROLLER_SWITCH},
  {MODE_PLUS,           "Mode +",               "MD+",          MIDI_KEY   | CONTROLLER_SWITCH},
  {MENU_MODE,           "Mode\nMenu",           "MODE",         MIDI_KEY   | CONTROLLER_SWITCH},
  {MOX,                 "MOX",                  "MOX",          MIDI_KEY   | CONTROLLER_SWITCH},
  {MUTE,                "Mute",                 "MUTE",         MIDI_KEY   | CONTROLLER_SWITCH},
  {NB,                  "NB",                   "NB",           MIDI_KEY   | CONTROLLER_SWITCH},
  {NR,                  "NR",                   "NR",           MIDI_KEY   | CONTROLLER_SWITCH},
  {MENU_NOISE,          "Noise\nMenu",          "NOISE",        MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_0,            "NumPad 0",             "0",            MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_1,            "NumPad 1",             "1",            MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_2,            "NumPad 2",             "2",            MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_3,            "NumPad 3",             "3",            MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_4,            "NumPad 4",             "4",            MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_5,            "NumPad 5",             "5",            MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_6,            "NumPad 6",             "6",            MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_7,            "NumPad 7",             "7",            MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_8,            "NumPad 8",             "8",            MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_9,            "NumPad 9",             "9",            MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_BS,           "NumPad\nBS",           "BS",           MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_CL,           "NumPad\nCL",           "CL",           MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_DEC,          "NumPad\nDec",          "DEC",          MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_KHZ,          "NumPad\nkHz",          "KHZ",          MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_MHZ,          "NumPad\nMHz",          "MHZ",          MIDI_KEY   | CONTROLLER_SWITCH},
  {NUMPAD_ENTER,        "NumPad\nEnter",        "EN",           MIDI_KEY   | CONTROLLER_SWITCH},
  {PAN,                 "PanZoom",              "PAN",          MIDI_WHEEL | CONTROLLER_ENCODER},
  {PAN_MINUS,           "Pan-",                 "PAN-",         MIDI_KEY   | CONTROLLER_SWITCH},
  {PAN_PLUS,            "Pan+",                 "PAN+",         MIDI_KEY   | CONTROLLER_SWITCH},
  {PANADAPTER_HIGH,     "Panadapter\nHigh",     "PANH",         MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {PANADAPTER_LOW,      "Panadapter\nLow",      "PANL",         MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {PANADAPTER_STEP,     "Panadapter\nStep",     "PANS",         MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {PREAMP,              "Preamp\nOn/Off",       "PRE",          MIDI_KEY   | CONTROLLER_SWITCH},
  {PS,                  "PS On/Off",            "PST",          MIDI_KEY   | CONTROLLER_SWITCH},
  {MENU_PS,             "PS Menu",              "PS",           MIDI_KEY   | CONTROLLER_SWITCH},
  {PTT,                 "PTT",                  "PTT",          MIDI_KEY   | CONTROLLER_SWITCH},
  {RF_GAIN,             "RF Gain",              "RFGAIN",       MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {RF_GAIN_RX1,         "RF Gain\nRX1",         "RFGAIN1",      MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {RF_GAIN_RX2,         "RF Gain\nRX2",         "RFGAIN2",      MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {RIT,                 "RIT",                  "RIT",          MIDI_WHEEL | CONTROLLER_ENCODER},
  {RIT_CLEAR,           "RIT\nClear",           "RITCL",        MIDI_KEY   | CONTROLLER_SWITCH},
  {RIT_ENABLE,          "RIT\nOn/Off",          "RITT",         MIDI_KEY   | CONTROLLER_SWITCH},
  {RIT_MINUS,           "RIT -",                "RIT-",         MIDI_KEY   | CONTROLLER_SWITCH},
  {RIT_PLUS,            "RIT +",                "RIT+",         MIDI_KEY   | CONTROLLER_SWITCH},
  {RIT_RX1,             "RIT\nRX1",             "RIT1",         MIDI_WHEEL | CONTROLLER_ENCODER},
  {RIT_RX2,             "RIT\nRX2",             "RIT2",         MIDI_WHEEL | CONTROLLER_ENCODER},
  {RIT_STEP,            "RIT\nStep",            "RITST",        MIDI_KEY   | CONTROLLER_SWITCH},
  {RSAT,                "RSAT",                 "RSAT",         MIDI_KEY   | CONTROLLER_SWITCH},
  {SAT,                 "SAT",                  "SAT",          MIDI_KEY   | CONTROLLER_SWITCH},
  {SNB,                 "SNB",                  "SNB",          MIDI_KEY   | CONTROLLER_SWITCH},
  {SPLIT,               "Split",                "SPLIT",        MIDI_KEY   | CONTROLLER_SWITCH},
  {SQUELCH,             "Squelch",              "SQUELCH",      MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {SQUELCH_RX1,         "Squelch\nRX1",         "SQUELCH1",     MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {SQUELCH_RX2,         "Squelch\nRX2",         "SQUELCH2",     MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {SWAP_RX,             "Swap RX",              "SWAPRX",       MIDI_KEY   | CONTROLLER_SWITCH},
  {TUNE,                "Tune",                 "TUNE",         MIDI_KEY   | CONTROLLER_SWITCH},
  {TUNE_DRIVE,          "Tune\nDrv",            "TUNDRV",       MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {TUNE_FULL,           "Tune\nFull",           "TUNF",         MIDI_KEY   | CONTROLLER_SWITCH},
  {TUNE_MEMORY,         "Tune\nMem",            "TUNM",         MIDI_KEY   | CONTROLLER_SWITCH},
  {DRIVE,               "TX Drive",             "TXDRV",        MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {TWO_TONE,            "Two-Tone",             "2TONE",        MIDI_KEY   | CONTROLLER_SWITCH},
  {VFO,                 "VFO",                  "VFO",          MIDI_WHEEL | CONTROLLER_ENCODER},
  {MENU_FREQUENCY,      "VFO\nMenu",           "FREQ",         MIDI_KEY   | CONTROLLER_SWITCH},
  {VFO_STEP_MINUS,      "VFO Step -",           "STEP-",        MIDI_KEY   | CONTROLLER_SWITCH},
  {VFO_STEP_PLUS,       "VFO Step +",           "STEP+",        MIDI_KEY   | CONTROLLER_SWITCH},
  {VFOA,                "VFO A",                "VFOA",         MIDI_WHEEL | CONTROLLER_ENCODER},
  {VFOB,                "VFO B",                "VFOB",         MIDI_WHEEL | CONTROLLER_ENCODER},
  {VOX,                 "VOX\nOn/Off",          "VOX",          MIDI_KEY   | CONTROLLER_SWITCH},
  {VOXLEVEL,            "VOX\nLevel",           "VOXLEV",       MIDI_WHEEL | CONTROLLER_ENCODER},
  {WATERFALL_HIGH,      "Wfall\nHigh",          "WFALLH",       MIDI_WHEEL | CONTROLLER_ENCODER},
  {WATERFALL_LOW,       "Wfall\nLow",           "WFALLL",       MIDI_WHEEL | CONTROLLER_ENCODER},
  {XIT,                 "XIT",                  "XIT",          MIDI_WHEEL | CONTROLLER_ENCODER},
  {XIT_CLEAR,           "XIT\nClear",           "XITCL",        MIDI_KEY   | CONTROLLER_SWITCH},
  {XIT_ENABLE,          "XIT\nOn/Off",          "XITT",         MIDI_KEY   | CONTROLLER_SWITCH},
  {XIT_MINUS,           "XIT -",                "XIT-",         MIDI_KEY   | CONTROLLER_SWITCH},
  {XIT_PLUS,            "XIT +",                "XIT+",         MIDI_KEY   | CONTROLLER_SWITCH},
  {ZOOM,                "Zoom",                 "ZOOM",         MIDI_KNOB  | MIDI_WHEEL | CONTROLLER_ENCODER},
  {ZOOM_MINUS,          "Zoom -",               "ZOOM-",        MIDI_KEY   | CONTROLLER_SWITCH},
  {ZOOM_PLUS,           "Zoom +",               "ZOOM+",        MIDI_KEY   | CONTROLLER_SWITCH},
  {ACTIONS,             "None",                 "NONE",         TYPE_NONE}
};

static guint timer = 0;
static gboolean timer_released;

static int timeout_cb(gpointer data) {
  if (timer_released) {
    g_free(data);
    timer = 0;
    return G_SOURCE_REMOVE;
  }

  // process the action;
  process_action(data);
  return TRUE;
}

static inline double KnobOrWheel(const PROCESS_ACTION *a, double oldval, double minval, double maxval, double inc) {
  //
  // Knob ("Potentiometer"):  set value
  // Wheel("Rotary Encoder"): increment/decrement the value (by "inc" per tick)
  //
  // In both cases, the returned value is
  //  - in the range minval...maxval
  //  - rounded to a multiple of inc
  //
  switch (a->mode) {
  case RELATIVE:
    oldval += a->val * inc;
    break;

  case ABSOLUTE:
    oldval = minval + a->val * (maxval - minval) * 0.01;
    break;

  default:
    // do nothing
    break;
  }

  //
  // Round and check range
  //
  oldval = inc * round(oldval / inc);

  if (oldval > maxval) { oldval = maxval; }

  if (oldval < minval) { oldval = minval; }

  return oldval;
}

//
// This interface puts an "action" into the GTK idle queue,
// but "CW key" actions are processed immediately
//
void schedule_action(enum ACTION action, enum ACTION_MODE mode, gint val) {
  PROCESS_ACTION *a;

  switch (action) {
  case CW_LEFT:
  case CW_RIGHT:
    cw_key_hit = 1;
    keyer_event(action == CW_LEFT, mode == PRESSED);
    break;

  case CW_KEYER_KEYDOWN:

    //
    // hard "key-up/down" action WITHOUT break-in
    // intended for external keyers (MIDI or GPIO connected)
    // which take care of PTT themselves.
    //
    if (mode == PRESSED && (cw_keyer_internal == 0 || CAT_cw_is_active)) {
      cw_key_down = 960000; // max. 20 sec to protect hardware
      cw_key_up = 0;
      cw_key_hit = 1;
    } else {
      cw_key_down = 0;
      cw_key_up = 0;
    }

    break;

  default:
    //
    // schedule action through GTK idle queue
    //
    a = g_new(PROCESS_ACTION, 1);
    a->action = action;
    a->mode = mode;
    a->val = val;
    g_idle_add(process_action, a);
    break;
  }
}

int process_action(void *data) {
  PROCESS_ACTION *a = (PROCESS_ACTION *)data;
  double value;
  int i;
  gboolean free_action = TRUE;

  //t_print("%s: action=%d mode=%d value=%d\n",__FUNCTION__,a->action,a->mode,a->val);
  switch (a->action) {
  case A_SWAP_B:
    if (a->mode == PRESSED) {
      vfo_a_swap_b();
    }

    break;

  case A_TO_B:
    if (a->mode == PRESSED) {
      vfo_a_to_b();
    }

    break;

  case AF_GAIN:
    value = KnobOrWheel(a, active_receiver->volume, -40.0, 0.0, 1.0);
    set_af_gain(active_receiver->id, value);
    break;

  case AF_GAIN_RX1:
    value = KnobOrWheel(a, receiver[0]->volume, -40.0, 0.0, 1.0);
    set_af_gain(0, value);
    break;

  case AF_GAIN_RX2:
    if (receivers == 2) {
      value = KnobOrWheel(a, receiver[1]->volume, -40.0, 0.0, 1.0);
      set_af_gain(1, value);
    }

    break;

  case AGC:
    if (a->mode == PRESSED) {
      active_receiver->agc++;

      if (active_receiver->agc >= AGC_LAST) {
        active_receiver->agc = 0;
      }

      set_agc(active_receiver, active_receiver->agc);
      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case AGC_GAIN:
    value = KnobOrWheel(a, active_receiver->agc_gain, -20.0, 120.0, 1.0);
    set_agc_gain(active_receiver->id, value);
    break;

  case AGC_GAIN_RX1:
    value = KnobOrWheel(a, receiver[0]->agc_gain, -20.0, 120.0, 1.0);
    set_agc_gain(0, value);
    break;

  case AGC_GAIN_RX2:
    if (receivers == 2) {
      value = KnobOrWheel(a, receiver[1]->agc_gain, -20.0, 120.0, 1.0);
      set_agc_gain(1, value);
    }

    break;

  case ANF:
    if (a->mode == PRESSED) {
      if (active_receiver->anf == 0) {
        active_receiver->anf = 1;
        mode_settings[vfo[active_receiver->id].mode].anf = 1;
      } else {
        active_receiver->anf = 0;
        mode_settings[vfo[active_receiver->id].mode].anf = 0;
      }

      SetRXAANFRun(active_receiver->id, active_receiver->anf);
      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case ATTENUATION:
    if (have_rx_att) {
      value = KnobOrWheel(a, adc[active_receiver->adc].attenuation,   0.0, 31.0, 1.0);
      set_attenuation_value(value);
    }

    break;

  case B_TO_A:
    if (a->mode == PRESSED) {
      vfo_b_to_a();
    }

    break;

  case BAND_10:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band10);
    }

    break;

  case BAND_12:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band12);
    }

    break;

  case BAND_1240:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band1240);
    }

    break;

  case BAND_144:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band144);
    }

    break;

  case BAND_15:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band15);
    }

    break;

  case BAND_160:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band160);
    }

    break;

  case BAND_17:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band17);
    }

    break;

  case BAND_20:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band20);
    }

    break;

  case BAND_220:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band220);
    }

    break;

  case BAND_2300:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band2300);
    }

    break;

  case BAND_30:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band30);
    }

    break;

  case BAND_3400:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band3400);
    }

    break;

  case BAND_40:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band40);
    }

    break;

  case BAND_430:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band430);
    }

    break;

  case BAND_6:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band6);
    }

    break;

  case BAND_60:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band60);
    }

    break;

  case BAND_70:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band70);
    }

    break;

  case BAND_80:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band80);
    }

    break;

  case BAND_902:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, band902);
    }

    break;

  case BAND_AIR:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, bandAIR);
    }

    break;

  case BAND_GEN:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, bandGen);
    }

    break;

  case BAND_MINUS:
    if (a->mode == PRESSED) {
      band_minus(active_receiver->id);
    }

    break;

  case BAND_PLUS:
    if (a->mode == PRESSED) {
      band_plus(active_receiver->id);
    }

    break;

  case BAND_WWV:
    if (a->mode == PRESSED) {
      vfo_band_changed(active_receiver->id, bandWWV);
    }

    break;

  case BANDSTACK_MINUS:
    if (a->mode == PRESSED) {
      const BAND *band = band_get_band(vfo[active_receiver->id].band);
      const BANDSTACK *bandstack = band->bandstack;
      int b = vfo[active_receiver->id].bandstack - 1;

      if (b < 0) { b = bandstack->entries - 1; };

      vfo_bandstack_changed(b);
    }

    break;

  case BANDSTACK_PLUS:
    if (a->mode == PRESSED) {
      const BAND *band = band_get_band(vfo[active_receiver->id].band);
      const BANDSTACK *bandstack = band->bandstack;
      int b = vfo[active_receiver->id].bandstack + 1;

      if (b >= bandstack->entries) { b = 0; }

      vfo_bandstack_changed(b);
    }

    break;

  case COMP_ENABLE:
    if (can_transmit && a->mode == PRESSED) {
      transmitter_set_compressor(transmitter, transmitter->compressor ? FALSE : TRUE);
      mode_settings[transmitter->mode].compressor = transmitter->compressor;
    }

    break;

  case COMPRESSION:
    if (can_transmit) {
      value = KnobOrWheel(a, transmitter->compressor_level, 0.0, 20.0, 1.0);
      transmitter_set_compressor_level(transmitter, value);
      transmitter_set_compressor(transmitter, value > 0.5);
      mode_settings[transmitter->mode].compressor = transmitter->compressor;
      mode_settings[transmitter->mode].compressor_level = transmitter->compressor_level;
      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case CTUN:
    if (a->mode == PRESSED) {
      int state = vfo[active_receiver->id].ctun ? 0 : 1;
      vfo_ctun_update(active_receiver->id, state);
      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case CW_AUDIOPEAKFILTER:
    if (a->mode == PRESSED) {
      active_receiver->cwAudioPeakFilter = active_receiver->cwAudioPeakFilter ? 0 : 1;
      receiver_filter_changed(active_receiver);
      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case CW_FREQUENCY:
    value = KnobOrWheel(a, (double)cw_keyer_sidetone_frequency, 300.0, 1000.0, 10.0);
    cw_keyer_sidetone_frequency = (int)value;
    receiver_filter_changed(active_receiver);
    // we may omit the P2 high-prio packet since this is sent out at regular intervals
    g_idle_add(ext_vfo_update, NULL);
    break;

  case CW_SPEED:
    value = KnobOrWheel(a, (double)cw_keyer_speed, 1.0, 60.0, 1.0);
    cw_keyer_speed = (int)value;
    keyer_update();
    g_idle_add(ext_vfo_update, NULL);
    break;

  case DIV:
    if (a->mode == PRESSED) {
      diversity_enabled = diversity_enabled == 1 ? 0 : 1;

      if (protocol == NEW_PROTOCOL) {
        schedule_high_priority();
        schedule_receive_specific();
      }

      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case DIV_GAIN:
    update_diversity_gain((double)a->val * 0.5);
    break;

  case DIV_GAIN_COARSE:
    update_diversity_gain((double)a->val * 2.5);
    break;

  case DIV_GAIN_FINE:
    update_diversity_gain((double)a->val * 0.1);
    break;

  case DIV_PHASE:
    update_diversity_phase((double)a->val * 0.5);
    break;

  case DIV_PHASE_COARSE:
    update_diversity_phase((double)a->val * 2.5);
    break;

  case DIV_PHASE_FINE:
    update_diversity_phase((double)a->val * 0.1);
    break;

  case DRIVE:
    value = KnobOrWheel(a, getDrive(), 0.0, drive_max, 1.0);
    set_drive(value);
    break;

  case DUPLEX:
    if (can_transmit && !isTransmitting() && a->mode == PRESSED) {
      duplex = duplex == 1 ? 0 : 1;
      g_idle_add(ext_set_duplex, NULL);
    }

    break;

  case FILTER_MINUS:
    if (a->mode == PRESSED) {
      int f = vfo[active_receiver->id].filter + 1;

      if (f >= FILTERS) { f = 0; }

      vfo_filter_changed(f);
    }

    break;

  case FILTER_PLUS:
    if (a->mode == PRESSED) {
      int f = vfo[active_receiver->id].filter - 1;

      if (f < 0) { f = FILTERS - 1; }

      vfo_filter_changed(f);
    }

    break;

  case FILTER_CUT_HIGH: {
    filter_cut_changed(active_receiver->id, FILTER_CUT_HIGH, a->val);
  }
  break;

  case FILTER_CUT_LOW: {
    filter_cut_changed(active_receiver->id, FILTER_CUT_LOW, a->val);
  }
  break;

  case FILTER_CUT_DEFAULT:
    if (a->mode == PRESSED) {
      filter_cut_default(active_receiver->id);
    }

    break;

  case FUNCTION:
    if (a->mode == PRESSED) {
      function++;

      if (function >= MAX_FUNCTIONS) {
        function = 0;
      }

      toolbar_switches = switches_controller1[function];
      update_toolbar_labels();

      if (controller == CONTROLLER1) {
        switches = switches_controller1[function];
      }
    }

    break;

  case IF_SHIFT:
    filter_shift_changed(active_receiver->id, a->val);
    break;

  case IF_SHIFT_RX1:
    filter_shift_changed(0, a->val);
    break;

  case IF_SHIFT_RX2:
    filter_shift_changed(1, a->val);
    break;

  case IF_WIDTH:
    filter_width_changed(active_receiver->id, a->val);
    break;

  case IF_WIDTH_RX1:
    filter_width_changed(0, a->val);
    break;

  case IF_WIDTH_RX2:
    filter_width_changed(1, a->val);
    break;

  case LINEIN_GAIN:
    value = KnobOrWheel(a, linein_gain, -34.0, 12.5, 1.5);
    set_linein_gain(value);
    break;

  case LOCK:
    if (a->mode == PRESSED) {
#ifdef CLIENT_SERVER

      if (radio_is_remote) {
        send_lock(client_socket, locked == 1 ? 0 : 1);
      } else {
#endif
        locked = locked == 1 ? 0 : 1;
        g_idle_add(ext_vfo_update, NULL);
#ifdef CLIENT_SERVER
      }

#endif
    }

    break;

  case MENU_AGC:
    if (a->mode == PRESSED) {
      start_agc();
    }

    break;

  case MENU_BAND:
    if (a->mode == PRESSED) {
      start_band();
    }

    break;

  case MENU_BANDSTACK:
    if (a->mode == PRESSED) {
      start_bandstack();
    }

    break;

  case MENU_DIVERSITY:
    if (a->mode == PRESSED) {
      start_diversity();
    }

    break;

  case MENU_FILTER:
    if (a->mode == PRESSED) {
      start_filter();
    }

    break;

  case MENU_FREQUENCY:
    if (a->mode == PRESSED) {
      start_vfo(active_receiver->id);
    }

    break;

  case MENU_MEMORY:
    if (a->mode == PRESSED) {
      start_store();
    }

    break;

  case MENU_MODE:
    if (a->mode == PRESSED) {
      start_mode();
    }

    break;

  case MENU_NOISE:
    if (a->mode == PRESSED) {
      start_noise();
    }

    break;

  case MENU_PS:
    if (a->mode == PRESSED) {
      start_ps();
    }

    break;

  case MIC_GAIN:
    value = KnobOrWheel(a, mic_gain, -12.0, 50.0, 1.0);
    set_mic_gain(value);
    break;

  case MODE_MINUS:
    if (a->mode == PRESSED) {
      int mode = vfo[active_receiver->id].mode;
      mode--;

      if (mode < 0) { mode = MODES - 1; }

      vfo_mode_changed(mode);
    }

    break;

  case MODE_PLUS:
    if (a->mode == PRESSED) {
      int mode = vfo[active_receiver->id].mode;
      mode++;

      if (mode >= MODES) { mode = 0; }

      vfo_mode_changed(mode);
    }

    break;

  case MOX:
    if (a->mode == PRESSED) {
      int state = getMox();
      mox_update(!state);
    }

    break;

  case MUTE:
    if (a->mode == PRESSED) {
      active_receiver->mute_radio = !active_receiver->mute_radio;
    }

    break;

  case NB:
    if (a->mode == PRESSED) {
      active_receiver->nb++;

      if (active_receiver->nb > 2) { active_receiver->nb = 0; }

      mode_settings[vfo[active_receiver->id].mode].nb = active_receiver->nb;
      update_noise();
    }

    break;

  case NR:
    if (a->mode == PRESSED) {
      active_receiver->nr++;
#ifdef EXTNR

      if (active_receiver->nr > 4) { active_receiver->nr = 0; }

#else

      if (active_receiver->nr > 2) { active_receiver->nr = 0; }

#endif
      mode_settings[vfo[active_receiver->id].mode].nr = active_receiver->nr;
      update_noise();
    }

    break;

  case NUMPAD_0:
    if (a->mode == PRESSED) {
      num_pad(0, active_receiver->id);
    }

    break;

  case NUMPAD_1:
    if (a->mode == PRESSED) {
      num_pad(1, active_receiver->id);
    }

    break;

  case NUMPAD_2:
    if (a->mode == PRESSED) {
      num_pad(2, active_receiver->id);
    }

    break;

  case NUMPAD_3:
    if (a->mode == PRESSED) {
      num_pad(3, active_receiver->id);
    }

    break;

  case NUMPAD_4:
    if (a->mode == PRESSED) {
      num_pad(4, active_receiver->id);
    }

    break;

  case NUMPAD_5:
    if (a->mode == PRESSED) {
      num_pad(5, active_receiver->id);
    }

    break;

  case NUMPAD_6:
    if (a->mode == PRESSED) {
      num_pad(6, active_receiver->id);
    }

    break;

  case NUMPAD_7:
    if (a->mode == PRESSED) {
      num_pad(7, active_receiver->id);
    }

    break;

  case NUMPAD_8:
    if (a->mode == PRESSED) {
      num_pad(8, active_receiver->id);
    }

    break;

  case NUMPAD_9:
    if (a->mode == PRESSED) {
      num_pad(9, active_receiver->id);
    }

    break;

  case NUMPAD_BS:
    if (a->mode == PRESSED) {
      num_pad(-6, active_receiver->id);
    }

    break;

  case NUMPAD_CL:
    if (a->mode == PRESSED) {
      num_pad(-1, active_receiver->id);
    }

    break;

  case NUMPAD_ENTER:
    if (a->mode == PRESSED) {
      num_pad(-2, active_receiver->id);
    }

    break;

  case NUMPAD_KHZ:
    if (a->mode == PRESSED) {
      num_pad(-3, active_receiver->id);
    }

    break;

  case NUMPAD_MHZ:
    if (a->mode == PRESSED) {
      num_pad(-4, active_receiver->id);
    }

    break;

  case NUMPAD_DEC:
    if (a->mode == PRESSED) {
      num_pad(-5, active_receiver->id);
    }

    break;

  case PAN:
    set_pan(active_receiver->id,  active_receiver->pan + 100*a->val);
    break;

  case PAN_MINUS:
    if (a->mode == PRESSED) {
      set_pan(active_receiver->id,  active_receiver->pan - 100);
    }

    break;

  case PAN_PLUS:
    if (a->mode == PRESSED) {
      set_pan(active_receiver->id,  active_receiver->pan + 100);
    }

    break;

  case PANADAPTER_HIGH:
    value = KnobOrWheel(a, active_receiver->panadapter_high, -60.0, 20.0, 1.0);
    active_receiver->panadapter_high = (int)value;
    break;

  case PANADAPTER_LOW:
    value = KnobOrWheel(a, active_receiver->panadapter_low, -160.0, -60.0, 1.0);
    active_receiver->panadapter_low = (int)value;
    break;

  case PANADAPTER_STEP:
    value = KnobOrWheel(a, active_receiver->panadapter_step, 5.0, 30.0, 1.0);
    active_receiver->panadapter_step = (int)value;
    break;

  case PREAMP:
    break;

  case PS:
    if (a->mode == PRESSED) {
      if (can_transmit) {
        if (transmitter->puresignal == 0) {
          tx_set_ps(transmitter, 1);
        } else {
          tx_set_ps(transmitter, 0);
        }
      }
    }

    break;

  case PTT:
    if (a->mode == PRESSED || a->mode == RELEASED) {
      mox_update(a->mode == PRESSED);
    }

    break;

  case RF_GAIN:
    if (have_rx_gain) {
      value = KnobOrWheel(a, adc[active_receiver->adc].gain, adc[active_receiver->adc].min_gain,
                          adc[active_receiver->adc].max_gain, 1.0);
      set_rf_gain(active_receiver->id, value);
    }

    break;

  case RF_GAIN_RX1:
    if (have_rx_gain) {
      value = KnobOrWheel(a, adc[receiver[0]->adc].gain, adc[receiver[0]->adc].min_gain, adc[receiver[0]->adc].max_gain, 1.0);
      set_rf_gain(0, value);
    }

    break;

  case RF_GAIN_RX2:
    if (have_rx_gain && receivers == 2) {
      value = KnobOrWheel(a, adc[receiver[1]->adc].gain, adc[receiver[1]->adc].min_gain, adc[receiver[1]->adc].max_gain, 1.0);
      set_rf_gain(1, value);
    }

    break;

  case RIT:
    vfo_rit(active_receiver->id, a->val);
    break;

  case RIT_CLEAR:
    if (a->mode == PRESSED) {
      vfo_rit_clear(active_receiver->id);
    }

    break;

  case RIT_ENABLE:
    if (a->mode == PRESSED) {
      vfo_rit_update(active_receiver->id);
    }

    break;

  case RIT_MINUS:
    if (a->mode == PRESSED) {
      vfo_rit(active_receiver->id, -1);

      if (timer == 0) {
        timer = g_timeout_add(250, timeout_cb, a);
        timer_released = FALSE;
      }

      free_action = FALSE;
    } else {
      timer_released = TRUE;
    }

    break;

  case RIT_PLUS:
    if (a->mode == PRESSED) {
      vfo_rit(active_receiver->id, 1);

      if (timer == 0) {
        timer = g_timeout_add(250, timeout_cb, a);
        timer_released = FALSE;
      }

      free_action = FALSE;
    } else {
      timer_released = TRUE;
    }

    break;

  case RIT_RX1:
    vfo_rit(0, a->val);
    break;

  case RIT_RX2:
    vfo_rit(1, a->val);
    break;

  case RIT_STEP:
    if (a->mode == PRESSED) {
      rit_increment = 10 * rit_increment;
      if (rit_increment > 100) { rit_increment = 1; }
    }
    g_idle_add(ext_vfo_update, NULL);
    break;

  case RSAT:
    if (a->mode == PRESSED) {
      if (sat_mode == RSAT_MODE) {
        sat_mode = SAT_NONE;
      } else {
        sat_mode = RSAT_MODE;
      }

      t_print("%s: TODO: report sat mode change upstream\n", __FUNCTION__);
      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case SAT:
    if (a->mode == PRESSED) {
      if (sat_mode == SAT_MODE) {
        sat_mode = SAT_NONE;
      } else {
        sat_mode = SAT_MODE;
      }

      t_print("%s: TODO: report sat mode change upstream\n", __FUNCTION__);
      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case SNB:
    if (a->mode == PRESSED) {
      if (active_receiver->snb == 0) {
        active_receiver->snb = 1;
        mode_settings[vfo[active_receiver->id].mode].snb = 1;
      } else {
        active_receiver->snb = 0;
        mode_settings[vfo[active_receiver->id].mode].snb = 0;
      }

      update_noise();
    }

    break;

  case SPLIT:
    if (a->mode == PRESSED) {
      radio_split_toggle();
    }

    break;

  case SQUELCH:
    value = KnobOrWheel(a, active_receiver->squelch, 0.0, 100.0, 1.0);
    active_receiver->squelch = value;
    set_squelch(active_receiver);
    break;

  case SQUELCH_RX1:
    value = KnobOrWheel(a, receiver[0]->squelch, 0.0, 100.0, 1.0);
    receiver[0]->squelch = value;
    set_squelch(receiver[0]);
    break;

  case SQUELCH_RX2:
    if (receivers == 2) {
      value = KnobOrWheel(a, receiver[1]->squelch, 0.0, 100.0, 1.0);
      receiver[1]->squelch = value;
      set_squelch(receiver[1]);
    }

    break;

  case SWAP_RX:
    if (a->mode == PRESSED) {
      if (receivers == 2) {
        active_receiver = receiver[active_receiver->id == 1 ? 0 : 1];
        g_idle_add(menu_active_receiver_changed, NULL);
        g_idle_add(ext_vfo_update, NULL);
        g_idle_add(sliders_active_receiver_changed, NULL);
      }
    }

    break;

  case TUNE:
    if (a->mode == PRESSED) {
      int state = getTune();
      tune_update(!state);
    }

    break;

  case TUNE_DRIVE:
    if (can_transmit) {
      value = KnobOrWheel(a, (double) transmitter->tune_drive, 0.0, 100.0, 1.0);
      transmitter->tune_drive = (int) value;
      transmitter->tune_use_drive = 1;
      show_popup_slider(TUNE_DRIVE, 0, 0.0, 100.0, 1.0, value, "TUNE DRIVE");
    }
    

    break;

  case TUNE_FULL:
    if (a->mode == PRESSED) {
      if (can_transmit) {
        full_tune = full_tune ? FALSE : TRUE;
        memory_tune = FALSE;
      }
    }

    break;

  case TUNE_MEMORY:
    if (a->mode == PRESSED) {
      if (can_transmit) {
        memory_tune = memory_tune ? FALSE : TRUE;
        full_tune = FALSE;
      }
    }

    break;

  case TWO_TONE:
    if (a->mode == PRESSED) {
      if (can_transmit) {
        int state = transmitter->twotone ? 0 : 1;
        tx_set_twotone(transmitter, state);
      }
    }

    break;

  case VFO:
    if (a->mode == RELATIVE && !locked) {
      vfo_step(a->val);
    }

    break;

  case VFO_STEP_MINUS:
    if (a->mode == PRESSED) {
      i = vfo_get_stepindex();
      vfo_set_step_from_index(--i);
      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case VFO_STEP_PLUS:
    if (a->mode == PRESSED) {
      i = vfo_get_stepindex();
      vfo_set_step_from_index(++i);
      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case VFOA:
    if (a->mode == RELATIVE && !locked) {
      vfo_id_step(0, (int)a->val);
    }

    break;

  case VFOB:
    if (a->mode == RELATIVE && !locked) {
      vfo_id_step(1, (int)a->val);
    }

    break;

  case VOX:
    if (a->mode == PRESSED) {
      vox_enabled = !vox_enabled;
      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case VOXLEVEL:
    vox_threshold = KnobOrWheel(a, vox_threshold, 0.0, 1.0, 0.01);
    break;

  case WATERFALL_HIGH:
    value = KnobOrWheel(a, active_receiver->waterfall_high, -100.0, 0.0, 1.0);
    active_receiver->waterfall_high = (int)value;
    break;

  case WATERFALL_LOW:
    value = KnobOrWheel(a, active_receiver->waterfall_low, -150.0, -50.0, 1.0);
    active_receiver->waterfall_low = (int)value;
    break;

  case XIT:
    value = KnobOrWheel(a, (double)transmitter->xit, -9999.0, 9999.0, (double) rit_increment);
    transmitter->xit = (int)value;
    transmitter->xit_enabled = (value != 0);

    if (protocol == NEW_PROTOCOL) {
      schedule_high_priority();
    }

    g_idle_add(ext_vfo_update, NULL);
    break;

  case XIT_CLEAR:
    if (a->mode == PRESSED) {
      if (can_transmit) {
        transmitter->xit = 0;
        transmitter->xit_enabled = 0;

        if (protocol == NEW_PROTOCOL) {
          schedule_high_priority();
        }

        g_idle_add(ext_vfo_update, NULL);
      }
    }

    break;

  case XIT_ENABLE:
    if (a->mode == PRESSED) {
      if (can_transmit) {
        transmitter->xit_enabled = transmitter->xit_enabled == 1 ? 0 : 1;

        if (protocol == NEW_PROTOCOL) {
          schedule_high_priority();
        }
      }

      g_idle_add(ext_vfo_update, NULL);
    }

    break;

  case XIT_MINUS:
    if (can_transmit) {
      if (a->mode == PRESSED) {
        value = (double)transmitter->xit;
        value -= (double)rit_increment;

        if (value < -9999.0) {
          value = -9999.0;
        } else if (value > 9999.0) {
          value = 9999.0;
        }

        transmitter->xit = (int)value;
        transmitter->xit_enabled = (transmitter->xit != 0);

        if (protocol == NEW_PROTOCOL) {
          schedule_high_priority();
        }

        g_idle_add(ext_vfo_update, NULL);

        if (timer == 0) {
          timer = g_timeout_add(250, timeout_cb, a);
          timer_released = FALSE;
        }

        free_action = FALSE;
      } else {
        timer_released = TRUE;
      }
    }

    break;

  case XIT_PLUS:
    if (can_transmit) {
      if (a->mode == PRESSED) {
        value = (double)transmitter->xit;
        value += (double)rit_increment;

        if (value < -10000.0) {
          value = -10000.0;
        } else if (value > 10000.0) {
          value = 10000.0;
        }

        transmitter->xit = (int)value;
        transmitter->xit_enabled = (transmitter->xit != 0);

        if (protocol == NEW_PROTOCOL) {
          schedule_high_priority();
        }

        g_idle_add(ext_vfo_update, NULL);

        if (timer == 0) {
          timer = g_timeout_add(250, timeout_cb, a);
          timer_released = FALSE;
        }

        free_action = FALSE;
      } else {
        timer_released = TRUE;
      }
    }

    break;

  case ZOOM:
    value = KnobOrWheel(a, active_receiver->zoom, 1.0, 8.0, 1.0);
    set_zoom(active_receiver->id, (int)  value);
    break;

  case ZOOM_MINUS:
    if (a->mode == PRESSED) {
      set_zoom(active_receiver->id, active_receiver->zoom - 1);
    }

    break;

  case ZOOM_PLUS:
    if (a->mode == PRESSED) {
      set_zoom(active_receiver->id, active_receiver->zoom + 1);
    }

    break;

  case CW_KEYER_PTT:

    //
    // If you do CW with the key attached to the radio, and use a foot-switch for
    // PTT, then this should trigger the standard PTT event. However, if you have the
    // the key attached to the radio and want to use an external keyer (e.g.
    // controlled by a contest logger), then "internal CW" muste temporarily be
    // disabled in the radio (while keying from piHPSDR) in the radio.
    // This is exactly the same situation as when using CAT
    // CW commands together with "internal" CW (status variable CAT_cw_is_active),
    // so the mechanism is already there. Therefore, the present case is just
    // the same as "PTT" except that we set/clear the "CAT CW" condition.
    //
    // If the "CAT CW" flag is already cleared when the PTT release arrives, this
    // means that the CW message from the keyer has been aborted by hitting the
    // CW key. In this case, the radio takes care of "going RX".
    //
    switch (a->mode) {
    case PRESSED:
      CAT_cw_is_active = 1;
      mox_update(1);
      break;

    case RELEASED:
      if (CAT_cw_is_active == 1) {
        CAT_cw_is_active = 0;
        mox_update(0);
      }

      break;

    default:
      // should not happen
      break;
    }

    break;

  case NO_ACTION:
    // do nothing
    break;

  default:
    if (a->action >= 0 && a->action < ACTIONS) {
      t_print("%s: UNKNOWN PRESSED SWITCH ACTION %d (%s)\n", __FUNCTION__, a->action, ActionTable[a->action].str);
    } else {
      t_print("%s: INVALID PRESSED SWITCH ACTION %d\n", __FUNCTION__, a->action);
    }

    break;
  }

  if (free_action) {
    g_free(data);
  }

  return 0;
}

//
// Function to convert an internal action number to a unique string
// This is used to specify actions in the props files.
//
void Action2String(int id, char *str) {
  if (id < 0 || id >= ACTIONS) {
    strcpy(str, "NONE");
  } else {
    strcpy(str, ActionTable[id].button_str);
  }
}

//
// Function to convert a string to an action number
// This is used to specify actions in the props files.
//
int String2Action(const char *str) {
  int i;

  for (i = 0; i < ACTIONS; i++) {
    if (!strcmp(str, ActionTable[i].button_str)) { return i; }
  }

  return NO_ACTION;
}
