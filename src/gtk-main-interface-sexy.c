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
//! @file src/gtk-main-interface-sexy.c
//!
//! @brief Replaces the search GtkEntry with a SexySpellEntry.
//!
//! File used for implimenting a libsexy entry as the search query.  Libsexy
//! is nice because it allows spell checking compatibility.  Because some day
//! I may want to remove the dependeny, this code mingles with the other files
//! minimally.  One the initialize_sexy() command should be used if you want
//! to use the libsexy search entry.
//!


#include <stdlib.h>
#include <regex.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>
#include <libsexy/sexy.h>

#include <gwaei/definitions.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/utilities.h>
#include <gwaei/search-objects.h>
#include <gwaei/preferences.h>

#include <gwaei/gtk.h>
#include <gwaei/gtk-main-callbacks.h>
#include <gwaei/gtk-settings-callbacks.h>


//!
//! @brief Turns on or aff spellcheck given specific conditions
//!
//! The function has to make sure that the user wants spellcheck, and if 
//! the query is convertable to hiragana if the conversion pref is turned on.
//!
//! @param widget Unused GtkWidget
//! @param data Unused gpointer
//!
void do_conditionally_enable_spellcheck (GtkWidget *widget, gpointer data)
{
     char id[50];

     GtkWidget *entry;
     entry = search_entry;

     gboolean spellcheck_pref;
     spellcheck_pref = gw_pref_get_boolean (GCKEY_GW_SPELLCHECK, TRUE);
     int rk_conv_pref;
     rk_conv_pref = gw_pref_get_int (GCKEY_GW_ROMAN_KANA, 0);
     gboolean want_conv;
     want_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !gw_util_is_japanese_locale()));

     char *text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
     if (text == NULL) return;

     char kana[MAX_QUERY];
     gboolean is_convertable_to_hiragana;
     is_convertable_to_hiragana = (want_conv && gw_util_str_roma_to_hira (text, kana, MAX_QUERY));

     if (gw_util_all_chars_are_in_range (text, L' ', L'|') == TRUE &&
         is_convertable_to_hiragana == FALSE                     &&
         spellcheck_pref == TRUE                                   )
     {
       GtkWidget *combobox ;
       strcpy (id, "dictionary_combobox");
       combobox = GTK_WIDGET (gtk_builder_get_object (builder, id));

       //Make sure the combobox is sane
       if (!GTK_WIDGET_IS_SENSITIVE (combobox))
       {
         return;
       }

       //Default to the english dictionary when using the english-japanese dict
       char *active;
       active = gtk_combo_box_get_active_text (GTK_COMBO_BOX (combobox));
       GError *err;
       err = NULL;
       GwDictInfo *di = gw_dictlist_get_selected()->data;


       if ((strcmp ("Radicals", di->name) == 0) ||
           (strcmp ("Names",    di->name) == 0)   )
       {
         //No spell checking needed
         sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), FALSE);
       }
       else if ((strcmp ("English",  di->name) == 0) ||
                (strcmp ("Kanji",    di->name) == 0) ||
                (strcmp ("Mix",      di->name) == 0) ||
                (strcmp ("Examples", di->name) == 0) ||
                (strcmp ("Places",   di->name) == 0)   )
       {
         //Set the spellchecked language to english
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "en") == FALSE)
           sexy_spell_entry_activate_language (SEXY_SPELL_ENTRY (entry), "en", &err);
         if (err != NULL)
         {
           g_error_free (err);
           err = NULL;
         }
         sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), TRUE);
	 
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "fr") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "fr");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "de") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "de");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "es") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "es");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "it") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "it");
       }
       else if (strcmp ("French", di->name) == 0)
       {
         //Set the spellchecked language to french
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "fr") == FALSE)
           sexy_spell_entry_activate_language (SEXY_SPELL_ENTRY (entry), "fr", &err);

         if (err != NULL)
         {
           g_error_free (err);
           err = NULL;
         }
         sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), TRUE);
	 
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "en") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "en");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "de") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "de");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "es") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "es");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "it") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "it");
       }
       else if (strcmp ("German", di->name) == 0)
       {
         //Set the spellchecked language to german
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "de") == TRUE)
           sexy_spell_entry_activate_language (SEXY_SPELL_ENTRY (entry), "de", &err);

         if (err != NULL)
         {
           g_error_free (err);
           err = NULL;
         }
         sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), TRUE);
	 
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "fr") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "fr");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "en") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "en");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "es") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "es");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "it") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "it");
       }
       else if (strcmp ("Spanish", di->name) == 0)
       {
         //Set the spellchecked language to spanish
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "es") == TRUE)
           sexy_spell_entry_activate_language (SEXY_SPELL_ENTRY (entry), "es", &err);

         if (err != NULL)
         {
           g_error_free (err);
           err = NULL;
         }
         sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), TRUE);
	 
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "fr") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "fr");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "de") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "de");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "en") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "en");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "it") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "it");
       }
       else if (strcmp ("Italian", di->name) == 0)
       {
         //Set the spellchecked language to italian
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "it") == TRUE)
           sexy_spell_entry_activate_language (SEXY_SPELL_ENTRY (entry), "it", &err);

         if (err != NULL)
         {
           g_error_free (err);
           err = NULL;
         }
         sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), TRUE);
	 
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "fr") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "fr");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "de") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "de");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "es") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "es");
         if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (entry), "en") == TRUE)
           sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (entry), "en");
       }
       //Default to the system default otherwise
       else
       {
         //Set the system default language
         sexy_spell_entry_activate_default_languages (SEXY_SPELL_ENTRY (entry));
         sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), TRUE);
       }
     }
     else
     {
       sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), FALSE);
     }
     g_free (text);
}


//!
//! @brief Conitionally enables search when focus enters the searh entry
//!
//! @param widget Unused GtkWidget
//! @param data Unused gpointer
//! @see do_enable_spellcheck_when_focus_out ()
//! @return always returns false
//!
gboolean do_enable_spellcheck_when_focus_in (GtkWidget *widget, gpointer data)
{
      do_conditionally_enable_spellcheck (NULL, NULL);
      return FALSE;
}


//!
//! @brief Disables spell check wehn the focus isn't in the search entry
//!
//! The though behind is is the red underline is distracting when the results
//! are being searched through by the user.
//!
//! @param widget Unused GtkWidget
//! @param data Unused gpointer
//! @see do_enable_spellcheck_when_focus_in ()
//! @return always returns false
//!
gboolean do_disable_spellcheck_when_focus_out (GtkWidget *widget, gpointer data)
{
    sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (search_entry), FALSE);
    return FALSE;
}


//!
//! @brief Sets up the sexy text entry
//!
//! The function takes the time to get rid of the gtkentry and replace it with
//! a SexySpellEntry.  In the process, it makes sure all of the signals are
//! connected to the new widget. It also make hidden spellcheck toolbutton
//! and checkbox visible.
//!
void gw_sexy_initialize_libsexy ()
{
    char id[50];


    //Make the hidden spellcheck toolbutton appear
    GtkWidget *spellcheck_toolbutton;
    strcpy (id, "spellcheck_toolbutton");
    spellcheck_toolbutton = GTK_WIDGET (gtk_builder_get_object (builder, id));
    gtk_widget_show (spellcheck_toolbutton);


    //Swap the original entry for the libsexy one
    GtkWidget *entry;
    strcpy (id, "search_entry");
    entry = GTK_WIDGET (gtk_builder_get_object (builder, id));

    gchar *tooltip_text = gtk_widget_get_tooltip_text (entry);
    gtk_widget_destroy (entry);

    entry = GTK_WIDGET (sexy_spell_entry_new ());
    search_entry = entry;

    GtkWidget *entry_container;
    strcpy(id, "search_entry_container");
    entry_container = GTK_WIDGET (gtk_builder_get_object (builder, id));

    gtk_container_add (GTK_CONTAINER (entry_container), entry);
    gtk_widget_set_name (entry, "search_entry");
    if (tooltip_text != NULL)
    {
      gtk_widget_set_tooltip_text (entry, tooltip_text);
      g_free (tooltip_text);
    }
    gtk_entry_set_max_length (GTK_ENTRY (entry), 50);
    gtk_widget_show (entry);

    GtkWidget* results_tv = get_widget_from_target(GW_TARGET_RESULTS);

    //Mimic original callbacks from the original search entry
    g_signal_connect( G_OBJECT (entry),       "activate",
                      G_CALLBACK (do_search), NULL);
    g_signal_connect( G_OBJECT (entry),       "focus-in-event",
                      G_CALLBACK (do_update_clipboard_on_focus_change), entry);
    g_signal_connect( G_OBJECT (entry),       "key-press-event",
                      G_CALLBACK (do_focus_change_on_key_press), NULL);
    g_signal_connect( G_OBJECT (entry),       "changed",
                      G_CALLBACK (do_update_button_states_based_on_entry_text), NULL);
    g_signal_connect( G_OBJECT (entry),       "icon-release",
                      G_CALLBACK (do_clear_search), NULL);

    //New callbacks specifically for libsexy
    g_signal_connect( G_OBJECT (entry), "changed",
                      G_CALLBACK (do_conditionally_enable_spellcheck), NULL);
    g_signal_connect( G_OBJECT (entry), "focus-in-event",
                      G_CALLBACK (do_enable_spellcheck_when_focus_in), entry);
    g_signal_connect( G_OBJECT (results_tv), "focus-in-event",
                      G_CALLBACK (do_disable_spellcheck_when_focus_out), entry);
    g_signal_connect( G_OBJECT (kanji_tv), "focus-in-event",
                      G_CALLBACK (do_disable_spellcheck_when_focus_out), entry);

    //Show the spellcheck checkbox in the preferences
    GtkWidget *spellcheck_checkbox;
    strcpy (id, "query_spellcheck");
    spellcheck_checkbox = GTK_WIDGET (gtk_builder_get_object (builder, id));
    gtk_widget_show (spellcheck_checkbox);

}


//!
//! @brief Sets the gui widgets consistently to the requested state
//!
//! The function makes sure that both of the widgets in the gui are the same
//! when the user clicks a one of them to change the settings.
//!
//! @param request the requested state for spellchecking widgets
//!
void gw_sexy_ui_set_spellcheck(gboolean request)
{
    char id[50];

    GtkWidget *pref_checkbox, *toolbar_button;
    strcpy(id, "query_spellcheck");
    pref_checkbox = GTK_WIDGET (gtk_builder_get_object(builder, id));
    strcpy(id, "spellcheck_toolbutton");
    toolbar_button = GTK_WIDGET (gtk_builder_get_object(builder, id));

    g_signal_handlers_block_by_func(pref_checkbox, do_spellcheck_toggle, NULL); 
    g_signal_handlers_block_by_func(toolbar_button, do_spellcheck_toggle, NULL); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (pref_checkbox), request);
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON (toolbar_button), request);
    g_signal_handlers_unblock_by_func(pref_checkbox, do_spellcheck_toggle, NULL); 
    g_signal_handlers_unblock_by_func(toolbar_button, do_spellcheck_toggle, NULL); 
    do_conditionally_enable_spellcheck (NULL, NULL);
}