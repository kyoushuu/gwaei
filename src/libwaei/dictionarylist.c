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

#include <libwaei/libwaei.h>


//Private methods
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



void 
lw_dictionarylist_init (LwDictionaryList *dictionarylist)
{
    dictionarylist->list = NULL;
    g_mutex_init (&dictionarylist->mutex);

    lw_dictionarylist_reload (dictionarylist);
}


void 
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
lw_dictionarylist_reload (LwDictionaryList *dictionarylist)
{
    //Declarations
    gchar** filelist;
    gchar **ptr;
    gchar** pair;
    GType type;
    LwDictionary *dictionary;
    const gchar *FILENAME;

    lw_dictionarylist_clear (dictionarylist);

    filelist = lw_io_get_dictionary_file_list (100);
    if (filelist != NULL)
    {
      for (ptr = filelist; *ptr != NULL; ptr++)
      {
        pair = g_strsplit_set (*ptr, "/", 2);
        if (pair != NULL && pair[0] != NULL && pair[1] != NULL) 
        {
          type = g_type_from_name (pair[0]);
          FILENAME = pair[1];
          dictionary = LW_DICTIONARY (g_object_new (type, "dictionary-filename", FILENAME, NULL));
          if (dictionary != NULL && LW_IS_DICTIONARY (dictionary))
            lw_dictionarylist_add_dictionary (dictionarylist, dictionary);
        }
        g_strfreev (pair); pair = NULL;
      }
      g_strfreev (filelist); filelist = NULL;
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
    if (FUZZY_DESCRIPTION == NULL )
    {
      if (dictionarylist->list != NULL)
        dictionary = (LwDictionary*) dictionarylist->list->data;
      else
        dictionary = NULL;
    }

    //Otherwise try getting a dictionary using a few different string parsers
    else
    {
      if (dictionary == NULL)
        dictionary = lw_dictionarylist_get_dictionary_by_idstring (dictionarylist, FUZZY_DESCRIPTION);
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
      if (g_ascii_strcasecmp (FILENAME2, FILENAME) == 0)
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
lw_dictionarylist_get_dictionary_by_id_string (LwDictionaryList *dictionarylist, const char* ENGINE_AND_FILENAME)
{
    //Sanity checks
    g_assert (ENGINE_AND_FILENAME != NULL);

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

    //Return true if the dictionary exists
    for (link = dictionarylist->list; link != NULL && exists == FALSE; link = link->next)
    {
      exists = lw_dictionary_equals (dictionary, LW_DICTIONARY (link->data));
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
        *ptr = lw_dictionary_build_description (dictionary);
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

    const int MAX = 1000;
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
    description_a = lw_dictionary_build_description (dictionary_a);
    if (description_a != NULL)
    {
      found_a = g_hash_table_lookup_extended (hashtable, &description_a, NULL, &position_a_ptr);
      position_a = GPOINTER_TO_INT (position_a_ptr);
      g_free (description_a); description_a = NULL;
    }
    
    dictionary_b = LW_DICTIONARY (b);
    description_b = lw_dictionary_build_description (dictionary_b);
    if (description_b != NULL)
    {
      found_b = g_hash_table_lookup_extended (hashtable, &description_b, NULL, &position_b_ptr);
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
      order = 1;
    }
    else if (found_b)
    {
      order = -1;
    }
    else
    {
      order = 0;
    }

    return order;
}




//!
//! @brief Checks to see if the current InstallDictionaryList is installation ready
//!
gboolean 
lw_dictionarylist_data_is_valid (LwInstallDictionaryList *dil)
{
    //Declarations
    GList *iter;
    LwInstallDictionary* di;
    int number_selected;

    //Initializations
    number_selected = 0;

    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
      di = LwInstallDictionary (iter->data);
      if (!lw_dictinst_data_is_valid (di) && di->selected) return FALSE;
      if (di->selected) number_selected++;
    }
    return (number_selected > 0);
}


//!
//!  @brief  Gets a LwInstallDictionary object by a fuzzy string description.
//!          It can be either of the form "parser/dictionary" or 
//!          just be the dictionary name.  Case is ignored.
//!  @param FUZZY_DESCRIPTION A fuzzy description of the wanted dictionary.
//!  @returns A matching LwInstallDictionary object or NULL
//!
LwInstallDictionary* 
lw_dictionarylist_get_dictinst_fuzzy (LwInstallDictionaryList *dil, const char* FUZZY_DESCRIPTION)
{
    //Declarations
    LwInstallDictionary *di;

    //Initializations
    di = NULL;

    //Try getting the first dictionary if none is specified
    if (FUZZY_DESCRIPTION == NULL )
    {
      if (g_list_length (dil->list))
        di = dil->list->data;
      else
        di = NULL;
    }

    //Otherwise try getting a dictionary using a few different string parsers
    else
    {
      if (di == NULL)
        di = lw_dictionarylist_get_dictinst_by_idstring (dil, FUZZY_DESCRIPTION);
      if (di == NULL)
        di = lw_dictionarylist_get_dictinst_by_filename (dil, FUZZY_DESCRIPTION);
    }

    return di;
}

//!
//! @brief Grabs the first dictionary with a matching dictionary 
//!        filename.  If you have dictionaries with different 
//!        parsers but the same name, the others will not 
//!        be accessible with this function.
//! @param NAME A constant string to search for in the dictionary names.  
//!             This is a fuzzy search, ignoring DICTTYPE and case
//! @returns The requested LwInstallDictionary object if found or null.
//!
LwInstallDictionary* 
lw_dictionarylist_get_dictinst_by_filename (LwInstallDictionaryList *dil, const char* FILENAME)
{
    //Declarations
    GList *iter;
    LwInstallDictionary *di;

    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
      di = LwInstallDictionary (iter->data)
      if (g_ascii_strcasecmp (di->filename, FILENAME) == 0)
        break;
      di = NULL;
    }

    return di;
}


//!
//! @brief Finds a dictionary by using an id string of the form of 
//!        "engine/dictionary". Case is ignored.
//! @param ENGINE_AND_FILENAME A string in the form "engine/dictionary"
//!                            used to search for a dictionary
//! @returns The requested LwInstallDictionary object if found or NULL.
//!
LwInstallDictionary* 
lw_dictionarylist_get_dictinst_by_idstring (LwInstallDictionaryList *dil, const char* ENGINE_AND_FILENAME)
{
    //Declarations
    GList *iter;
    LwInstallDictionary *di;
    char **tokens;
    char *filename;
    LwDictType engine;

    //Initializations
    iter = NULL;
    di = NULL;
    tokens = g_strsplit (ENGINE_AND_FILENAME, "/", 2);

    if (g_strv_length (tokens) == 2)
    {
      engine = lw_util_get_dicttype_from_string (tokens[0]);
      filename = tokens[1];

      for (iter = dil->list; iter != NULL; iter = iter->next)
      {
        di = LwInstallDictionary (iter->data);
        if (di->type == engine && g_ascii_strcasecmp (di->filename, filename) == 0)
          break;
        di = NULL;
      }
    }

    g_strfreev (tokens);

    return di;
}


void 
lw_dictionarylist_set_cancel_operations (LwInstallDictionaryList *dil, gboolean state)
{
    LwInstallDictionary *di;
    GList *iter;

    for (iter = dil->list; iter != NULL; iter = iter->next)
    {
      di = LwInstallDictionary (iter->data);
      if (di != NULL)
        lw_dictinst_set_cancel_operations (di, state);
    }

    dil->cancel = state;
}


//!
//! @brief Sets up the built-in installabale dictionaries
//!
LwDictionaryList* 
lw_dictionarylist_build_install_list (LwPreferences *preferences)
{

  LwDictionary *dictionary;
  LwDictionaryList *dictionarylist;

  dictionarylist = lw_dictionarylist_new ();

  if (dictionarylist != NULL)
  {
    dictionarylist->list = NULL;
    dictionarylist->cancel = FALSE;

    dictionary = lw_dictionary_new_using_pref_uri (
      "English",
      gettext("English"),
      gettext("English Dictionary"),
      gettext("The venerable Japanese-English Dictionary developed by Jim Breen."),
      preferences,
      LW_SCHEMA_DICTIONARY,
      LW_KEY_ENGLISH_SOURCE,
      LW_DICTTYPE_EDICT,
      LW_COMPRESSION_GZIP,
      LW_ENCODING_EUC_JP,
      FALSE,
      FALSE,
      TRUE 
    );
    dictionarylist->list = g_list_append (dictionarylist->list, dictionary);

    dictionary = lw_dictionary_new_using_pref_uri (
      "Kanji",
      gettext("Kanji"),
      gettext("Kanji Dictionary"),
      gettext("A Kanji dictionary based off of kanjidic with radical information combined."),
      preferences,
      LW_SCHEMA_DICTIONARY,
      LW_KEY_KANJI_SOURCE,
      LW_DICTTYPE_KANJI,
      LW_COMPRESSION_GZIP,
      LW_ENCODING_EUC_JP,
      FALSE,
      TRUE,
      TRUE 
    );
    dictionarylist->list = g_list_append (dictionarylist->list, dictionary);

    dictionary = lw_dictionary_new_using_pref_uri (
      "Names and Places",
      gettext("Names and Places"),
      gettext("Names and Places Dictionary"),
      gettext("Based off of Enamdic, but with the names split from the places for 2 separate dictionaries."),
      preferences,
      LW_SCHEMA_DICTIONARY,
      LW_KEY_NAMES_PLACES_SOURCE,
      LW_DICTTYPE_EDICT,
      LW_COMPRESSION_GZIP,
      LW_ENCODING_EUC_JP,
      TRUE,
      FALSE,
      TRUE 
    );
    dictionarylist->list = g_list_append (dictionarylist->list, dictionary);

    dictionary = lw_dictionary_new_using_pref_uri (
      "Examples",
      gettext("Examples"),
      gettext("Examples Dictionary"),
      gettext("A collection of Japanese/English sentences initially compiled "
              "by Professor Yasuhito Tanaka at Hyogo University and his students."),
      preferences,
      LW_SCHEMA_DICTIONARY,
      LW_KEY_EXAMPLES_SOURCE,
      LW_DICTTYPE_EXAMPLES,
      LW_COMPRESSION_GZIP,
      LW_ENCODING_EUC_JP,
      FALSE,
      FALSE,
      TRUE 
    );
    dictionarylist->list = g_list_append (dictionarylist->list, dictionary);

    dictionary = lw_dictionary_new (
      "",
      gettext("Other"),
      gettext("Other Dictionary"),
      gettext("Install a custom dictionary."),
      "",
      LW_DICTTYPE_UNKNOWN,
      LW_COMPRESSION_NONE,
      LW_ENCODING_UTF8,
      FALSE,
      FALSE,
      FALSE
    );
    dictionarylist->list = g_list_append (dictionarylist->list, dictionary);

  }

  return dictionarylist;
}


  curl_global_init (CURL_GLOBAL_ALL);
    curl_global_cleanup ();

