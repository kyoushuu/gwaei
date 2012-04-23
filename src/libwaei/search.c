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
//! @file searchitem.c
//!


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>

static gboolean _query_is_sane (const char* query)
{
    //Declarations
    char *q;
    gboolean is_sane;

    //Initializations
    q = lw_util_prepare_query (query, TRUE); 
    is_sane = TRUE;

    //Tests
    if (strlen (q) == 0)
      is_sane = FALSE;

    if (g_str_has_prefix (q, "|") || g_str_has_prefix (q, "&")) 
      is_sane = FALSE;
    if (g_str_has_suffix (q, "\\") || g_str_has_suffix (q, "|") || g_str_has_suffix (q, "&")) 
      is_sane = FALSE;

    g_free (q);

    return is_sane;
}


//!
//! @brief Creates a new LwSearch object. 
//! @param query The text to be search for
//! @param dictionary The LwDictionary object to use
//! @param TARGET The widget to output the results to
//! @param preferences The Application preference manager to get information from
//! @param error A GError to place errors into or NULL
//! @return Returns an allocated LwSearch object that should be freed with lw_search_free or NULL on error
//!
LwSearch* 
lw_search_new (const gchar* QUERY, LwDictionary* dictionary, LwPreferences *preferences, GError **error)
{
    if (!_query_is_sane (QUERY)) return NULL;

    LwSearch *temp;

    temp = (LwSearch*) malloc(sizeof(LwSearch));

    if (temp != NULL)
    {
      lw_search_init (temp, QUERY, dictionary, preferences, error);

      if (error != NULL && *error != NULL)
      {
        lw_search_free (temp);
        temp = NULL;
      }
    }

    return temp;
}


//!
//! @brief Releases a LwSearch object from memory. 
//!
//! All of the various interally allocated memory in the LwSearch is freed.
//! The file descriptiors and such are made sure to also be closed.
//!
//! @param item The LwSearch to have it's memory freed.
//!
void 
lw_search_free (LwSearch* item)
{
    //Sanity check
    g_assert (item != NULL);

    lw_search_deinit (item);

    free (item);
}


//!
//! @brief Used to initialize the memory inside of a new LwSearch
//!        object.  Usually lw_search_new calls this for you.  It is also 
//!        used in class implimentations that extends LwSearch.
//! @param item A LwSearch to initialize the inner variables of
//! @param QUERY The text to be search for
//! @param dictionary The LwDictionary object to use
//! @param TARGET The widget to output the results to
//! @param preferences The Application preference manager to get information from
//! @param error A GError to place errors into or NULL
//!
void 
lw_search_init (LwSearch *item, const gchar* TEXT, LwDictionary* dictionary, LwPreferences *preferences, GError **error)
{
    item->results_high = NULL;
    item->results_medium = NULL;
    item->results_low = NULL;
    item->thread = NULL;
    g_mutex_init (&item->mutex);

    //Set the internal pointers to the correct global variables
    item->fd = NULL;
    item->status = LW_SEARCHSTATUS_IDLE;
    item->scratch_buffer = NULL;
    item->dictionary = dictionary;
    item->data = NULL;
    item->free_data_func = NULL;
    item->total_relevant_results = 0;
    item->total_irrelevant_results = 0;
    item->total_results = 0;
    item->current = 0L;
    item->result = NULL;
    item->query = lw_query_new ();
    item->history_relevance_idle_timer = 0;

    lw_dictionary_parse_query (item->dictionary, item->query, TEXT);
}


//!
//! @brief Used to free the memory inside of a LwSearch object.
//!         Usually lw_search_free calls this for you.  It is also used
//!         in class implimentations that extends LwSearch.
//! @param item The LwSearch object to have it's inner memory freed.
//!
void 
lw_search_deinit (LwSearch *item)
{
    lw_search_cancel_search (item);
    lw_search_clear_results (item);
    lw_search_cleanup_search (item);
    lw_query_free (item->query);
    if (lw_search_has_data (item))
      lw_search_free_data (item);

    g_mutex_clear (&item->mutex);
}


void 
lw_search_clear_results (LwSearch *item)
{
    item->total_relevant_results = 0;
    item->total_irrelevant_results = 0;
    item->total_results = 0;

    while (item->results_low != NULL)
    {
      lw_result_free (LW_RESULT (item->results_low->data));
      item->results_low = g_list_delete_link (item->results_low, item->results_low);
    }
    while (item->results_medium != NULL)
    {
      lw_result_free (LW_RESULT (item->results_medium->data));
      item->results_medium = g_list_delete_link (item->results_medium, item->results_medium);
    }
    while (item->results_high != NULL)
    {
      lw_result_free (LW_RESULT (item->results_high->data));
      item->results_high = g_list_delete_link (item->results_high, item->results_high);
    }
}


//!
//! @brief Does variable preparation required before a search
//!
//! The input and output scratch buffers have their memory allocated
//! the current_line integer is reset to 0, the comparison buffer
//! reset to it's initial state, the search status set to
//! SEARCHING, and the file descriptior is opened.
//!
//! @param item The LwSearch to its variables prepared
//! @return Returns false on seachitem prep failure.
//!
void  
lw_search_prepare_search (LwSearch* item)
{
    lw_search_clear_results (item);
    lw_search_cleanup_search (item);

    //Declarations

    //Initializations
    item->scratch_buffer = (char*) malloc (sizeof(char*) * LW_IO_MAX_FGETS_LINE);
    item->result = lw_result_new ();
    item->current = 0L;
    item->total_relevant_results = 0;
    item->total_irrelevant_results = 0;
    item->total_results = 0;
    item->thread = NULL;
    item->fd = lw_dictionary_open (LW_DICTIONARY (item->dictionary));
    item->status = LW_SEARCHSTATUS_SEARCHING;
}


//!
//! @brief Cleanups after a search completes
//!
//! The file descriptior is closed, various variables are
//! reset, and the search status is set to IDLE.
//!
//! @param item The LwSearch to its state reset.
//!
void 
lw_search_cleanup_search (LwSearch* item)
{
    if (item->fd != NULL)
    {
      fclose(item->fd);
      item->fd = NULL;
    }

    if (item->scratch_buffer != NULL)
    {
      free(item->scratch_buffer);
      item->scratch_buffer = NULL;
    }

    if (item->result != NULL)
    {
      lw_result_free (item->result);
      item->result = NULL;
    }

    item->status = LW_SEARCHSTATUS_FINISHING;
}


//!
//! @brief comparison function for determining if two LwSearchs are equal
//! @param item1 The first item
//! @param item2 The second item
//! @returns Returns true when both items are either the same item or have similar innards
//!
gboolean 
lw_search_is_equal (LwSearch *item1, LwSearch *item2)
{
  //Declarations
  gboolean queries_are_equal;
  gboolean dictionaries_are_equal;
  //Sanity checks
  if (item1 == NULL)
  {
    return FALSE;
  }
  if (item2 == NULL)
  {
    return FALSE;
  }

  if (item1 == item2)
  {
    return TRUE;
  }

  //Initializations
  queries_are_equal = (strcmp(item1->query->text, item2->query->text) == 0);
  dictionaries_are_equal = (item1->dictionary == item2->dictionary);

  return (queries_are_equal && dictionaries_are_equal);
}


//!
//! @brief a method for incrementing an internal integer for determining if a result set has worth
//! @param item The LwSearch to increment the timer on
//!
void 
lw_search_increment_history_relevance_timer (LwSearch *item)
{
  if (item != NULL && item->history_relevance_idle_timer < LW_HISTORY_TIME_TO_RELEVANCE)
    item->history_relevance_idle_timer++;
}


//!
//! @brief Checks if the relevant timer has passed a threshold
//! @param item The LwSearch to check for history relevance
//! @param use_idle_timer This variable shoud be set to true if the program does automatic searches so it checks the timer
//!
gboolean 
lw_search_has_history_relevance (LwSearch *item, gboolean use_idle_timer)
{
  return (item != NULL && 
          item->total_results > 0 && 
          (!use_idle_timer || item->history_relevance_idle_timer >= LW_HISTORY_TIME_TO_RELEVANCE));
}


//!
//! @brief Used to set custom search data (Such as Window or TextView pointers)
//! @param item The LwSearch to set the data on.  It will free any previous data if it is already set.
//! @param data The data to set.
//! @param free_data_func A callback to use to free the data automatically as needed
//!
void 
lw_search_set_data (LwSearch *item, gpointer data, LwSearchDataFreeFunc free_data_func)
{
    //Sanity check
    g_assert (item != NULL);

    if (lw_search_has_data (item))
      lw_search_free_data (item);

    item->data = data;
    item->free_data_func = free_data_func;
}


//!
//! @brief to retieve custom search data (Such as Window or TextView pointers)
//! @param item The LwSearch object to retrieve the data on.
//! @returns A generic pointer to the data that should be cast.
//!
gpointer 
lw_search_get_data (LwSearch *item)
{
    //Sanity check
    g_assert (item != NULL);

    return item->data;
}


//!
//! @brief Frees the data on an LwSearch object if it exists
//! @param item The LwSearch to free the data on
//!
void 
lw_search_free_data (LwSearch *item)
{
    //Sanity check
    g_assert (item != NULL);

    if (item->free_data_func != NULL && item->data != NULL)
    {
      (item->free_data_func) (item->data);
    }

    item->data = NULL;
    item->free_data_func = NULL;
}


//!
//! @brief Returns true if the LwSearch had its data set
//! @param item An LwSearch to check for data
//! @returns Returns true if the data is not NULL
//!
gboolean 
lw_search_has_data (LwSearch *item)
{
    g_assert (item != NULL);

    return (item->data != NULL && item->free_data_func != NULL);
}


//!
//! @brief A wrapper around gmutex made for LwSearch objects
//! @param item An LwSearch to lock the mutex on
//!
void 
lw_search_lock (LwSearch *item)
{
  g_mutex_lock (&item->mutex);
}

//!
//! @brief A wrapper around gmutex made for LwSearch objects
//! @param item An LwSearch to unlock the mutex on
//!
void 
lw_search_unlock (LwSearch *item)
{
  g_mutex_unlock (&item->mutex);
}


gdouble 
lw_search_get_progress (LwSearch *item)
{
    //Declarations
    long current;
    long length;
    double fraction;

    //Initializations
    current = 0L;
    length = 0L;
    fraction = 0.0;

    if (item != NULL && item->dictionary != NULL && item->status == LW_SEARCHSTATUS_SEARCHING)
    {
      current = item->current;
      length = lw_dictionary_get_length(LW_DICTIONARY (item->dictionary));

      if (current > 0L && length > 0L && current != length) 
        fraction = (gdouble) current / (gdouble) length;
    }

    return fraction;
}


void
lw_search_set_status (LwSearch *item, LwSearchStatus status)
{
    lw_search_lock (item);
    item->status = status;
    lw_search_unlock (item);
}


LwSearchStatus
lw_search_get_status (LwSearch *item)
{
    LwSearchStatus status;
    lw_search_lock (item);
    status = item->status;
    lw_search_unlock (item);

    return status;
}

