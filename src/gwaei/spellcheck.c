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
//! @file spellcheck.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>
#include <enchant/enchant.h>

#include <gwaei/gwaei.h>
#include <gwaei/spellcheck-private.h>

static void gw_spellcheck_attach_signals (GwSpellcheck*);
static void gw_spellcheck_remove_signals (GwSpellcheck*);

G_DEFINE_TYPE (GwSpellcheck, gw_spellcheck, G_TYPE_OBJECT)


typedef enum
{
  PROP_0,
  PROP_APPLICATION
} GwSpellcheckProps;


GwSpellcheck*
gw_spellcheck_new (GwApplication *application)
{
    GwSpellcheck *spellcheck;

    spellcheck = GW_SPELLCHECK (g_object_new (GW_TYPE_SPELLCHECK, "application", application, NULL));

    return spellcheck;
}


GwSpellcheck* 
gw_spellcheck_new_with_entry (GwApplication *application, GtkEntry *entry)
{
    GwSpellcheck *spellcheck;
    
    spellcheck = gw_spellcheck_new (application);

    if (spellcheck != NULL)
      gw_spellcheck_set_entry (spellcheck, entry);

    return spellcheck;
}


static void
gw_spellcheck_init (GwSpellcheck *spellcheck)
{
    spellcheck->priv = GW_SPELLCHECK_GET_PRIVATE (spellcheck);
    memset(spellcheck->priv, 0, sizeof(GwSpellcheckPrivate));

    GwSpellcheckPrivate *priv;

    priv = spellcheck->priv;

    priv->broker = enchant_broker_init();

#ifdef OS_MINGW
    gchar *current_dir = g_get_current_dir();
    gchar *path = g_build_filename (current_dir, "..", "share", "enchant", "myspell", NULL);
    enchant_broker_set_param (priv->broker, "enchant.myspell.dictionary.path", path);
    if (path != NULL) g_free (path); path = NULL;
    if (current_dir != NULL) g_free (current_dir); current_dir = NULL;
#endif

    priv->dictionary = enchant_broker_request_dict (priv->broker, "en");

    gw_spellcheck_set_timeout_threshold (spellcheck, 3);

    gw_spellcheck_attach_signals (spellcheck);
}


static void
gw_spellcheck_finalize (GObject *object)
{
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;

    spellcheck = GW_SPELLCHECK (object);
    priv = spellcheck->priv;

    if (priv->dictionary != NULL) enchant_broker_free_dict (priv->broker, priv->dictionary); priv->dictionary = NULL;
    if (priv->broker != NULL) enchant_broker_free (priv->broker); priv->broker = NULL;

    gw_spellcheck_remove_signals (spellcheck);
    gw_spellcheck_clear (spellcheck);

    if (priv->entry != NULL) gtk_widget_queue_draw (GTK_WIDGET (priv->entry));

    G_OBJECT_CLASS (gw_spellcheck_parent_class)->finalize (object);
}


static void 
gw_spellcheck_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;

    spellcheck = GW_SPELLCHECK (object);
    priv = spellcheck->priv;

    switch (property_id)
    {
      case PROP_APPLICATION:
        priv->application = GW_APPLICATION (g_value_get_object (value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void 
gw_spellcheck_get_property (GObject      *object,
                            guint         property_id,
                            GValue       *value,
                            GParamSpec   *pspec)
{
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;

    spellcheck = GW_SPELLCHECK (object);
    priv = spellcheck->priv;

    switch (property_id)
    {
      case PROP_APPLICATION:
        g_value_set_object (value, priv->application);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void
gw_spellcheck_class_init (GwSpellcheckClass *klass)
{
    //Declarations
    GParamSpec *pspec;
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = gw_spellcheck_set_property;
    object_class->get_property = gw_spellcheck_get_property;
    object_class->finalize = gw_spellcheck_finalize;

    g_type_class_add_private (object_class, sizeof (GwSpellcheckPrivate));

    pspec = g_param_spec_object ("application",
                                 "Application construct prop",
                                 "Set GwSpellcheck's Application",
                                 GW_TYPE_APPLICATION,
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE
    );
    g_object_class_install_property (object_class, PROP_APPLICATION, pspec);
}


static void  
gw_spellcheck_attach_signals (GwSpellcheck *spellcheck)
{
}


static void 
gw_spellcheck_remove_signals (GwSpellcheck *spellcheck)
{
    //Declarations
    GwSpellcheckPrivate *priv;
    GtkEntry *entry;
    GSource *source;
    int i;

    priv = spellcheck->priv;
    entry = priv->entry;

    for (i = 0; i < TOTAL_GW_SPELLCHECK_SIGNALIDS && entry != NULL; i++)
    {
      if (priv->signalid[i] > 0)
      {
        g_signal_handler_disconnect (G_OBJECT (entry), priv->signalid[i]);
        priv->signalid[i] = 0;
      }
    }

    for (i = 0; i < TOTAL_GW_SPELLCHECK_TIMEOUTIDS; i++)
    {
      if (g_main_current_source () != NULL &&
          !g_source_is_destroyed (g_main_current_source ()) &&
          priv->timeoutid[i] > 0
         )
      {
        source = g_main_context_find_source_by_id (NULL, priv->timeoutid[i]);
        if (source != NULL)
        {
          g_source_destroy (source);
        }
      }
      priv->timeoutid[i] = 0;
    }
}


void
gw_spellcheck_set_timeout_threshold (GwSpellcheck *spellcheck, guint threshold)
{
   GwSpellcheckPrivate *priv;

   priv = spellcheck->priv;

   priv->threshold = threshold;
}


void
gw_spellcheck_set_entry (GwSpellcheck *spellcheck, GtkEntry *entry)
{

    GwSpellcheckPrivate *priv;

    priv = spellcheck->priv;

    //Remove the old signals
    if (priv->entry != NULL)
    {
      if (priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW] != 0)
        g_signal_handler_disconnect (G_OBJECT (priv->entry), priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW]);

      if (priv->signalid[GW_SPELLCHECK_SIGNALID_CHANGED] != 0)
        g_signal_handler_disconnect (G_OBJECT (priv->entry), priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW]);

      if (priv->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP] != 0)
        g_signal_handler_disconnect (G_OBJECT (priv->entry), priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW]);

      if (priv->signalid[GW_SPELLCHECK_SIGNALID_DESTROY] != 0)
        g_signal_handler_disconnect (G_OBJECT (priv->entry), priv->signalid[GW_SPELLCHECK_SIGNALID_DESTROY]);

      g_object_remove_weak_pointer (G_OBJECT (priv->entry), (gpointer*) (&(priv->entry)));
    }

    //Set the entry pointer
    priv->entry = entry;

    if (entry == NULL) return;

    g_object_add_weak_pointer (G_OBJECT (priv->entry), (gpointer*) (&(priv->entry)));

    //set the new signals
    priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW] = g_signal_connect_after (
        G_OBJECT (entry), 
        "draw", 
        G_CALLBACK (gw_spellcheck_draw_underline_cb), 
        spellcheck
    );
    priv->signalid[GW_SPELLCHECK_SIGNALID_CHANGED] = g_signal_connect (
        G_OBJECT (entry), 
        "changed", 
        G_CALLBACK (gw_spellcheck_queue_cb), 
        spellcheck
    );

    priv->signalid[GW_SPELLCHECK_SIGNALID_BUTTON_PRESS_EVENT] = g_signal_connect (
        G_OBJECT (entry), 
        "button-press-event", 
        G_CALLBACK (gw_spellcheck_button_press_event_cb), 
        spellcheck
    );

    priv->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP] = g_signal_connect (
        G_OBJECT (entry), 
        "populate-popup", 
        G_CALLBACK (gw_spellcheck_populate_popup_cb), 
        spellcheck
    );

    priv->signalid[GW_SPELLCHECK_SIGNALID_DESTROY] = g_signal_connect_swapped (
        G_OBJECT (entry), 
        "destroy", 
        G_CALLBACK (gw_spellcheck_remove_signals), 
        spellcheck
    );

    gw_spellcheck_queue (spellcheck);
}


void
gw_spellcheck_clear (GwSpellcheck *spellcheck)
{
    g_return_if_fail (spellcheck != NULL);

    GwSpellcheckPrivate *priv;
    priv = spellcheck->priv;

    priv->timeout = 0;
    
    if (priv->tolkens != NULL)
      g_strfreev (priv->tolkens); priv->tolkens = NULL;
    if (priv->misspelled != NULL)
      g_list_free (priv->misspelled); priv->misspelled = NULL;
}


gint
gw_spellcheck_get_layout_y_offset (GwSpellcheck *spellcheck)
{
    g_return_val_if_fail (spellcheck != NULL, 0);

    //Declarations
    GwSpellcheckPrivate *priv;
    PangoRectangle rect;
    PangoLayout *layout;
    gint layout_offset;

    //Initializations
    priv = spellcheck->priv;
    layout = gtk_entry_get_layout (priv->entry);
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    gtk_entry_get_layout_offsets (priv->entry, NULL, &layout_offset);

    return (layout_offset + 1);
}


gint
gw_spellcheck_get_layout_x_offset (GwSpellcheck *spellcheck)
{
    g_return_val_if_fail (spellcheck != NULL, 0);

    //Declarations
    GwSpellcheckPrivate *priv;
    PangoRectangle rect;
    PangoLayout *layout;
    int layout_offset;

    //Initializations
    priv = spellcheck->priv;
    layout = gtk_entry_get_layout (priv->entry);
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    gtk_entry_get_layout_offsets (priv->entry, &layout_offset, NULL);

    return (layout_offset);
}


void
gw_spellcheck_queue (GwSpellcheck *spellcheck)
{
    g_return_if_fail (spellcheck != NULL);

    //Declarations
    GwSpellcheckPrivate *priv;
    LwPreferences *preferences;
    gboolean spellcheck_pref;
    int rk_conv_pref;
    gboolean want_conv;
    const char *query;
    gboolean is_convertable_to_hiragana;
    const int MAX = 300;
    char kana[MAX];
    gboolean needs_spellcheck;
    gboolean should_redraw;

    //Initializations
    priv = spellcheck->priv;
    preferences = gw_application_get_preferences (priv->application);
    rk_conv_pref = lw_preferences_get_int_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_ROMAN_KANA);
    want_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !lw_util_is_japanese_locale()));
    query = gtk_entry_get_text (priv->entry);
    is_convertable_to_hiragana = (want_conv && lw_util_str_roma_to_hira (query, kana, MAX));
    spellcheck_pref = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK);
    needs_spellcheck = (query != NULL && *query != '\0' && spellcheck_pref && !is_convertable_to_hiragana);
    should_redraw = (priv->misspelled != NULL);
    g_return_if_fail (enchant_broker_dict_exists (priv->broker, "en") != FALSE);

    if (needs_spellcheck)
    {
      priv->timeout = 0;
      if (priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] == 0)
        priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] = g_timeout_add_full (
          G_PRIORITY_LOW, 
          100, (GSourceFunc) 
          gw_spellcheck_update_timeout, 
          spellcheck, 
          NULL
      );
    }

    gw_spellcheck_clear (spellcheck);

    if (should_redraw) 
    {
      gtk_widget_queue_draw (GTK_WIDGET (priv->entry));
    }
}


gboolean
gw_spellcheck_update (GwSpellcheck *spellcheck)
{
    GwSpellcheckPrivate *priv;

    priv = spellcheck->priv;

    if (priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] == 0)
    {
      return FALSE;
    }
    else if (priv->timeout < priv->threshold) 
    {
      priv->timeout++;
      return TRUE;
    }

    gw_spellcheck_clear (spellcheck); //Make sure the memory is freed

    const gchar *query;
    gchar **iter;

    query = gtk_entry_get_text (priv->entry);
    priv->tolkens = g_strsplit (query, " ", -1);

    for (iter = priv->tolkens; *iter != NULL; iter++)
    {
      if (**iter != '\0' && enchant_dict_check (priv->dictionary, *iter, strlen(*iter)))
      {
        priv->misspelled = g_list_append (priv->misspelled, *iter);
      }
    }

    priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] = 0;
    priv->timeout = 0;

    gtk_widget_queue_draw (GTK_WIDGET (priv->entry));

    return FALSE;
}


void
gw_spellcheck_record_mouse_cordinates (GwSpellcheck *spellcheck, GdkEvent *event)
{
    GwSpellcheckPrivate *priv;
    GtkWidget *toplevel;
    gint toplevel_x, toplevel_y;

    priv = spellcheck->priv;
    toplevel = GTK_WIDGET (gtk_widget_get_ancestor (GTK_WIDGET (priv->entry), GTK_TYPE_WINDOW));

    gdk_window_get_device_position (
      gtk_widget_get_window (GTK_WIDGET (priv->entry)),
      gdk_event_get_device (event),
      &toplevel_x, 
      &toplevel_y, 
      NULL
    );

    gtk_widget_translate_coordinates (
      toplevel, 
      GTK_WIDGET (priv->entry), 
      toplevel_x, 
      toplevel_y, 
      &priv->x, 
      &priv->y
    );
}


static int 
_get_string_index (GtkEntry *entry, int x, int y)
{
    //Declarations
    int layout_index;
    int entry_index;
    int trailing;
    PangoLayout *layout;

    //Initalizations
    layout = gtk_entry_get_layout (GTK_ENTRY (entry));
    if (pango_layout_xy_to_index (layout, x * PANGO_SCALE, y * PANGO_SCALE, &layout_index, &trailing))
      entry_index = gtk_entry_layout_index_to_text_index (GTK_ENTRY (entry), layout_index);
    else
      entry_index = -1;

    return entry_index;
}


void 
gw_spellcheck_populate_popup (GwSpellcheck *spellcheck, GtkMenu *menu)
{
    //Declarations
    GwSpellcheckPrivate *priv;
    GtkWidget *menuitem;

    priv = spellcheck->priv;
    int index;
    int xoffset, yoffset, x, y;
    int start_offset, end_offset;
    int i;
    gchar **iter;
    gchar **suggestions;
    size_t total_suggestions;

    if (priv->tolkens == NULL) return;
    g_return_if_fail (enchant_broker_dict_exists (priv->broker, "en") != FALSE);

    xoffset = gw_spellcheck_get_layout_x_offset (spellcheck);
    yoffset = gw_spellcheck_get_layout_y_offset (spellcheck);
    x = priv->x - xoffset;
    y = priv->y - yoffset; //Since a GtkEntry is single line, we want the y to always be in the area
    index =  _get_string_index (priv->entry, x, y);

    start_offset = 0;
    iter = priv->tolkens;
    while (*iter != NULL && start_offset + strlen(*iter) < index)
    {
      start_offset += strlen(*iter) + 1;
      iter++;
    }
    if (*iter == NULL) return;
    end_offset = start_offset + strlen(*iter);

    suggestions = enchant_dict_suggest (priv->dictionary, *iter, strlen(*iter), &total_suggestions);
    if (total_suggestions > 0 && suggestions != NULL)
    {
      menuitem = gtk_separator_menu_item_new ();
      gtk_widget_show (GTK_WIDGET (menuitem));
      gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);

      //Menuitems
      for (i = 0; i < total_suggestions; i++)
      {
        menuitem = gtk_menu_item_new_with_label (suggestions[i]);
        g_object_set_data (G_OBJECT (menuitem), "start-offset", GINT_TO_POINTER (start_offset));
        g_object_set_data (G_OBJECT (menuitem), "end-offset", GINT_TO_POINTER (end_offset));
        g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (gw_spellcheck_menuitem_activated_cb), spellcheck);
        gtk_widget_show (GTK_WIDGET (menuitem));
        gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
      }

      enchant_dict_free_string_list (priv->dictionary, suggestions);
    }
}



