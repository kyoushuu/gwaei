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
//! @file spellcheck-callbacks.c
//!
//! @brief To be written 
//!

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/spellcheck-private.h>

void 
gw_spellcheck_menuitem_activated_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    _SpellingReplacementData *srd;
    char *text;
    char *buffer;
    char *replacement;
    int start_offset;
    int end_offset;
    int index;

    //Initializations
    srd = data;
    replacement = srd->replacement_text;
    start_offset = srd->start_offset;
    end_offset = srd->end_offset;
    text = g_strdup (gtk_entry_get_text (GTK_ENTRY (srd->entry)));
    buffer = (char*) malloc (sizeof(char) * (strlen(replacement) + strlen(text)));

    strcpy(buffer, text);
    strcpy (buffer + start_offset, replacement);
    strcat (buffer, text + end_offset);

    index = gtk_editable_get_position (GTK_EDITABLE (srd->entry));
    if (index > end_offset || index > start_offset + strlen(replacement))
      index = index - (end_offset - start_offset) + strlen(replacement);
    gtk_entry_set_text (GTK_ENTRY (srd->entry), buffer);
    gtk_editable_set_position (GTK_EDITABLE (srd->entry), index);

    //Cleanup
    free (buffer);
    g_free (text);
}


gboolean 
_get_line_coordinates (GwSpellcheck *spellcheck, int startindex, int endindex, int *x, int *y, int *x2, int *y2)
{
    //Declarations
    GwSpellcheckPrivate *priv;
    int index;
    PangoLayout *layout;
    PangoRectangle rect;
    PangoLayoutIter *iter;
    int xoffset, yoffset;

    //Initializations
    priv = spellcheck->priv;
    layout = gtk_entry_get_layout (priv->entry);
    iter = pango_layout_get_iter (layout);
    xoffset = gw_spellcheck_get_x_offset (spellcheck);
    yoffset = gw_spellcheck_get_y_offset (spellcheck);
    *x = *y = *x2 = *y2 = 0;

    do {
      index = pango_layout_iter_get_index (iter);
      pango_layout_iter_get_char_extents  (iter, &rect);
      if (index == startindex)
      {
        *x = PANGO_PIXELS (rect.x) + xoffset;
        *y = PANGO_PIXELS (rect.y + rect.height) + yoffset;
      }
      if (index == endindex - 1)
      {
        *x2 = PANGO_PIXELS (rect.width + rect.x) + xoffset;
        *y2 = *y;
      }
    } while (pango_layout_iter_next_char (iter));

    //Cleanup
    pango_layout_iter_free (iter);

    return (*x > 0 && *y > 0 && *x2 > 0 && *y2 > 0);
}

void 
_draw_line (cairo_t *cr, int x, int y, int x2, int y2)
{
    //Declarations
    int ydelta;
    int xdelta;
    int i;
    gboolean up;

    //Initializations
    xdelta = 2;
    ydelta = 2;
    up = FALSE;
    y += ydelta;
    x++;

    cairo_set_line_width (cr, 0.8);
    cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 0.8);

    cairo_move_to (cr, x, y2);
    for (i = x + xdelta; i < x2; i += xdelta)
    {
      if (up)
        y2 -= ydelta;
      if (!up)
        y2 += ydelta;
      up = !up;

      cairo_line_to (cr, i, y2);
    }
    cairo_stroke (cr);
}


G_MODULE_EXPORT gboolean 
gw_spellcheck_button_press_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GwSpellcheck *spellcheck;

    spellcheck = GW_SPELLCHECK (data);

    gw_spellcheck_update_cordinates (spellcheck, event);

    return FALSE;
}


G_MODULE_EXPORT void 
gw_spellcheck_populate_popup_cb (GtkEntry *entry, GtkMenu *menu, gpointer data)
{
    GwSpellcheck *spellcheck;

    spellcheck = GW_SPELLCHECK (data);

    gw_spellcheck_populate_popup (spellcheck, menu);
}


G_MODULE_EXPORT gboolean 
gw_spellcheck_draw_underline_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;
    gint x, y, x2, y2;
    GList *iter;
    char **info;
    char **atoms;
    int start_offset, end_offset;

    //Initializations
    spellcheck = GW_SPELLCHECK (data);
    priv = spellcheck->priv;

    g_mutex_lock (&priv->mutex);
    for (iter = priv->corrections; iter != NULL; iter = iter->next)
    {
      if (iter->data == NULL) continue;

      info = g_strsplit (iter->data, ":", -1);
      atoms = g_strsplit (info[0], " ", -1);

      start_offset = (int) g_ascii_strtoull (atoms[3], NULL, 10);
      end_offset = strlen(atoms[1]) + start_offset;
      start_offset = gtk_entry_text_index_to_layout_index (GTK_ENTRY (widget), start_offset);
      end_offset = gtk_entry_text_index_to_layout_index (GTK_ENTRY (widget), end_offset);

      //Calculate the line
      if (_get_line_coordinates (spellcheck, start_offset, end_offset, &x, &y, &x2, &y2))
      {
        _draw_line (cr, x, y, x2, y2);
      }

      g_strfreev (info);
      g_strfreev (atoms);
    }
    g_mutex_unlock (&priv->mutex);

    return FALSE;
}


G_MODULE_EXPORT void 
gw_spellcheck_queue_cb (GtkEditable *editable, gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;

    //Initializations
    spellcheck = GW_SPELLCHECK (data);
    priv = spellcheck->priv;

    if (priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] != 0)
    {
      priv->timeout = 0;
    }

    else
    {
      priv->timeout = 0;
      priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] = g_timeout_add_full (
          G_PRIORITY_LOW, 
          100, (GSourceFunc) 
          gw_spellcheck_update_timeout, 
          spellcheck, 
          NULL
      );
      gw_spellcheck_reset (spellcheck);
      gtk_widget_queue_draw (GTK_WIDGET (editable));
    }
}


GwSpellcheckStreamWithData* gw_spellcheck_streamwithdata_new (GwSpellcheck *spellcheck, int stream, const char* data, int length, GPid pid)
{
    GwSpellcheckStreamWithData *temp;

    if ((temp = malloc(sizeof(GwSpellcheckStreamWithData))) != NULL)
    {
      temp->spellcheck = spellcheck;
      temp->stream = stream;
      temp->data = malloc(length);
      if (temp->data != NULL) strncpy(temp->data, data, length);
      temp->length = length;
      temp->pid = pid;
    }

    return temp;
}


void 
gw_spellcheck_streamwithdata_free (GwSpellcheckStreamWithData *swd)
{
    free(swd->data);
    free(swd);
}


gboolean 
gw_spellcheck_update_timeout (gpointer data)
{
    //Declarations
    GwSpellcheck *spellcheck;
    GwSpellcheckPrivate *priv;
    GwSpellcheckStatus status;
    gboolean return_value;

    //Initializaitons
    spellcheck = GW_SPELLCHECK (data);
    priv = spellcheck->priv;
    status = gw_spellcheck_get_status (spellcheck);
    return_value = FALSE;

    //Sanity check
    if (priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] == 0) return FALSE;

    switch (status)
    {
      case GW_SPELLCHECKSTATUS_IDLE:
        if (priv->timeout < priv->threshold) //Wait for the user to stop typing
          priv->timeout++;
        else //Start the check
          gw_spellcheck_start_check (spellcheck);
      case GW_SPELLCHECKSTATUS_CHECKING: //Do nothing but wait while a spellcheck is running
        return_value = TRUE;
        break;
      case GW_SPELLCHECKSTATUS_FINISHING: //Cleanup from the spellcheck
        if (priv->thread != NULL) g_thread_join (priv->thread); priv->thread = NULL;
        gw_spellcheck_set_status (spellcheck, GW_SPELLCHECKSTATUS_IDLE);
        priv->timeoutid[GW_SPELLCHECK_TIMEOUTID_UPDATE] = 0;
        if (priv->timeout < priv->threshold) //Clear out the results if another check was scheduled and start over
        {
          gw_spellcheck_reset (spellcheck);
          return_value = TRUE;
        }
        else //There wasn't one scheduled so don't clear the results
        {
          return_value = FALSE;
        }
        gtk_widget_queue_draw (GTK_WIDGET (priv->entry));
        break;
      default:
        g_assert_not_reached ();
        return_value = FALSE;
        break;
    }

    return return_value;
}


void
gw_spellcheck_free_menuitem_data_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    _SpellingReplacementData *srd;

    //Initializations
    srd = data;

    //Cleanup
    g_free (srd->replacement_text);
    free (srd);
}



