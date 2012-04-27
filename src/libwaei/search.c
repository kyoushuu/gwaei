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
//! @param search The LwSearch to have it's memory freed.
//!
void 
lw_search_free (LwSearch* search)
{
    //Sanity check
    g_assert (search != NULL);

    lw_search_deinit (search);

    free (search);
}


//!
//! @brief Used to initialize the memory inside of a new LwSearch
//!        object.  Usually lw_search_new calls this for you.  It is also 
//!        used in class implimentations that extends LwSearch.
//! @param search A LwSearch to initialize the inner variables of
//! @param QUERY The text to be search for
//! @param dictionary The LwDictionary object to use
//! @param TARGET The widget to output the results to
//! @param preferences The Application preference manager to get information from
//! @param error A GError to place errors into or NULL
//!
void 
lw_search_init (LwSearch *search, const gchar* TEXT, LwDictionary* dictionary, LwPreferences *preferences, GError **error)
{
    memset(search, 0, sizeof(LwSearch));

    g_mutex_init (&search->mutex);
    search->status = LW_SEARCHSTATUS_IDLE;
    search->dictionary = dictionary;
    search->query = lw_query_new ();

    lw_dictionary_parse_query (search->dictionary, search->query, TEXT, error);
}


//!
//! @brief Used to free the memory inside of a LwSearch object.
//!         Usually lw_search_free calls this for you.  It is also used
//!         in class implimentations that extends LwSearch.
//! @param search The LwSearch object to have it's inner memory freed.
//!
void 
lw_search_deinit (LwSearch *search)
{
    lw_search_cancel_search (search);
    lw_search_clear_results (search);
    lw_search_cleanup_search (search);
    lw_query_free (search->query);
    if (lw_search_has_data (search))
      lw_search_free_data (search);

    g_mutex_clear (&search->mutex);
}


void 
lw_search_clear_results (LwSearch *search)
{
    search->total_relevant_results = 0;
    search->total_irrelevant_results = 0;
    search->total_results = 0;

    while (search->results_low != NULL)
    {
      lw_result_free (LW_RESULT (search->results_low->data));
      search->results_low = g_list_delete_link (search->results_low, search->results_low);
    }
    while (search->results_medium != NULL)
    {
      lw_result_free (LW_RESULT (search->results_medium->data));
      search->results_medium = g_list_delete_link (search->results_medium, search->results_medium);
    }
    while (search->results_high != NULL)
    {
      lw_result_free (LW_RESULT (search->results_high->data));
      search->results_high = g_list_delete_link (search->results_high, search->results_high);
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
//! @param search The LwSearch to its variables prepared
//! @return Returns false on seachitem prep failure.
//!
void  
lw_search_prepare_search (LwSearch* search)
{
    lw_search_clear_results (search);
    lw_search_cleanup_search (search);

    //Declarations

    //Initializations
    search->scratch_buffer = (char*) malloc (sizeof(char*) * LW_IO_MAX_FGETS_LINE);
    search->result = lw_result_new ();
    search->current = 0L;
    search->total_relevant_results = 0;
    search->total_irrelevant_results = 0;
    search->total_results = 0;
    search->thread = NULL;
    search->fd = lw_dictionary_open (LW_DICTIONARY (search->dictionary));
    search->status = LW_SEARCHSTATUS_SEARCHING;
}


//!
//! @brief Cleanups after a search completes
//!
//! The file descriptior is closed, various variables are
//! reset, and the search status is set to IDLE.
//!
//! @param search The LwSearch to its state reset.
//!
void 
lw_search_cleanup_search (LwSearch* search)
{
    if (search->fd != NULL)
    {
      fclose(search->fd);
      search->fd = NULL;
    }

    if (search->scratch_buffer != NULL)
    {
      free(search->scratch_buffer);
      search->scratch_buffer = NULL;
    }

    if (search->result != NULL)
    {
      lw_result_free (search->result);
      search->result = NULL;
    }

    search->status = LW_SEARCHSTATUS_FINISHING;
}


//!
//! @brief comparison function for determining if two LwSearchs are equal
//! @param item1 The first search
//! @param item2 The second search
//! @returns Returns true when both items are either the same search or have similar innards
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
//! @param search The LwSearch to increment the timer on
//!
void 
lw_search_increment_history_relevance_timer (LwSearch *search)
{
  if (search != NULL && search->history_relevance_idle_timer < LW_HISTORY_TIME_TO_RELEVANCE)
    search->history_relevance_idle_timer++;
}


//!
//! @brief Checks if the relevant timer has passed a threshold
//! @param search The LwSearch to check for history relevance
//! @param use_idle_timer This variable shoud be set to true if the program does automatic searches so it checks the timer
//!
gboolean 
lw_search_has_history_relevance (LwSearch *search, gboolean use_idle_timer)
{
  return (search != NULL && 
          search->total_results > 0 && 
          (!use_idle_timer || search->history_relevance_idle_timer >= LW_HISTORY_TIME_TO_RELEVANCE));
}


//!
//! @brief Used to set custom search data (Such as Window or TextView pointers)
//! @param search The LwSearch to set the data on.  It will free any previous data if it is already set.
//! @param data The data to set.
//! @param free_data_func A callback to use to free the data automatically as needed
//!
void 
lw_search_set_data (LwSearch *search, gpointer data, LwSearchDataFreeFunc free_data_func)
{
    //Sanity check
    g_assert (search != NULL);

    if (lw_search_has_data (search))
      lw_search_free_data (search);

    search->data = data;
    search->free_data_func = free_data_func;
}


//!
//! @brief to retieve custom search data (Such as Window or TextView pointers)
//! @param search The LwSearch object to retrieve the data on.
//! @returns A generic pointer to the data that should be cast.
//!
gpointer 
lw_search_get_data (LwSearch *search)
{
    //Sanity check
    g_assert (search != NULL);

    return search->data;
}


//!
//! @brief Frees the data on an LwSearch object if it exists
//! @param search The LwSearch to free the data on
//!
void 
lw_search_free_data (LwSearch *search)
{
    //Sanity check
    g_assert (search != NULL);

    if (search->free_data_func != NULL && search->data != NULL)
    {
      (search->free_data_func) (search->data);
    }

    search->data = NULL;
    search->free_data_func = NULL;
}


//!
//! @brief Returns true if the LwSearch had its data set
//! @param search An LwSearch to check for data
//! @returns Returns true if the data is not NULL
//!
gboolean 
lw_search_has_data (LwSearch *search)
{
    g_assert (search != NULL);

    return (search->data != NULL && search->free_data_func != NULL);
}


//!
//! @brief A wrapper around gmutex made for LwSearch objects
//! @param search An LwSearch to lock the mutex on
//!
void 
lw_search_lock (LwSearch *search)
{
  g_mutex_lock (&search->mutex);
}

//!
//! @brief A wrapper around gmutex made for LwSearch objects
//! @param search An LwSearch to unlock the mutex on
//!
void 
lw_search_unlock (LwSearch *search)
{
  g_mutex_unlock (&search->mutex);
}


gdouble 
lw_search_get_progress (LwSearch *search)
{
    //Declarations
    long current;
    long length;
    double fraction;

    //Initializations
    current = 0L;
    length = 0L;
    fraction = 0.0;

    if (search != NULL && search->dictionary != NULL && search->status == LW_SEARCHSTATUS_SEARCHING)
    {
      current = search->current;
      length = lw_dictionary_get_length(LW_DICTIONARY (search->dictionary));

      if (current > 0L && length > 0L && current != length) 
        fraction = (gdouble) current / (gdouble) length;
    }

    return fraction;
}


void
lw_search_set_status (LwSearch *search, LwSearchStatus status)
{
    lw_search_lock (search);
    search->status = status;
    lw_search_unlock (search);
}


LwSearchStatus
lw_search_get_status (LwSearch *search)
{
    LwSearchStatus status;
    lw_search_lock (search);
    status = search->status;
    lw_search_unlock (search);

    return status;
}


gboolean
lw_search_read_line (LwSearch *search)
{
    gchar *ptr;
    ptr = fgets(search->result->text, LW_IO_MAX_FGETS_LINE, search->fd);

    return (ptr != NULL && search->status != LW_SEARCHSTATUS_FINISHING);
}


gboolean 
lw_search_compare (LwSearch *search, const LwRelevance RELEVANCE)
{
    return lw_dictionary_compare (search->dictionary, search->query, search->result, RELEVANCE);
}


//!
//! @brief Find the relevance of a returned result
//!
//! THIS IS A PRIVATE FUNCTION. Function uses the stored relevance regrex
//! expressions in the LwSearch to get the relevance of a returned result.  It
//! then returns the answer to the caller in the form of an int.
//!
//! @param text a string to check the relevance of
//! @param search a search search to grab the regrexes from
//! @return Returns one of the integers: LOW_RELEVANCE, MEDIUM_RELEVANCE, or HIGH_RELEVANCE.
//!
static int 
lw_search_get_relevance (LwSearch *search) {
    if (lw_search_compare (search, LW_RELEVANCE_HIGH))
      return LW_RELEVANCE_HIGH;
    else if (lw_search_compare (search, LW_RELEVANCE_MEDIUM))
      return LW_RELEVANCE_MEDIUM;
    else
      return LW_RELEVANCE_LOW;
}


//!
//! @brief Preforms the brute work of the search
//!
//! THIS IS A PRIVATE FUNCTION. This function returns true until it finishes
//! searching the whole file.  It works in specified chunks before going back to
//! the thread to help improve speed.  
//!
//! @param data A LwSearch to search with
//! @return Returns true when the search isn't finished yet.
//!
static gpointer 
lw_search_stream_results_thread (gpointer data)
{
    //Declarations
    LwSearch *search;
    gboolean show_only_exact_matches;

    //Initializations
    search = LW_SEARCHITEM (data);
    show_only_exact_matches = search->exact;

    if (search == NULL || search->fd == NULL) return NULL;

    lw_search_lock (search);
    search->status = LW_SEARCHSTATUS_SEARCHING;

    //We loop, processing lines of the file until the max chunk size has been
    //reached or we reach the end of the file or a cancel request is recieved.
    while (lw_search_read_line (search))
    {
      //Give a chance for something else to run
      lw_search_unlock (search);
      if (search->status != LW_SEARCHSTATUS_FINISHING && g_main_context_pending (NULL))
      {
        g_main_context_iteration (NULL, FALSE);
      }
      lw_search_lock (search);

      search->current += strlen(search->result->text);

      //Commented input in the dictionary...we should skip over it
      if(search->result->text[0] == '#' || g_utf8_get_char(search->result->text) == L'？') 
      {
        continue;
      }
      else if (search->result->text[0] == 'A' && search->result->text[1] == ':' &&
               fgets(search->scratch_buffer, LW_IO_MAX_FGETS_LINE, search->fd) != NULL             )
      {
        search->current += strlen(search->scratch_buffer);
        char *eraser = NULL;
        if ((eraser = g_utf8_strchr (search->result->text, -1, L'\n')) != NULL) { *eraser = '\0'; }
        if ((eraser = g_utf8_strchr (search->scratch_buffer, -1, L'\n')) != NULL) { *eraser = '\0'; }
        if ((eraser = g_utf8_strrchr (search->result->text, -1, L'#')) != NULL) { *eraser = '\0'; }
        strcat(search->result->text, ":");
        strcat(search->result->text, search->scratch_buffer);
      }
      lw_search_parse_result_string (search);


      //Results match, add to the text buffer
      if (lw_search_compare (search, LW_RELEVANCE_LOW))
      {
        int relevance = lw_search_get_relevance (search);
        switch(relevance)
        {
          case LW_RELEVANCE_HIGH:
              if (search->total_relevant_results < LW_MAX_HIGH_RELEVENT_RESULTS)
              {
                search->total_results++;
                search->total_relevant_results++;
                search->result->relevance = LW_RESULT_RELEVANCE_HIGH;
                search->results_high =  g_list_append (search->results_high, search->result);
                search->result = lw_result_new ();
              }
              break;
          if (!show_only_exact_matches)
          {
            case LW_RELEVANCE_MEDIUM:
                if (search->total_irrelevant_results < LW_MAX_MEDIUM_IRRELEVENT_RESULTS)
                {
                  search->total_results++;
                  search->total_irrelevant_results++;
                  search->result->relevance = LW_RESULT_RELEVANCE_MEDIUM;
                  search->results_medium =  g_list_append (search->results_medium, search->result);
                  search->result = lw_result_new ();
                }
                break;
            default:
                if (search->total_irrelevant_results < LW_MAX_LOW_IRRELEVENT_RESULTS)
                {
                  search->total_results++;
                  search->total_irrelevant_results++;
                  search->result->relevance = LW_RESULT_RELEVANCE_LOW;
                  search->results_low = g_list_append (search->results_low, search->result);
                  search->result = lw_result_new ();
                }
                break;
          }
        }
      }
    }

    lw_search_cleanup_search (search);

    lw_search_unlock (search);

    return NULL;
}


//!
//! @brief Start a dictionary search
//! @param search a LwSearch argument to calculate results
//! @param create_thread Whether the search should run in a new thread.
//! @param exact Whether to show only exact matches for this search
//!
void 
lw_search_start_search (LwSearch *search, gboolean create_thread)
{
    GError *error;

    error = NULL;

    lw_search_prepare_search (search);
    if (create_thread)
    {
      search->thread = g_thread_try_new (
        "libwaei-search",
        (GThreadFunc) lw_search_stream_results_thread, 
        (gpointer) search, 
        &error
      );
      if (search->thread == NULL)
      {
        g_warning ("Thread Creation Error: %s\n", error->message);
        g_error_free (error);
        error = NULL;
      }
    }
    else
    {
      search->thread = NULL;
      lw_search_stream_results_thread ((gpointer) search);
    }
}


//!
//! @brief Uses a searchitem to cancel a window
//!
//! @param search A LwSearch to gleam information from
//!
void 
lw_search_cancel_search (LwSearch *search)
{
    if (search == NULL) return;

    lw_search_set_status (search, LW_SEARCHSTATUS_FINISHING);

    if (search->thread != NULL)
    {
      g_thread_join (search->thread);
      search->thread = NULL;
    }

    lw_search_set_status (search, LW_SEARCHSTATUS_IDLE);
}


//!
//! @brief Gets a result and removes a LwResult from the beginnig of a list of results
//! @returns a LwResult that should be freed with lw_result_free
//!
LwResult* 
lw_search_get_result (LwSearch *search)
{
    g_assert (search != NULL);

    LwResult *line;

    lw_search_lock (search);

    if (search->results_high != NULL)
    {
      line = LW_RESULT (search->results_high->data);
      search->results_high = g_list_delete_link (search->results_high, search->results_high);
    }
    else if (search->results_medium != NULL && search->status == LW_SEARCHSTATUS_IDLE)
    {
      line = LW_RESULT (search->results_medium->data);
      search->results_medium = g_list_delete_link (search->results_medium, search->results_medium);
    }
    else if (search->results_low != NULL && search->status == LW_SEARCHSTATUS_IDLE)
    {
      line = LW_RESULT (search->results_low->data);
      search->results_low = g_list_delete_link (search->results_low, search->results_low);
    }
    else
    {
      line = NULL;
    }

    lw_search_unlock (search);

    return line;
}



//!
//! @brief Tells if you should keep checking for results
//!
gboolean 
lw_search_should_check_results (LwSearch *search)
{
    if (search == NULL) return FALSE;

    gboolean should_check_results;
    LwSearchStatus status;

    status = lw_search_get_status (search);

    if (status == LW_SEARCHSTATUS_FINISHING)
    {
      lw_search_cancel_search (search);
      should_check_results = FALSE;
    }
    else
    {
      lw_search_lock (search);
      should_check_results = (status != LW_SEARCHSTATUS_IDLE ||
                              search->results_high != NULL ||
                              search->results_medium != NULL ||
                              search->results_low != NULL);
      lw_search_unlock (search);
    }

    return should_check_results;
}


