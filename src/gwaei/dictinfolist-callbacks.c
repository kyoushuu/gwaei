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
//! @file dictinfolist-callbacks.c
//!
//! @brief Unwritten
//!

#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


G_MODULE_EXPORT void gw_dictinfolist_list_store_row_changed_action_cb (GtkTreeModel *model,
                                                                            GtkTreePath *path,
                                                                            gpointer data)
{
    //Declarations
    int position;
    LwDictInfo *di;
    gpointer ptr;
    GtkTreeIter iter;

    //Initializations
    position = 0;

    g_signal_handler_block (model, app->dictinfolist->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED]);

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    {
      do {
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, GW_DICTINFOLIST_COLUMN_DICT_POINTER, &ptr, -1);
        if (ptr != NULL)
        {
          di = (LwDictInfo*) ptr;
          if (di != NULL)
          {
            di->load_position = position;
            position++;
          }
        }
      }
      while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter));
    }

    lw_dictinfolist_save_dictionary_order_pref (LW_DICTINFOLIST (app->dictinfolist), app->prefmanager);
    gw_dictinfolist_reload (app->dictinfolist, app->prefmanager);

    g_signal_handler_unblock (model, app->dictinfolist->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED]);
}



