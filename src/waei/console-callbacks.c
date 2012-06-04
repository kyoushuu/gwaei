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
    LwDictionaryInstallerStatus status;
    gchar *message;
    gdouble stage_fraction;
    gint stage_percent;

    //Initializations
    dictionary = LW_DICTIONARY (data);
    status = lw_dictionary_installer_get_status (dictionary);
    stage_fraction = lw_dictionary_installer_get_stage_progress (dictionary, fraction);
    stage_percent = (gint) (100.0 * stage_fraction); 

    //Update the dictinst progress state only when the delta is large enough
    if (stage_percent < 100 && _group_index_changed)
    {
      _group_index_changed = FALSE;
      printf("\n");
    }
    else if (stage_percent == 100)
    {
      _group_index_changed = TRUE;
    }

    message = lw_dictionary_installer_get_status_string (dictionary, TRUE);
    if (message != NULL && fraction == 1.0 && (status == LW_DICTIONARY_INSTALLER_STATUS_UNINSTALLED || status == LW_DICTIONARY_INSTALLER_STATUS_INSTALLED))
      fprintf(stdout, "\n%s\n\n", message);
    else if (message != NULL && _previous_percent != stage_percent)
      fprintf(stdout, "\r [%d%%] %s", stage_percent, message);
    fflush(stdout);
    _previous_percent = stage_percent;
    if (message != NULL) g_free (message); message = NULL;

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

