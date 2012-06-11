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
