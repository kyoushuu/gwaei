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
//! @file matchwindow.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/matchwindow-private.h>


//Static declarations
static void gw_matchwindow_attach_signals (GwMatchWindow*);
static void gw_matchwindow_remove_signals (GwMatchWindow*);

static void gw_matchwindow_init_accelerators (GwMatchWindow*);

G_DEFINE_TYPE (GwMatchWindow, gw_matchwindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_matchwindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwMatchWindow *window;

    //Initializations
    window = GW_MATCHWINDOW (g_object_new (GW_TYPE_MATCHWINDOW,
                                           "type",        GTK_WINDOW_TOPLEVEL,
                                           "application", GW_APPLICATION (application),
                                           "ui-xml",      "matchwindow.ui",
                                           NULL));

    return GTK_WINDOW (window);
}


static void 
gw_matchwindow_init (GwMatchWindow *window)
{
    window->priv = GW_MATCHWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwMatchWindowPrivate));

    GwMatchWindowPrivate *priv;
    priv = window->priv;
}


static void 
gw_matchwindow_finalize (GObject *object)
{
    GwMatchWindow *window;
    GwMatchWindowPrivate *priv;

    window = GW_MATCHWINDOW (object);
    priv = window->priv;

    G_OBJECT_CLASS (gw_matchwindow_parent_class)->finalize (object);
}


static void 
gw_matchwindow_constructed (GObject *object)
{
    //Declarations
    GwMatchWindow *window;
    GwMatchWindowPrivate *priv;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_matchwindow_parent_class)->constructed (object);
    }

    //Initializations
    window = GW_MATCHWINDOW (object);
    priv = window->priv;

    //Set up the gtkbuilder links
    priv->left_treeview = GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "left_treeview"));
    priv->right_treeview = GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "right_treeview"));
    priv->correct_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "correct_label"));
    priv->incorrect_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "incorrect_label"));
    priv->progressbar = GTK_PROGRESS_BAR (gw_window_get_object (GW_WINDOW (window), "progressbar"));

    //Set up the gtk window
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
    gtk_window_set_default_size (GTK_WINDOW (window), 620, 500);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_title (GTK_WINDOW (window), gettext ("gWaei Vocabulary Match Study"));

    gw_matchwindow_init_accelerators (window);
    gw_matchwindow_attach_signals (window);
}


static void
gw_matchwindow_class_init (GwMatchWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gw_matchwindow_constructed;
  object_class->finalize = gw_matchwindow_finalize;

  g_type_class_add_private (object_class, sizeof (GwMatchWindowPrivate));
}


static void
gw_matchwindow_init_accelerators (GwMatchWindow *window)
{
    GtkWidget *widget;
    GtkAccelGroup *accelgroup;

    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));
/*
    //Set menu accelerators
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "new_window_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_N), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
*/
}


static void 
gw_matchwindow_attach_signals (GwMatchWindow *window)
{
}


static void 
gw_matchwindow_remove_signals (GwMatchWindow *window)
{
}

