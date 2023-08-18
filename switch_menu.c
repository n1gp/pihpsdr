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
#include <glib/gprintf.h>
#include <stdio.h>
#include <string.h>

#include "main.h"
#include "new_menu.h"
#include "agc_menu.h"
#include "agc.h"
#include "band.h"
#include "channel.h"
#include "radio.h"
#include "receiver.h"
#include "vfo.h"
#include "button_text.h"
#include "toolbar.h"
#include "actions.h"
#include "action_dialog.h"
#include "gpio.h"
#include "i2c.h"

static GtkWidget *dialog = NULL;

static SWITCH *temp_switches;

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

static gboolean switch_cb(GtkWidget *widget, GdkEvent *event, gpointer data) {
  int sw = GPOINTER_TO_INT(data);
  int action = action_dialog(dialog, CONTROLLER_SWITCH, temp_switches[sw].switch_function);
  gtk_button_set_label(GTK_BUTTON(widget), ActionTable[action].str);
  temp_switches[sw].switch_function = action;
  update_toolbar_labels();
  return TRUE;
}

void switch_menu(GtkWidget *parent) {
  gint row;
  gint col;
  GtkWidget *grid;
  GtkWidget *widget;
  dialog = gtk_dialog_new();
  gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent));
  gtk_window_set_title(GTK_WINDOW(dialog), "piHPSDR - Switch Actions");
  g_signal_connect (dialog, "delete_event", G_CALLBACK (close_cb), NULL);
  g_signal_connect (dialog, "destroy", G_CALLBACK (close_cb), NULL);
  GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  grid = gtk_grid_new();
  gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
  gtk_grid_set_row_homogeneous(GTK_GRID(grid), FALSE);
  gtk_grid_set_column_spacing (GTK_GRID(grid), 0);
  gtk_grid_set_row_spacing (GTK_GRID(grid), 0);
  GtkWidget *close_b = gtk_button_new_with_label("Close");
  gtk_widget_set_name(close_b, "close_button");
  g_signal_connect (close_b, "button-press-event", G_CALLBACK(close_cb), NULL);
  gtk_grid_attach(GTK_GRID(grid), close_b, 0, 0, 2, 1);
  row = 1;
  col = 0;

  switch (controller) {
  case NO_CONTROLLER:
  case CONTROLLER1:
    //
    // NOTREACHED
    // Controller1 switches are assigned via the "Toolbar" menu
    //
    break;

  case CONTROLLER2_V1:
    temp_switches = switches_controller2_v1;
    break;

  case CONTROLLER2_V2:
    temp_switches = switches_controller2_v2;
    break;

  case G2_FRONTPANEL:
    temp_switches = switches_g2_frontpanel;
    break;
  }

  if (controller == CONTROLLER2_V1 || controller == CONTROLLER2_V2) {
    row = row + 5;
    col = 0;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[0].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(0));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    col++;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[1].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(1));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    col++;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[2].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(2));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    col++;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[3].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(3));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    col++;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[4].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(4));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    col++;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[5].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(5));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    col++;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[6].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(6));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    row = 1;
    col = 8;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[7].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(7));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    row++;
    col = 7;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[8].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(8));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    col++;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[9].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(9));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    row++;
    col = 7;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[10].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(10));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    col++;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[11].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(11));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    row++;
    col = 7;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[12].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(12));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    col++;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[13].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(13));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    row++;
    col = 7;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[14].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(14));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    col++;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[15].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(15));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    gtk_container_add(GTK_CONTAINER(content), grid);
  }

  if (controller == G2_FRONTPANEL) {
    row = 3;
    col = 0;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[11].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(11));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    row++;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[13].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(13));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    // padding
    row = 2;
    col = 1;
    widget = gtk_label_new(NULL);
    gtk_widget_set_name(widget, "small_button");
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    row = 2;
    col = 2;
    widget = gtk_label_new(NULL);
    gtk_widget_set_name(widget, "small_button");
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    row = 1;
    col = 6;

    for (int i = 10; i > 7; i--) {
      widget = gtk_button_new_with_label(ActionTable[temp_switches[i].switch_function].str);
      gtk_widget_set_name(widget, "small_button");
      g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(i));
      gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
      col++;
    }

    row = 2;
    col = 6;
    int a[3] = {7, 15, 14};

    for (int i = 0; i < 3; i++) {
      widget = gtk_button_new_with_label(ActionTable[temp_switches[a[i]].switch_function].str);
      gtk_widget_set_name(widget, "small_button");
      g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(a[i]));
      gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
      col++;
    }

    row = 3;
    col = 6;
    int b[3] = {6, 5, 3};

    for (int i = 0; i < 3; i++) {
      widget = gtk_button_new_with_label(ActionTable[temp_switches[b[i]].switch_function].str);
      gtk_widget_set_name(widget, "small_button");
      g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(b[i]));
      gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
      col++;
    }

    row = 4;
    col = 6;

    for (int i = 2; i > -1; i--) {
      widget = gtk_button_new_with_label(ActionTable[temp_switches[i].switch_function].str);
      gtk_widget_set_name(widget, "small_button");
      g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(i));
      gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
      col++;
    }

    // padding
    row = 5;
    col = 6;
    widget = gtk_label_new(NULL);
    gtk_widget_set_name(widget, "small_button");
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    row = 6;
    col = 6;
    widget = gtk_label_new(NULL);
    gtk_widget_set_name(widget, "small_button");
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    row = 7;
    col = 6;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[12].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(12));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    col += 2;
    widget = gtk_button_new_with_label(ActionTable[temp_switches[4].switch_function].str);
    gtk_widget_set_name(widget, "small_button");
    g_signal_connect(widget, "button-press-event", G_CALLBACK(switch_cb), GINT_TO_POINTER(4));
    gtk_grid_attach(GTK_GRID(grid), widget, col, row, 1, 1);
    gtk_container_add(GTK_CONTAINER(content), grid);
  }

  sub_menu = dialog;
  gtk_widget_show_all(dialog);
}
