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
    priv->mutex = g_mutex_new ();

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

    gw_spellcheck_remove_signals (spellcheck);
    gw_spellcheck_reset (spellcheck);

    if (priv->entry != NULL) gtk_widget_queue_draw (GTK_WIDGET (priv->entry));
    if (priv->thread != NULL) g_thread_join (priv->thread); priv->thread = NULL;
    if (priv->mutex != NULL) g_mutex_free (priv->mutex); priv->mutex = NULL;

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


GwSpellcheckStatus 
gw_spellcheck_get_status (GwSpellcheck *spellcheck)
{
  GwSpellcheckPrivate *priv = spellcheck->priv;
  GwSpellcheckStatus status;

  g_mutex_lock (priv->mutex);
  status = priv->status;
  g_mutex_unlock (priv->mutex);
  return status;
}


void 
gw_spellcheck_set_status (GwSpellcheck *spellcheck, GwSpellcheckStatus status)
{
  GwSpellcheckPrivate *priv = spellcheck->priv;

  g_mutex_lock (priv->mutex);
  priv->status = status;
  g_mutex_unlock (priv->mutex);
}


void
gw_spellcheck_set_entry (GwSpellcheck *spellcheck, GtkEntry *entry)
{

    GwSpellcheckPrivate *priv;

    priv = spellcheck->priv;

    //Remove the old signals
    if (entry != NULL && priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW] != 0)
      g_signal_handler_disconnect (G_OBJECT (entry), priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW]);

    if (entry != NULL && priv->signalid[GW_SPELLCHECK_SIGNALID_CHANGED] != 0)
      g_signal_handler_disconnect (G_OBJECT (entry), priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW]);

    if (entry != NULL && priv->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP] != 0)
      g_signal_handler_disconnect (G_OBJECT (entry), priv->signalid[GW_SPELLCHECK_SIGNALID_DRAW]);

    if (entry != NULL && priv->signalid[GW_SPELLCHECK_SIGNALID_DESTROY] != 0)
    g_signal_handler_disconnect (G_OBJECT (entry), priv->signalid[GW_SPELLCHECK_SIGNALID_DESTROY]);

    //Set the entry pointer
    priv->entry = entry;

    if (entry == NULL) return;

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
    priv->signalid[GW_SPELLCHECK_SIGNALID_POPULATE_POPUP] = g_signal_connect (
        G_OBJECT (entry), 
        "populate-popup", 
        G_CALLBACK (gw_spellcheck_populate_cb), 
        spellcheck
    );

    priv->signalid[GW_SPELLCHECK_SIGNALID_DESTROY] = g_signal_connect_swapped (
        G_OBJECT (entry), 
        "destroy", 
        G_CALLBACK (gw_spellcheck_remove_signals), 
        spellcheck
    );

    gw_spellcheck_start_check (spellcheck);
}


void
gw_spellcheck_reset (GwSpellcheck *spellcheck)
{
    GwSpellcheckPrivate *priv;
    GList *link;

    priv = spellcheck->priv;

    if (priv->corrections == NULL) return;

    g_mutex_lock (priv->mutex);
    for (link = priv->corrections; link != NULL; link = link->next)
      g_free (link->data);
    g_list_free (priv->corrections); priv->corrections = NULL;
    g_mutex_unlock (priv->mutex);
}


gint
gw_spellcheck_get_y_offset (GwSpellcheck *spellcheck)
{
    //Declarations
    GwSpellcheckPrivate *priv;
    PangoRectangle rect;
    PangoLayout *layout;

    int allocation_offset;
    int layout_offset;
    int rect_offset;

    //Initializations
    priv = spellcheck->priv;
    layout = gtk_entry_get_layout (priv->entry);
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    rect_offset = rect.height;
    allocation_offset = gtk_widget_get_allocated_height (GTK_WIDGET (priv->entry));
    gtk_entry_get_layout_offsets (priv->entry, NULL, &layout_offset);

    return (((allocation_offset - rect_offset) / 2) - layout_offset);
}


gint
gw_spellcheck_get_x_offset (GwSpellcheck *spellcheck)
{
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


static gpointer 
gw_spellcheck_infunc (gpointer data)
{
    //Declarations
    GwSpellcheckStreamWithData *swd;
    FILE *file;
    int stream;
    char *text;

    //Initializations
    swd = data;
    stream = swd->stream;
    text = swd->data;
    file = fdopen(stream, "w");

    if (file != NULL)
    {
      if (ferror(file) == 0 && feof(file) == 0)
      {
        fwrite(text, sizeof(char), strlen(text), file);
      }

      fclose(file);
    }

    gw_spellcheck_streamwithdata_free (swd);

    return NULL;
}


static gpointer 
gw_spellcheck_outfunc (gpointer data)
{
    //Declarations
    const int MAX = 500;
    GwSpellcheckStreamWithData *swd;
    FILE *file;
    char buffer[MAX];
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;

    //Initializations
    swd = data;
    spellcheck = swd->spellcheck;
    priv = spellcheck->priv;
    file = fdopen (swd->stream, "r");

    if (file != NULL)
    {
      gw_spellcheck_reset (spellcheck);

      //Add the new links
      while (file != NULL && ferror(file) == 0 && feof(file) == 0 && fgets(buffer, MAX, file) != NULL)
      {
        g_mutex_lock (priv->mutex);
        if (buffer[0] != '@' && buffer[0] != '*' && buffer[0] != '#' && strlen(buffer) > 1)
          priv->corrections = g_list_append (priv->corrections, g_strdup (buffer));
        g_mutex_unlock (priv->mutex);
      }

      //Cleanup
      fclose (file); file = NULL;
    }

    g_spawn_close_pid (swd->pid);

    gw_spellcheck_streamwithdata_free (swd);

    gw_spellcheck_set_status (spellcheck, GW_SPELLCHECKSTATUS_FINISHING);

    return NULL;
}

void
gw_spellcheck_start_check (GwSpellcheck *spellcheck)
{
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
    gboolean enchant_exists;
    GError *error;

    char *argv[] = { ENCHANT, "-a", "-d", "en", NULL};
    GPid pid;
    int stdin_stream;
    int stdout_stream;
    gboolean success;
    GwSpellcheckStreamWithData *indata;
    GwSpellcheckStreamWithData *outdata;
    
    //Initializations
    priv = spellcheck->priv;
    preferences = gw_application_get_preferences (priv->application);
    rk_conv_pref = lw_preferences_get_int_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_ROMAN_KANA);
    want_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !lw_util_is_japanese_locale()));
    query = gtk_entry_get_text (priv->entry);
    is_convertable_to_hiragana = (want_conv && lw_util_str_roma_to_hira (query, kana, MAX));
    spellcheck_pref = lw_preferences_get_boolean_by_schema (preferences, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK);
    enchant_exists = g_file_test (ENCHANT, G_FILE_TEST_IS_REGULAR);
    error = NULL;

    //Sanity checks
    if (
      enchant_exists == FALSE || 
      query == NULL           ||
      strlen(query) == 0      || 
      !spellcheck_pref        || 
      is_convertable_to_hiragana
    )
    {
      return;
    }

    gw_spellcheck_set_status (spellcheck, GW_SPELLCHECKSTATUS_CHECKING);
    gw_spellcheck_reset (spellcheck);

    //Open the pipes
    success = g_spawn_async_with_pipes (
      NULL, 
      argv,
      NULL,
      0,
      NULL,
      NULL,
      &pid,
      &stdin_stream,
      &stdout_stream,
      NULL,
      &error
    );

    //Stream the results
    if (success)
    {
      indata = gw_spellcheck_streamwithdata_new (spellcheck, stdin_stream, query, strlen(query) + 1, pid);
      outdata = gw_spellcheck_streamwithdata_new (spellcheck, stdout_stream, query, strlen(query) + 1, pid);

      if (indata != NULL && outdata != NULL)
      {
        gw_spellcheck_infunc ((gpointer) indata);
        priv->thread = g_thread_create (gw_spellcheck_outfunc, (gpointer) outdata, TRUE, &error);
      }

      if (priv->thread == NULL)
      {
        gw_spellcheck_set_status (spellcheck, GW_SPELLCHECKSTATUS_IDLE);
      }
    }
    
    gw_application_handle_error (priv->application, NULL, FALSE, &error);
  
    return;
}


