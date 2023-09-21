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

#include <gtk/gtk.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <wdsp.h>

#include "main.h"
#include "new_menu.h"
#include "display_menu.h"
#include "radio.h"

static GtkWidget *dialog = NULL;
static GtkWidget *waterfall_high_r = NULL;
static GtkWidget *waterfall_low_r = NULL;

static void cleanup() {
  if (dialog != NULL) {
    GtkWidget *tmp = dialog;
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

static void detector_cb(GtkToggleButton *widget, gpointer data) {
  int val = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));

  switch (val) {
  case 0:
    display_detector_mode = DETECTOR_MODE_PEAK;
    break;

  case 1:
    display_detector_mode = DETECTOR_MODE_ROSENFELL;
    break;

  case 2:
    display_detector_mode = DETECTOR_MODE_AVERAGE;
    break;

  case 3:
    display_detector_mode = DETECTOR_MODE_SAMPLE;
    break;
  }

  SetDisplayDetectorMode(active_receiver->id, 0, display_detector_mode);
}

static void average_cb(GtkToggleButton *widget, gpointer data) {
  int val = gtk_combo_box_get_active (GTK_COMBO_BOX(widget));

  switch (val) {
  case 0:
    display_average_mode = AVERAGE_MODE_NONE;
    break;

  case 1:
    display_average_mode = AVERAGE_MODE_RECURSIVE;
    break;

  case 2:
    display_average_mode = AVERAGE_MODE_TIME_WINDOW;
    break;

  case 3:
    display_average_mode = AVERAGE_MODE_LOG_RECURSIVE;
    break;
  }

  //
  // I observed artifacts when changing the mode from "Log Recursive"
  // to "Time Window", so I generally switch to NONE first, and then
  // to the target averaging mode
  //
  SetDisplayAverageMode(active_receiver->id, 0, AVERAGE_MODE_NONE);
  usleep(50000);
  SetDisplayAverageMode(active_receiver->id, 0, display_average_mode);
  calculate_display_average(active_receiver);
}

static void time_value_changed_cb(GtkWidget *widget, gpointer data) {
  display_average_time = gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget));
  calculate_display_average(active_receiver);
}

static void filled_cb(GtkWidget *widget, gpointer data) {
  display_filled = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void gradient_cb(GtkWidget *widget, gpointer data) {
  display_gradient = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void frames_per_second_value_changed_cb(GtkWidget *widget, gpointer data) {
  updates_per_second = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
  active_receiver->fps = updates_per_second;
  calculate_display_average(active_receiver);
  set_displaying(active_receiver, 1);
}

static void panadapter_high_value_changed_cb(GtkWidget *widget, gpointer data) {
  active_receiver->panadapter_high = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

static void panadapter_low_value_changed_cb(GtkWidget *widget, gpointer data) {
  active_receiver->panadapter_low = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

static void panadapter_step_value_changed_cb(GtkWidget *widget, gpointer data) {
  active_receiver->panadapter_step = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

static void waterfall_high_value_changed_cb(GtkWidget *widget, gpointer data) {
  active_receiver->waterfall_high = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

static void waterfall_low_value_changed_cb(GtkWidget *widget, gpointer data) {
  active_receiver->waterfall_low = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
}

static void waterfall_automatic_cb(GtkWidget *widget, gpointer data) {
  int val = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  active_receiver->waterfall_automatic = val;
  gtk_widget_set_sensitive(waterfall_high_r, !val);
  gtk_widget_set_sensitive(waterfall_low_r, !val);
}

static void display_waterfall_cb(GtkWidget *widget, gpointer data) {
  active_receiver->display_waterfall = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  reconfigure_radio();
}

static void display_zoompan_cb(GtkWidget *widget, gpointer data) {
  display_zoompan = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  reconfigure_radio();
}

static void display_sliders_cb(GtkWidget *widget, gpointer data) {
  display_sliders = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  reconfigure_radio();
}


static void display_toolbar_cb(GtkWidget *widget, gpointer data) {
  display_toolbar = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  reconfigure_radio();
}

static void display_sequence_errors_cb(GtkWidget *widget, gpointer data) {
  display_sequence_errors = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

void display_menu(GtkWidget *parent) {
  GtkWidget *label;
  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
  gtk_window_set_title(GTK_WINDOW(dialog), "piHPSDR - Display");
  g_signal_connect (dialog, "delete_event", G_CALLBACK (close_cb), NULL);
  g_signal_connect (dialog, "destroy", G_CALLBACK (close_cb), NULL);
  GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  GtkWidget *grid = gtk_grid_new();
  gtk_grid_set_column_spacing (GTK_GRID(grid), 10);
  gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
  int col = 0;
  int row = 0;
  GtkWidget *close_b = gtk_button_new_with_label("Close");
  gtk_widget_set_name(close_b, "close_button");
  g_signal_connect (close_b, "button-press-event", G_CALLBACK(close_cb), NULL);
  gtk_grid_attach(GTK_GRID(grid), close_b, col, row, 1, 1);
  row++;
  col = 0;
  label = gtk_label_new("Frames Per Second:");
  gtk_widget_set_name (label, "boldlabel");
  gtk_widget_set_halign(label, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);
  col++;
  GtkWidget *frames_per_second_r = gtk_spin_button_new_with_range(1.0, 100.0, 1.0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(frames_per_second_r), (double)updates_per_second);
  gtk_widget_show(frames_per_second_r);
  gtk_grid_attach(GTK_GRID(grid), frames_per_second_r, col, row, 1, 1);
  g_signal_connect(frames_per_second_r, "value_changed", G_CALLBACK(frames_per_second_value_changed_cb), NULL);
  row++;
  col = 0;
  label = gtk_label_new("Panadapter High:");
  gtk_widget_set_name (label, "boldlabel");
  gtk_widget_set_halign(label, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);
  col++;
  GtkWidget *panadapter_high_r = gtk_spin_button_new_with_range(-220.0, 100.0, 1.0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(panadapter_high_r), (double)active_receiver->panadapter_high);
  gtk_widget_show(panadapter_high_r);
  gtk_grid_attach(GTK_GRID(grid), panadapter_high_r, col, row, 1, 1);
  g_signal_connect(panadapter_high_r, "value_changed", G_CALLBACK(panadapter_high_value_changed_cb), NULL);
  row++;
  col = 0;
  label = gtk_label_new("Panadapter Low:");
  gtk_widget_set_name (label, "boldlabel");
  gtk_widget_set_halign(label, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);
  col++;
  GtkWidget *panadapter_low_r = gtk_spin_button_new_with_range(-220.0, 100.0, 1.0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(panadapter_low_r), (double)active_receiver->panadapter_low);
  gtk_widget_show(panadapter_low_r);
  gtk_grid_attach(GTK_GRID(grid), panadapter_low_r, col, row, 1, 1);
  g_signal_connect(panadapter_low_r, "value_changed", G_CALLBACK(panadapter_low_value_changed_cb), NULL);
  row++;
  col = 0;
  label = gtk_label_new("Panadapter Step:");
  gtk_widget_set_name (label, "boldlabel");
  gtk_widget_set_halign(label, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);
  col++;
  GtkWidget *panadapter_step_r = gtk_spin_button_new_with_range(1.0, 20.0, 1.0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(panadapter_step_r), (double)active_receiver->panadapter_step);
  gtk_widget_show(panadapter_step_r);
  gtk_grid_attach(GTK_GRID(grid), panadapter_step_r, col, row, 1, 1);
  g_signal_connect(panadapter_step_r, "value_changed", G_CALLBACK(panadapter_step_value_changed_cb), NULL);
  row++;
  col = 0;
  label = gtk_label_new("Waterfall High:");
  gtk_widget_set_name (label, "boldlabel");
  gtk_widget_set_halign(label, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);
  col++;
  waterfall_high_r = gtk_spin_button_new_with_range(-220.0, 100.0, 1.0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(waterfall_high_r), (double)active_receiver->waterfall_high);
  gtk_widget_show(waterfall_high_r);
  gtk_grid_attach(GTK_GRID(grid), waterfall_high_r, col, row, 1, 1);
  g_signal_connect(waterfall_high_r, "value_changed", G_CALLBACK(waterfall_high_value_changed_cb), NULL);
  row++;
  col = 0;
  label = gtk_label_new("Waterfall Low:");
  gtk_widget_set_name (label, "boldlabel");
  gtk_widget_set_halign(label, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);
  col++;
  waterfall_low_r = gtk_spin_button_new_with_range(-220.0, 100.0, 1.0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(waterfall_low_r), (double)active_receiver->waterfall_low);
  gtk_widget_show(waterfall_low_r);
  gtk_grid_attach(GTK_GRID(grid), waterfall_low_r, col, row, 1, 1);
  g_signal_connect(waterfall_low_r, "value_changed", G_CALLBACK(waterfall_low_value_changed_cb), NULL);
  row++;
  col = 0;
  label = gtk_label_new("Waterfall Automatic:");
  gtk_widget_set_name (label, "boldlabel");
  gtk_widget_set_halign(label, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);
  col++;
  GtkWidget *waterfall_automatic_b = gtk_check_button_new();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (waterfall_automatic_b), active_receiver->waterfall_automatic);
  gtk_widget_show(waterfall_automatic_b);
  gtk_grid_attach(GTK_GRID(grid), waterfall_automatic_b, col, row, 1, 1);
  g_signal_connect(waterfall_automatic_b, "toggled", G_CALLBACK(waterfall_automatic_cb), NULL);
  col = 2;
  row = 1;
  label = gtk_label_new("Detector:");
  gtk_widget_set_name (label, "boldlabel");
  gtk_widget_set_halign(label, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);
  GtkWidget *detector_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(detector_combo), NULL, "Peak");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(detector_combo), NULL, "Rosenfell");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(detector_combo), NULL, "Average");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(detector_combo), NULL, "Sample");

  switch (display_detector_mode) {
  case DETECTOR_MODE_PEAK:
    gtk_combo_box_set_active(GTK_COMBO_BOX(detector_combo), 0);
    break;

  case DETECTOR_MODE_ROSENFELL:
    gtk_combo_box_set_active(GTK_COMBO_BOX(detector_combo), 1);
    break;

  case DETECTOR_MODE_AVERAGE:
    gtk_combo_box_set_active(GTK_COMBO_BOX(detector_combo), 2);
    break;

  case DETECTOR_MODE_SAMPLE:
    gtk_combo_box_set_active(GTK_COMBO_BOX(detector_combo), 2);
    break;
  }

  my_combo_attach(GTK_GRID(grid), detector_combo, col + 1, row, 1, 1);
  g_signal_connect(detector_combo, "changed", G_CALLBACK(detector_cb), NULL);
  row++;
  label = gtk_label_new("Averaging: ");
  gtk_widget_set_name (label, "boldlabel");
  gtk_widget_set_halign(label, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);
  GtkWidget *average_combo = gtk_combo_box_text_new();
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(average_combo), NULL, "None");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(average_combo), NULL, "Recursive");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(average_combo), NULL, "Time Window");
  gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(average_combo), NULL, "Log Recursive");

  switch (display_detector_mode) {
  case AVERAGE_MODE_NONE:
    gtk_combo_box_set_active(GTK_COMBO_BOX(average_combo), 0);
    break;

  case AVERAGE_MODE_RECURSIVE:
    gtk_combo_box_set_active(GTK_COMBO_BOX(average_combo), 1);
    break;

  case AVERAGE_MODE_TIME_WINDOW:
    gtk_combo_box_set_active(GTK_COMBO_BOX(average_combo), 2);
    break;

  case AVERAGE_MODE_LOG_RECURSIVE:
    gtk_combo_box_set_active(GTK_COMBO_BOX(average_combo), 2);
    break;
  }

  my_combo_attach(GTK_GRID(grid), average_combo, col + 1, row, 1, 1);
  g_signal_connect(average_combo, "changed", G_CALLBACK(average_cb), NULL);
  row++;
  label = gtk_label_new("Av. Time (ms):");
  gtk_widget_set_name (label, "boldlabel");
  gtk_widget_set_halign(label, GTK_ALIGN_END);
  gtk_grid_attach(GTK_GRID(grid), label, col, row, 1, 1);
  GtkWidget *time_r = gtk_spin_button_new_with_range(1.0, 9999.0, 1.0);
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(time_r), (double)display_average_time);
  gtk_widget_show(time_r);
  gtk_grid_attach(GTK_GRID(grid), time_r, col + 1, row, 1, 1);
  g_signal_connect(time_r, "value_changed", G_CALLBACK(time_value_changed_cb), NULL);
  row++;
  GtkWidget *filled_b = gtk_check_button_new_with_label("Fill Panadapter");
  gtk_widget_set_name (filled_b, "boldlabel");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (filled_b), display_filled);
  gtk_widget_show(filled_b);
  gtk_grid_attach(GTK_GRID(grid), filled_b, col, row, 1, 1);
  g_signal_connect(filled_b, "toggled", G_CALLBACK(filled_cb), NULL);
  GtkWidget *gradient_b = gtk_check_button_new_with_label("Gradient");
  gtk_widget_set_name (gradient_b, "boldlabel");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (gradient_b), display_gradient);
  gtk_widget_show(gradient_b);
  gtk_grid_attach(GTK_GRID(grid), gradient_b, col + 1, row, 1, 1);
  g_signal_connect(gradient_b, "toggled", G_CALLBACK(gradient_cb), NULL);
  row++;
  GtkWidget *b_display_waterfall = gtk_check_button_new_with_label("Display Waterfall");
  gtk_widget_set_name (b_display_waterfall, "boldlabel");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b_display_waterfall), active_receiver->display_waterfall);
  gtk_widget_show(b_display_waterfall);
  gtk_grid_attach(GTK_GRID(grid), b_display_waterfall, col, row, 1, 1);
  g_signal_connect(b_display_waterfall, "toggled", G_CALLBACK(display_waterfall_cb), (gpointer)NULL);
  GtkWidget *b_display_zoompan = gtk_check_button_new_with_label("Display Zoom/Pan");
  gtk_widget_set_name (b_display_zoompan, "boldlabel");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b_display_zoompan), display_zoompan);
  gtk_widget_show(b_display_zoompan);
  gtk_grid_attach(GTK_GRID(grid), b_display_zoompan, col + 1, row, 1, 1);
  g_signal_connect(b_display_zoompan, "toggled", G_CALLBACK(display_zoompan_cb), (gpointer)NULL);
  row++;
  GtkWidget *b_display_sliders = gtk_check_button_new_with_label("Display Sliders");
  gtk_widget_set_name (b_display_sliders, "boldlabel");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b_display_sliders), display_sliders);
  gtk_widget_show(b_display_sliders);
  gtk_grid_attach(GTK_GRID(grid), b_display_sliders, col, row, 1, 1);
  g_signal_connect(b_display_sliders, "toggled", G_CALLBACK(display_sliders_cb), (gpointer)NULL);
  GtkWidget *b_display_toolbar = gtk_check_button_new_with_label("Display Toolbar");
  gtk_widget_set_name (b_display_toolbar, "boldlabel");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b_display_toolbar), display_toolbar);
  gtk_widget_show(b_display_toolbar);
  gtk_grid_attach(GTK_GRID(grid), b_display_toolbar, col + 1, row, 1, 1);
  g_signal_connect(b_display_toolbar, "toggled", G_CALLBACK(display_toolbar_cb), (gpointer)NULL);
  row++;
  GtkWidget *b_display_sequence_errors = gtk_check_button_new_with_label("Display Seq Errs");
  gtk_widget_set_name (b_display_sequence_errors, "boldlabel");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (b_display_sequence_errors), display_sequence_errors);
  gtk_widget_show(b_display_sequence_errors);
  gtk_grid_attach(GTK_GRID(grid), b_display_sequence_errors, col, row, 1, 1);
  g_signal_connect(b_display_sequence_errors, "toggled", G_CALLBACK(display_sequence_errors_cb), (gpointer)NULL);
  gtk_container_add(GTK_CONTAINER(content), grid);
  sub_menu = dialog;

  if (active_receiver->waterfall_automatic) {
    gtk_widget_set_sensitive(waterfall_high_r, FALSE);
    gtk_widget_set_sensitive(waterfall_low_r,  FALSE);
  }

  gtk_widget_show_all(dialog);
}

