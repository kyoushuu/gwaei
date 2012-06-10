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
//! @file dictionarylist.c
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <libwaei/gettext.h>
#include <libwaei/libwaei.h>


//Private methods
static void lw_dictionarylist_init (LwDictionaryList*);
static void lw_dictionarylist_deinit (LwDictionaryList*);
static gint lw_dictionarylist_sort_compare_function (gconstpointer, gconstpointer, gpointer);


//!
//! @brief Constructor for a dictionary list object.
//! @return An allocated LwDictionaryList that will be needed to be freed by lw_dictionarylist_free ()
//!
LwDictionaryList* 
lw_dictionarylist_new ()
{
    LwDictionaryList *temp;
    temp = (LwDictionaryList*) malloc(sizeof(LwDictionaryList));

    if (temp != NULL)
    {
      lw_dictionarylist_init (temp);
    }

    return temp;
}


//!
//! @brief Frees up the LwDictionaryList dictionary list
//! The work of freeing each individual dictionary is automatically handled,
//! removing the chance for mistakes.
//!
void 
lw_dictionarylist_free (LwDictionaryList *dictionarylist)
{
    lw_dictionarylist_deinit (dictionarylist);
    free(dictionarylist);
}



static void 
lw_dictionarylist_init (LwDictionaryList *dictionarylist)
{
    memset(dictionarylist, 0, sizeof(LwDictionaryList));
    g_mutex_init (&dictionarylist->mutex);
}


static void 
lw_dictionarylist_deinit (LwDictionaryList *dictionarylist)
{
    lw_dictionarylist_clear (dictionarylist);
    g_mutex_clear (&dictionarylist->mutex);
}


void 
lw_dictionarylist_clear (LwDictionaryList *dictionarylist)
{
    //Sanity checks
    g_return_if_fail (dictionarylist != NULL);

    //Declarations
    LwDictionary *dictionary;
    GList *link;

    if (dictionarylist->list != NULL)
    {
      for (link = dictionarylist->list; link != NULL; link = link->next)
      {
        dictionary = LW_DICTIONARY (link->data);
        if (dictionary != NULL)
        {
          g_object_unref (dictionary); 
        }
      }
      g_list_free (dictionarylist->list);
    }
}


void 
lw_dictionarylist_load_installed (LwDictionaryList *dictionarylist)
{
    //Sanity checks
    g_return_if_fail (dictionarylist != NULL);

    //Declarations
    gchar** idlist;
    gchar **iditer;
    gchar** pair;
    gchar *typename;
    GType type;
    LwDictionary *dictionary;
    const gchar *FILENAME;

    lw_dictionarylist_clear (dictionarylist);

    idlist = lw_dictionary_get_installed_idlist (G_TYPE_NONE);
    if (idlist != NULL)
    {
      for (iditer = idlist; *iditer != NULL; iditer++)
      {
        pair = g_strsplit_set (*iditer, "/", 2);
        if (pair != NULL && pair[0] != NULL && pair[1] != NULL) 
        {
          typename = lw_dictionary_directoryname_to_typename (pair[0]);
          type = g_type_from_name (typename);
          FILENAME = pair[1];
          dictionary = LW_DICTIONARY (g_object_new (type, "filename", FILENAME, NULL));
          if (dictionary != NULL && LW_IS_DICTIONARY (dictionary))
            lw_dictionarylist_add_dictionary (dictionarylist, dictionary);
        }
        g_strfreev (pair); pair = NULL;
      }
      g_strfreev (idlist); idlist = NULL;
    }
}



//!
//! @brief Gets the dictionary by load position in the GUI
//! @param request The GUI load position of the desired dictionary
//! @return The position in the LwDictionaryList of the LwDictionary
//!
LwDictionary* 
lw_dictionarylist_get_dictionary_by_position (LwDictionaryList* dictionarylist, gint position)
{
    //Declarations
    LwDictionary *dictionary;

    //Initializiations
    dictionary = g_list_nth_data (dictionarylist->list, position);

    return dictionary;
}


gint
lw_dictionarylist_get_position (LwDictionaryList *dictionarylist, LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionarylist != NULL, -1);
    g_return_val_if_fail (dictionary != NULL, -1);

    return g_list_index (dictionarylist->list, dictionary);
}


//!
//! @brief Adds a dictionary to the LwDictionaryList with sanity checks
//! @param TYPE Engine of the dictionary to add
//! @param FILENAME Name of the dictionary to add
//!
void 
lw_dictionarylist_add_dictionary (LwDictionaryList *dictionarylist, LwDictionary *dictionary)
{
    //Sanity checks
    g_return_if_fail (dictionarylist != NULL);
    g_return_if_fail (dictionary != NULL);

    if (lw_dictionarylist_dictionary_exists (dictionarylist, dictionary)) return;

    //Append to the dictionary list if was loadable
    if (dictionary != NULL) dictionarylist->list = g_list_append (dictionarylist->list, dictionary);
}


//!
//! @brief Searchs for a specific dictionary by name
//! The function will go through each dictionary until it matches the requested
//! name.
//!
//! @param DICTTYPE The parsing engine of the dictionary wanted.  There can be
//!               dictionaries with the same name, but different engines.
//! @param NAME A constant string to search for in the dictionary names.
//! @returns The requested LwDictionary object if found or null.
//!
LwDictionary* 
lw_dictionarylist_get_dictionary (LwDictionaryList *dictionarylist, const GType TYPE, const gchar* FILENAME)
{
    //Sanity checks
    g_return_val_if_fail (dictionarylist != NULL && FILENAME != NULL, NULL);

    //Declarations
    GList *iter;
    LwDictionary *dictionary;
    GType type;
    const gchar *FILENAME2;

    //Initializations
    dictionary = NULL;

    for (iter = dictionarylist->list; iter != NULL; iter = iter->next)
    {
      dictionary = LW_DICTIONARY (iter->data);
      type = G_OBJECT_TYPE (dictionary);
      FILENAME2 = lw_dictionary_get_filename (dictionary);
      if (dictionary != NULL && g_type_is_a (type, TYPE) && strcmp (FILENAME2, FILENAME) == 0)
      {
        break;
      }
      dictionary = NULL;
    }

    return dictionary;
}

//!
//!  @brief  Gets a LwDictionary object by a fuzzy string description.
//!          It can be either of the form "parser/dictionary" or 
//!          just be the dictionary name.  Case is ignored.
//!  @param FUZZY_DESCRIPTION A fuzzy description of the wanted dictionary.
//!  @returns A matching LwDictionary object or NULL
//!
LwDictionary* 
lw_dictionarylist_get_dictionary_fuzzy (LwDictionaryList *dictionarylist, const char* FUZZY_DESCRIPTION)
{
    //Declarations
    LwDictionary *dictionary;

    //Initializations
    dictionary = NULL;

    //Try getting the first dictionary if none is specified
    if (FUZZY_DESCRIPTION == NULL)
    {
      if (dictionarylist->list != NULL)
        dictionary = LW_DICTIONARY (dictionarylist->list->data);
      else
        dictionary = NULL;
    }

    //Otherwise try getting a dictionary using a few different string parsers
    else
    {
      if (dictionary == NULL)
        dictionary = lw_dictionarylist_get_dictionary_by_id (dictionarylist, FUZZY_DESCRIPTION);
      if (dictionary == NULL)
        dictionary = lw_dictionarylist_get_dictionary_by_filename (dictionarylist, FUZZY_DESCRIPTION);
    }

    return dictionary;
}

//!
//! @brief Grabs the first dictionary with a matching dictionary 
//!        filename.  If you have dictionaries with different 
//!        parsers but the same name, the others will not 
//!        be accessible with this function.
//! @param NAME A constant string to search for in the dictionary names.  
//!             This is a fuzzy search, ignoring DICTTYPE and case
//! @returns The requested LwDictionary object if found or null.
//!
LwDictionary* 
lw_dictionarylist_get_dictionary_by_filename (LwDictionaryList *dictionarylist, const gchar* FILENAME)
{
    //Sanity checks
    g_assert (FILENAME != NULL);

    //Declarations
    GList *iter;
    LwDictionary *dictionary;
    const gchar *FILENAME2;

    //Initializations
    dictionary = NULL;

    for (iter = dictionarylist->list; iter != NULL; iter = iter->next)
    {
      dictionary = LW_DICTIONARY (iter->data);
      FILENAME2 = lw_dictionary_get_filename (dictionary);
      if (FILENAME2 != NULL && g_ascii_strcasecmp (FILENAME2, FILENAME) == 0)
        break;
      dictionary = NULL;
    }

    return dictionary;
}


//!
//! @brief Finds a dictionary by using an id string of the form of 
//!        "engine/dictionary". Case is ignored.
//! @param ENGINE_AND_FILENAME A string in the form "engine/dictionary"
//!                            used to search for a dictionary
//! @returns The requested LwDictionary object if found or NULL.
//!
LwDictionary* 
lw_dictionarylist_get_dictionary_by_id (LwDictionaryList *dictionarylist, const gchar* ENGINE_AND_FILENAME)
{
    //Sanity checks
    g_return_val_if_fail (dictionarylist != NULL && ENGINE_AND_FILENAME != NULL, NULL);

    //Declarations
    GList *link;
    LwDictionary *dictionary;
    gchar **pair;
    const gchar *FILENAME1;
    const gchar *FILENAME2;
    GType type1, type2;

    //Initializations
    link = NULL;
    dictionary = NULL;
    pair = g_strsplit (ENGINE_AND_FILENAME, "/", 2);

    if (pair != NULL)
    {
      if (g_strv_length (pair) == 2)
      {
        type1 = g_type_from_name (pair[0]);
        FILENAME1 = pair[1];

        for (link = dictionarylist->list; link != NULL; link = link->next)
        {
          dictionary = LW_DICTIONARY (link->data);
          FILENAME2 = lw_dictionary_get_filename (dictionary);
          type2 = G_OBJECT_TYPE (dictionary);
          if (g_type_is_a (type1, type2) && g_ascii_strcasecmp (FILENAME1, FILENAME2) == 0)
            break;
          dictionary = NULL;
        }
      }

      g_strfreev (pair); pair = NULL;
    }

    return dictionary;
}



//!
//! @brief Searchs for a specific dictionary by name to see if it is installed
//! @param NAME request a const string to search for in the dictionary names
//! @return returns true if the dictionary is installed
//!
gboolean 
lw_dictionarylist_dictionary_exists (LwDictionaryList *dictionarylist, LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionarylist != NULL, FALSE);
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
    gboolean exists;
    GList *link;

    //Initializations
    exists = FALSE;
    link = dictionarylist->list;

    //Return true if the dictionary exists
    while (link != NULL && exists == FALSE)
    {
      exists = lw_dictionary_equals (dictionary, LW_DICTIONARY (link->data));
      link = link->next;
    }

    return exists;
}


//!
//! @brief Returns the number of dictionaries in the dictionary list
//!
//! This is not the number of dictionaries that are active.  It shows
//! how many dictionary names are recorded in the dictionary list.
//! By default, the default dictionaries appended to the list with
//! an UNLW_DICT_STATUS_INSTALLED status if they are unavailable. If the LW_DICT_MIX dictionary
//! is installed, Kanji and Radicals disappear from the GUI, but are still
//! in this list.
//!
//! @return Integer representing the number of installed dictionaries
//!
gint 
lw_dictionarylist_get_total (LwDictionaryList *dictionarylist)
{
    //Sanity check
    g_return_val_if_fail (dictionarylist != NULL, 0);

    return g_list_length (dictionarylist->list);
}


//
//! @brief Saves the current load order to the preferences
//
void 
lw_dictionarylist_save_order (LwDictionaryList *dictionarylist, LwPreferences *preferences)
{
    //Declarations
    LwDictionary *dictionary;
    gchar *order;
    GList *link;
    gchar **atoms;
    gchar **ptr;

    //Initializations;
    atoms = g_new (gchar*, lw_dictionarylist_get_total (dictionarylist) + 1);

    //Construct the preference string
    if (atoms != NULL)
    {
      ptr = atoms;
      for (link = dictionarylist->list; link != NULL; link = link->next)
      {
        dictionary = LW_DICTIONARY (link->data);
        *ptr = lw_dictionary_build_id (dictionary);
        if (*ptr == NULL) break;
        ptr++;
      }
      *ptr = NULL;

      //Save the string
      order = g_strjoinv (";", atoms);
      if (order != NULL)
      {
        lw_preferences_set_string_by_schema (preferences, LW_SCHEMA_DICTIONARY, LW_KEY_LOAD_ORDER, order);
        g_free (order); order = NULL;
      }

      g_strfreev (atoms); atoms = NULL;
      ptr = NULL;
    }
}


//
//! @brief Loads the load order from the preferences
//
void 
lw_dictionarylist_load_order (LwDictionaryList *dictionarylist, LwPreferences *preferences)
{
    //Sanity checks
    g_return_if_fail (dictionarylist != NULL && preferences != NULL);

    const int MAX = 512;
    gchar order[MAX];
    gchar **atoms;
    gchar **ptr;
    GHashTable *hashtable;
    gint i;

    lw_preferences_get_string_by_schema (preferences, order, LW_SCHEMA_DICTIONARY, LW_KEY_LOAD_ORDER, MAX);
    atoms = g_strsplit_set (order, ";", -1);
    if (atoms != NULL)
    {
      i = 0;
      hashtable = g_hash_table_new (g_str_hash, g_str_equal);
      if (hashtable != NULL)
      {
        for (ptr = atoms; *ptr != NULL; ptr++)
        {
          g_hash_table_insert (hashtable, *ptr, GINT_TO_POINTER (i));
          i++;
        }

        dictionarylist->list = g_list_sort_with_data (dictionarylist->list, lw_dictionarylist_sort_compare_function, hashtable);
        
        g_hash_table_destroy (hashtable); hashtable = NULL;
      }
      g_strfreev (atoms); atoms = NULL;
    }
}


//!
//! @brief All dictionaries with a specific status get switched to the requested one
//! @param a Pointer to LwDictionary object a
//! @param b Pointer to LwDictionary object b
//! @returns Whether the position of a is less than (-1), equal (0) or greater than b (1)
//!
static gint 
lw_dictionarylist_sort_compare_function (gconstpointer a, gconstpointer b, gpointer data)
{
    //Sanity checks
    g_return_val_if_fail (data != NULL, 0);
  
    //Declarations
    GHashTable *hashtable;
    gchar *description_a, *description_b;
    LwDictionary *dictionary_a, *dictionary_b;
    gpointer position_a_ptr, position_b_ptr;
    gint position_a, position_b;
    gboolean found_a, found_b;
    gint order;

    //Initializations
    hashtable = (GHashTable*) data;
    found_a = found_b = FALSE;

    dictionary_a = LW_DICTIONARY (a);
    description_a = lw_dictionary_build_id (dictionary_a);
    if (description_a != NULL)
    {
      found_a = g_hash_table_lookup_extended (hashtable, description_a, NULL, &position_a_ptr);
      position_a = GPOINTER_TO_INT (position_a_ptr);
      g_free (description_a); description_a = NULL;
    }
    
    dictionary_b = LW_DICTIONARY (b);
    description_b = lw_dictionary_build_id (dictionary_b);
    if (description_b != NULL)
    {
      found_b = g_hash_table_lookup_extended (hashtable, description_b, NULL, &position_b_ptr);
      position_b = GPOINTER_TO_INT (position_b_ptr);
      g_free (description_b); description_b = NULL;
    }

    //Calculate the proper order
    if (found_a && found_b)
    {
      if (position_a > position_b) order = 1;
      else if (position_a < position_b) order = -1;
      else order = 0;
    }
    else if (found_a)
    {
      order = -1;
    }
    else if (found_b)
    {
      order = 1;
    }
    else
    {
      order = 0;
    }

    return order;
}


void 
lw_dictionarylist_cancel (LwDictionaryList *dictionarylist, gboolean state)
{
    LwDictionary *dictionary;
    GList *link;

    link = dictionarylist->list;

    while (link != NULL)
    {
      dictionary = LW_DICTIONARY (link->data);

      if (dictionary != NULL)
      {
        lw_dictionary_cancel (dictionary);
      }

      link = link->next;
    }
}


//!
//! @brief Sets up the built-in installabale dictionaries
//!
void
lw_dictionarylist_load_installable (LwDictionaryList *dictionarylist, LwPreferences *preferences)
{
    LwDictionary* dictionary;

    dictionary = lw_edictionary_new ("English");
    lw_dictionary_set_builtin_installer (
      dictionary, 
      "English",
      preferences,
      LW_KEY_ENGLISH_SOURCE,
      gettext("The venerable edict by Jim Breen."),
      LW_ENCODING_EUC_JP,
      FALSE
    );
    dictionarylist->list = g_list_append (dictionarylist->list, dictionary);

    dictionary = lw_kanjidictionary_new ("Kanji");
    lw_dictionary_set_builtin_installer (
      dictionary, 
      "Kanji",
      preferences,
      LW_KEY_KANJI_SOURCE,
      gettext("A Kanji dictionary based off of kanjidic with radical information combined."),
      LW_ENCODING_EUC_JP,
      TRUE
    );
    dictionarylist->list = g_list_append (dictionarylist->list, dictionary);

    dictionary = lw_edictionary_new ("Names and Places");
    lw_dictionary_set_builtin_installer (
      dictionary, 
      "Names;Places",
      preferences,
      LW_KEY_NAMES_PLACES_SOURCE,
      gettext("Based off of Enamdic, but with the names split from the places for 2 separate dictionaries."),
      LW_ENCODING_EUC_JP,
      TRUE
    );
    dictionarylist->list = g_list_append (dictionarylist->list, dictionary);

    dictionary = lw_exampledictionary_new ("Examples");
    lw_dictionary_set_builtin_installer (
      dictionary, 
      "Examples",
      preferences,
      LW_KEY_EXAMPLES_SOURCE,
      gettext("A collection of Japanese/English sentences initially compiled "
              "by Professor Yasuhito Tanaka at Hyogo University and his students."),
      LW_ENCODING_EUC_JP,
      TRUE
    );
    dictionarylist->list = g_list_append (dictionarylist->list, dictionary);

/*
      dictionary = lw_dictionary_new (
        "",
        gettext("Other"),
        gettext("Other Dictionary"),
        gettext("Install a custom dictionary."),
        "",
        LW_TYPE_UNKNOWNDICTIONARY,
        LW_COMPRESSION_NONE,
        LW_ENCODING_UTF8,
        FALSE,
        FALSE
      );
      dictionarylist->list = g_list_append (dictionarylist->list, dictionary);
    }
*/
}


//!
//! @brief Checks to see if the current InstallDictionaryList is installation ready
//!
gboolean 
lw_dictionarylist_installer_is_valid (LwDictionaryList *dictionarylist)
{
    //Declarations
    GList *list, *link;
    LwDictionary* dictionary;
    gint number_selected;
    gboolean selected;
    gboolean valid;

    //Initializations
    link = list = dictionarylist->list;
    number_selected = 0;

    while (link != NULL)
    {
      dictionary = LW_DICTIONARY (link->data);
      valid = lw_dictionary_installer_is_valid (dictionary);
      selected = lw_dictionary_is_selected (dictionary);

      if (!valid && selected) return FALSE;
      if (selected) number_selected++;

      link = link->next;
    }

    return (number_selected > 0);
}

