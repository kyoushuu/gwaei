/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file application-callbacks.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <gtk/gtk.h>

#include <gwaei/gettext.h>
#include <gwaei/gwaei.h>
#include <gwaei/application-private.h>


void 
gw_application_quit_cb (GSimpleAction *action,
                        GVariant      *parameter,
                        gpointer       data)
{
    gw_application_quit (GW_APPLICATION (data));
}


void 
gw_application_open_settingswindow_cb (GSimpleAction *action,
                                       GVariant      *parameter,
                                       gpointer       data)
{
    //Declarations
    GwApplication *application;
    GwSearchWindow *searchwindow;
    GtkWindow *settingswindow;
    GList *link;

    //Initializations
    searchwindow = GW_SEARCHWINDOW (gw_application_get_last_focused_searchwindow (GW_APPLICATION (data)));
    application = gw_window_get_application (GW_WINDOW (searchwindow));
    link = gtk_application_get_windows (GTK_APPLICATION (application));

    while (link != NULL && !GW_IS_SETTINGSWINDOW (link->data)) link = link->next;

    if (link != NULL)
    {
      settingswindow = GTK_WINDOW (link->data);
      gtk_window_set_transient_for (GTK_WINDOW (settingswindow), GTK_WINDOW (searchwindow));
      gtk_window_present (GTK_WINDOW (settingswindow));
    }
    else
    {
      settingswindow = gw_settingswindow_new (GTK_APPLICATION (application));
      gtk_window_set_transient_for (GTK_WINDOW (settingswindow), GTK_WINDOW (searchwindow));
      gtk_widget_show (GTK_WIDGET (settingswindow));
    }
}


//!
//! @brief Opens the gWaei about dialog
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void 
gw_application_open_aboutdialog_cb (GSimpleAction *action, 
                                    GVariant      *parameter,
                                    gpointer       data)
{
    gchar *global_path = DATADIR2 G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "logo.png";
    gchar *local_path = ".." G_DIR_SEPARATOR_S "share" G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "logo.png";

    gchar *programmer_credits[] = 
    {
      "Zachary Dovel <pizzach@gmail.com>",
      "Fabrizio Sabatini",
      NULL
    };

    GdkPixbuf *logo;
    if ( (logo = gdk_pixbuf_new_from_file (global_path,    NULL)) == NULL &&
         (logo = gdk_pixbuf_new_from_file (local_path, NULL)) == NULL    )
    {
      printf ("Was unable to load the gwaei logo.\n");
    }

    GtkWidget *about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
               "program-name", "gWaei", 
               "version", VERSION,
               "copyright", "gWaei (C) 2008-2012 Zachary Dovel\n" 
                            "Kanjipad backend (C) 2002 Owen Taylor\n"
                            "JStroke backend (C) 1997 Robert Wells",
               "comments", gettext("Program for Japanese translation and reference. The\n"
                                    "dictionaries are supplied by Jim Breen's WWWJDIC.\n"
                                    "Special thanks to the maker of GJITEN who served as an inspiration."),
               "license", "This software is GPL Licensed.\n\n"
                          "gWaei is free software: you can redistribute it and/or modify\n"
                          "it under the terms of the GNU General Public License as published by\n "
                          "the Free Software Foundation, either version 3 of the License, or\n"
                          "(at your option) any later version.\n\n"
                          "gWaei is distributed in the hope that it will be useful,\n"
                          "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                          "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
                          "GNU General Public License for more details.\n\n"
                          "You should have received a copy of the GNU General Public License\n"
                          "along with gWaei.  If not, see <http://www.gnu.org/licenses/>.",
               "logo", logo,
               // TRANSLATORS: You can add your own name to the translation of this field, it will be displayed in the "about" box when gwaei is run in your language
               "translator-credits", gettext("translator-credits"),
               "authors", programmer_credits,
               "website", "http://gwaei.sourceforge.net/",
               NULL);
    gtk_dialog_run (GTK_DIALOG (about));
    g_object_unref (logo);
    gtk_widget_destroy (about);
}


