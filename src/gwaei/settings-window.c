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
//! @file settings-window.c
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with GUI interface objects.
//! widgets.
//!


#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


void _settingswindow_initialize_dictionary_tree_view (GtkTreeView*);

//!
//! @brief Sets the initial status of the dictionaries in the settings dialog
//!
GwSettingsWindow* gw_settingswindow_new () 
{
    GwSettingsWindow *temp;
    GtkWidget *notebook;

    temp = (GwSettingsWindow*) malloc(sizeof(GwSettingsWindow));

    if (temp != NULL)
    {
      gw_app_block_searches (app);

      temp->builder = gtk_builder_new ();
      gw_window_load_ui_xml (GW_WINDOW (temp), "settings.ui");
      temp->toplevel = GTK_WINDOW (gtk_builder_get_object (temp->builder, "settings_window"));
      temp->type = GW_WINDOW_SETTINGS;

      GtkTreeView *view;
      view = GTK_TREE_VIEW (gtk_builder_get_object (temp->builder, "manage_dictionaries_treeview"));
      _settingswindow_initialize_dictionary_tree_view (view);

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_ROMAJI_KANA] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_BASE,
          LW_KEY_ROMAN_KANA,
          gw_settingswindow_sync_romaji_kana_conv_cb,
          temp
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_HIRA_KATA] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_BASE,
          LW_KEY_HIRA_KATA,
          gw_settingswindow_sync_hira_kata_conv_cb,
          temp
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_KATA_HIRA] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_BASE,
          LW_KEY_KATA_HIRA,
          gw_settingswindow_sync_kata_hira_conv_cb,
          temp
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_USE_GLOBAL_DOCUMENT_FONT] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_FONT,
          LW_KEY_FONT_USE_GLOBAL_FONT,
          gw_settingswindow_sync_use_global_document_font_cb,
          temp
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_GLOBAL_DOCUMENT_FONT] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_GNOME_INTERFACE,
          LW_KEY_DOCUMENT_FONT_NAME,
          gw_settingswindow_sync_global_document_font_cb,
          temp
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_CUSTOM_FONT] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_FONT,
          LW_KEY_FONT_CUSTOM_FONT,
          gw_settingswindow_sync_custom_font_cb,
          temp
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_SEARCH_AS_YOU_TYPE] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_BASE,
          LW_KEY_SEARCH_AS_YOU_TYPE,
          gw_settingswindow_sync_search_as_you_type_cb,
          temp
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_FG] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_HIGHLIGHT,
          LW_KEY_MATCH_FG,
          gw_settingswindow_sync_swatch_color_cb,
          gtk_builder_get_object (temp->builder, "match_foreground")
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_BG] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_HIGHLIGHT,
          LW_KEY_MATCH_BG,
          gw_settingswindow_sync_swatch_color_cb,
          gtk_builder_get_object (temp->builder, "match_background")
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_COMMENT_FG] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_HIGHLIGHT,
          LW_KEY_COMMENT_FG,
          gw_settingswindow_sync_swatch_color_cb,
          gtk_builder_get_object (temp->builder, "comment_foreground")
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_FG] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_HIGHLIGHT,
          LW_KEY_HEADER_FG,
          gw_settingswindow_sync_swatch_color_cb,
          gtk_builder_get_object (temp->builder, "header_foreground")
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_BG] = lw_prefmanager_add_change_listener_by_schema (
          app->prefmanager,
          LW_SCHEMA_HIGHLIGHT,
          LW_KEY_HEADER_BG,
          gw_settingswindow_sync_swatch_color_cb,
          gtk_builder_get_object (temp->builder, "header_background")
      );

      temp->signalids[GW_SETTINGSWINDOW_SIGNALID_DICTIONARIES_CHANGED] = g_signal_connect (
          G_OBJECT (app->dictinfolist->model),
          "changed",
          G_CALLBACK (gw_settingswindow_dictionaries_changed_cb),
          NULL
      );



      if (g_list_length (app->dictinfolist->list) == 0)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 1);
    }

    return temp;
}


//!
//! @brief Frees the memory used by the settings
//!
void gw_settingswindow_destroy (GwSettingsWindow *window)
{
    gtk_widget_destroy (GTK_WIDGET (window->toplevel));
    g_object_unref (window->builder);
    free(window);
    gw_app_block_searches (app);
}


//!
//! @brief Sets the text in the source gtkentry for the appropriate dictionary
//!
//! @param widget Pointer to a GtkEntry to set the text of
//! @param value The constant string to use as the source for the text
//!
void gw_settings_set_dictionary_source (GtkWidget *widget, const char* value)
{
    if (widget != NULL && value != NULL)
    {
      gtk_entry_set_text (GTK_ENTRY (widget), value);
    }
}


void _settingswindow_initialize_dictionary_tree_view (GtkTreeView *view)
{
      //Declarations
      GtkCellRenderer *renderer;
      GtkTreeViewColumn *column;

      gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL (app->dictinfolist->model));

      //Create the columns and renderer for each column
      renderer = gtk_cell_renderer_pixbuf_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new ();
      gtk_tree_view_column_set_title (column, " ");
      gtk_tree_view_column_pack_start (column, renderer, TRUE);
      gtk_tree_view_column_set_attributes (column, renderer, "icon-name", GW_DICTINFOLIST_COLUMN_IMAGE, NULL);
      gtk_tree_view_append_column (view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes ("#", renderer, "text", GW_DICTINFOLIST_COLUMN_POSITION, NULL);
      gtk_tree_view_append_column (view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes (gettext("Name"), renderer, "text", GW_DICTINFOLIST_COLUMN_LONG_NAME, NULL);
      gtk_tree_view_column_set_min_width (column, 100);
      gtk_tree_view_append_column (view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes (gettext("Engine"), renderer, "text", GW_DICTINFOLIST_COLUMN_ENGINE, NULL);
      gtk_tree_view_append_column (view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes (gettext("Shortcut"), renderer, "text", GW_DICTINFOLIST_COLUMN_SHORTCUT, NULL);
      gtk_tree_view_append_column (view, column);
}

