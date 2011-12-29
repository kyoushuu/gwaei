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
//! @file dictionarystore-callbacks.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdlib.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/dictionarystore-private.h>


G_MODULE_EXPORT void gw_dictionarystore_row_changed_cb (GtkTreeModel *model,
                                                        GtkTreePath  *path,
                                                        gpointer      data)
{
    //Declarations
    GwDictionaryStore *store;
    GwDictionaryStorePrivate *priv;
    gint position;
    gpointer ptr;
    GtkTreeIter iter;
    LwDictInfo *di;
    LwDictInfoList *dictinfolist;
    gboolean valid;

    //Initializations
    store = GW_DICTIONARYSTORE (model);
    dictinfolist = gw_dictionarystore_get_dictinfolist (store);
    priv = store->priv;
    position = 0;

    g_signal_handler_block (model, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);

    valid = gtk_tree_model_get_iter_first (model, &iter);
    while (valid)
    {
      gtk_tree_model_get (model, &iter, GW_DICTIONARYSTORE_COLUMN_DICT_POINTER, &ptr, -1);
      if (ptr != NULL)
      {
        di = LW_DICTINFO (ptr);
        di->load_position = position;
        position++;
      }
      valid = gtk_tree_model_iter_next (model, &iter);
    }

    g_signal_handler_unblock (model, priv->signalids[GW_DICTIONARYSTORE_SIGNALID_ROW_CHANGED]);

    lw_dictinfolist_sort_and_normalize_order (dictinfolist);
    gw_dictionarystore_update (store);
}




