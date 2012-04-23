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
//! @file engine.c
//!

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>
#include <libwaei/engine-data.h>


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
static int _get_relevance (LwSearch *search) {
    if (lw_search_run_comparison (search, LW_RELEVANCE_HIGH))
      return LW_RELEVANCE_HIGH;
    else if (lw_search_run_comparison (search, LW_RELEVANCE_MEDIUM))
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
static gpointer _stream_results_thread (gpointer data)
{
    //Declarations
    LwEngineData *enginedata;
    LwSearch *search;
    gboolean show_only_exact_matches;

    //Initializations
    enginedata = LW_ENGINEDATA (data);
    search = LW_SEARCHITEM (enginedata->search);
    show_only_exact_matches = enginedata->exact;

    if (search == NULL || search->fd == NULL) return NULL;
    char *line_pointer = NULL;

    lw_search_lock (search);
    search->status = LW_SEARCHSTATUS_SEARCHING;

    //We loop, processing lines of the file until the max chunk size has been
    //reached or we reach the end of the file or a cancel request is recieved.
    while ((line_pointer = fgets(search->result->text, LW_IO_MAX_FGETS_LINE, search->fd)) != NULL &&
           search->status != LW_SEARCHSTATUS_FINISHING)
    {

      //Give a chance for something else to run
      lw_search_unlock (search);
/*
      //THIS CODE CAUSES A DEADLOCK ON GTK+3.3.X
      if (search->status != LW_SEARCHSTATUS_FINISHING && g_main_context_pending (NULL))
      {
        g_main_context_iteration (NULL, FALSE);
      }
*/
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
      if (lw_search_run_comparison (search, LW_RELEVANCE_LOW))
      {
        int relevance = _get_relevance (search);
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
    lw_enginedata_free (enginedata);

    lw_search_unlock (search);

    return NULL;
}


//!
//! @brief Start a dictionary search
//! @param search a LwSearch argument to calculate results
//! @param create_thread Whether the search should run in a new thread.
//! @param exact Whether to show only exact matches for this search
//!
void lw_search_start_search (LwSearch *search, gboolean create_thread, gboolean exact)
{
    GError *error;
    gpointer data;

    error = NULL;
    data = lw_enginedata_new (search, exact);

    if (data != NULL)
    {
      lw_search_prepare_search (search);
      if (create_thread)
      {
        search->thread = g_thread_try_new (
          "libwaei-search",
          (GThreadFunc) _stream_results_thread, 
          (gpointer) data, 
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
        _stream_results_thread ((gpointer) data);
      }
    }
}


//!
//! @brief Uses a searchitem to cancel a window
//!
//! @param search A LwSearch to gleam information from
//!
void lw_search_cancel_search (LwSearch *search)
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
LwResult* lw_search_get_result (LwSearch *search)
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
gboolean lw_search_should_check_results (LwSearch *search)
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


