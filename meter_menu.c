/* Copyright (C)
* 2016 - John Melton, G0ORX/N6LYT
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
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include <wdsp.h>

#include "new_menu.h"
#include "receiver.h"
#include "meter_menu.h"
#include "meter.h"
#include "radio.h"

static GtkWidget *dialog = NULL;

static void cleanup() {
  if (dialog != NULL) {
    GtkWidget *tmp=dialog;
    dialog = NULL;
    gtk_widget_destroy(tmp);
    sub_menu = NULL;
    active_menu  = NO_MENU;
  }
}

static gboolean close_cb () {
  cleanup();
  return TRUE;
}

static void smeter_select_cb (GtkToggleButton *widget, gpointer        data) {
  int val = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));

  switch (val) {
    case 0:
      smeter = RXA_S_PK;
      break;
    case 1:
      smeter = RXA_S_AV;
      break;
  }
}

static void analog_cb (GtkToggleButton *widget, gpointer        data) {
  analog_meter = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));
}

static void alc_select_cb(GtkToggleButton *widget, gpointer data) {
  int val = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));

  switch (val) {
    case 0:
      alc = TXA_ALC_PK;
      break;
    case 1:
      alc = TXA_ALC_AV;
      break;
    case 2:
      alc = TXA_ALC_GAIN;
      break;
  }
}

void meter_menu (GtkWidget *parent) {
  GtkWidget *w;

  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
  gtk_window_set_title(GTK_WINDOW(dialog), "piHPSDR - Meter");
  g_signal_connect (dialog, "delete_event", G_CALLBACK (close_cb), NULL);
  g_signal_connect (dialog, "destroy", G_CALLBACK (close_cb), NULL);
  GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
  gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
  GtkWidget *close_b = gtk_button_new_with_label("Close");
  gtk_widget_set_name(close_b, "close_button");
  g_signal_connect (close_b, "button-press-event", G_CALLBACK(close_cb), NULL);
  gtk_grid_attach(GTK_GRID(grid), close_b, 0, 0, 1, 1);

  w=gtk_label_new("Meter Type:");
  gtk_widget_set_name(w, "boldlabel");
  gtk_widget_set_halign(w, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), w, 0, 1, 1, 1);

  w = gtk_combo_box_text_new();
  gtk_grid_attach(GTK_GRID(grid), w, 1, 1, 1, 1);
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(w), NULL, "Digital");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(w), NULL, "Analog");
  gtk_combo_box_set_active(GTK_COMBO_BOX(w), analog_meter ? 1 : 0);
  g_signal_connect(w, "changed", G_CALLBACK(analog_cb), NULL);

  w=gtk_label_new("S-Meter Reading:");
  gtk_widget_set_name(w, "boldlabel");
  gtk_widget_set_halign(w, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), w, 0, 2, 1, 1);

  w = gtk_combo_box_text_new();
  gtk_grid_attach(GTK_GRID(grid), w, 1, 2, 1, 1);
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(w), NULL, "Peak");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(w), NULL, "Average");
  switch (smeter) {
    case RXA_S_PK:
      gtk_combo_box_set_active(GTK_COMBO_BOX(w), 0);
      break;
    case RXA_S_AV:
      gtk_combo_box_set_active(GTK_COMBO_BOX(w), 0);
      break;
  }
  g_signal_connect(w, "changed", G_CALLBACK(smeter_select_cb), NULL);

  w=gtk_label_new("TX ALC Reading:");
  gtk_widget_set_name(w, "boldlabel");
  gtk_widget_set_halign(w, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), w, 0, 3, 1, 1);

  w = gtk_combo_box_text_new();
  gtk_grid_attach(GTK_GRID(grid), w, 1, 3, 1, 1);
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(w), NULL, "Peak");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(w), NULL, "Average");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(w), NULL, "Gain");
  switch (alc) {
    case TXA_ALC_PK:
      gtk_combo_box_set_active(GTK_COMBO_BOX(w), 0);
      break;
    case TXA_ALC_AV:
      gtk_combo_box_set_active(GTK_COMBO_BOX(w), 1);
      break;
    case TXA_ALC_GAIN:
      gtk_combo_box_set_active(GTK_COMBO_BOX(w), 1);
      break;
  }
  g_signal_connect(w, "changed", G_CALLBACK(alc_select_cb), NULL);


  gtk_container_add(GTK_CONTAINER(content), grid);
  sub_menu = dialog;
  gtk_widget_show_all(dialog);
}
