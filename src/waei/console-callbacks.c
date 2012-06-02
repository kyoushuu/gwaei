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
//! @file console-callbacks.c
//!
//! @brief To be written
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <waei/waei.h>

gint 
w_console_uninstall_progress_cb (gdouble fraction, gpointer data)
{
    //Declarations
    LwDictionary *dictionary;
    gchar *path;

    //Initializations
    dictionary = data;
    path = lw_dictionary_get_path (dictionary);
    
    if (path != NULL)
    {
      printf("Removing %s...\n", path);
      g_free (path); path = NULL;
    }

    return FALSE;
}




static gboolean _group_index_changed = FALSE;
static gint _previous_percent = -1;

gint 
w_console_install_progress_cb (gdouble fraction, gpointer data)
{
  //Declarations
  LwDictionary *dictionary;
  gchar *status;
  gdouble current_fraction;
  gint current_percent;

  //Initializations
  dictionary = data;
  current_fraction = lw_dictionary_installer_get_step_progress (dictionary);
  current_percent = (gint) (100.0 * current_fraction); 

  //Update the dictinst progress state only when the delta is large enough
  if (current_percent < 100 && _group_index_changed)
  {
    _group_index_changed = FALSE;
    printf("\n");
  }
  else if (current_percent == 100)
  {
    _group_index_changed = TRUE;
  }

  status = lw_dictionary_installer_get_status_string (dictionary, TRUE);
  if (status != NULL)
  {
    printf("\r [%d%%] %s", current_percent, status);
    _previous_percent = current_percent;
    g_free (status); status = NULL;
  }

  return FALSE;
}


gboolean 
w_console_append_result_timeout (gpointer data)
{
  //Sanity checks
  g_return_val_if_fail (data != NULL, FALSE);

  //Declarations
  LwSearch *search;
  LwSearchStatus status;
  WSearchData *sdata;
  gint chunk;
  gboolean keep_appending;

  //Initializations
  search = LW_SEARCH (data);
  status = lw_search_get_status (search);
  sdata = W_SEARCHDATA (lw_search_get_data (search));
  chunk = 50;

  if  (status != LW_SEARCHSTATUS_IDLE)
  {
    while (lw_search_has_results (search) && chunk-- > 0)
    {
      w_console_append_result (sdata->application, search);
    }
    keep_appending = TRUE;
  }
  else
  {
      w_console_no_result (sdata->application, search);
      g_main_loop_quit (sdata->loop);
      keep_appending = FALSE;
  }

  return keep_appending;
}

