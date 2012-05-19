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
//! @file query.c
//!

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <libwaei/libwaei.h>


LwQuery* 
lw_query_new ()
{
    LwQuery *temp;

    temp = g_new0 (LwQuery, 1);

    return temp;
}


void 
lw_query_free (LwQuery* query)
{
    g_return_if_fail (query != NULL);

    lw_query_clear (query);
    if (query->text != NULL) g_free (query->text); query->text = NULL;

    g_free (query);
}


static void 
lw_query_clear_tokens (LwQuery *query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    //Declarations
    gint i;

    if (query->tokenlist != NULL)
    {
      for (i = 0; i < TOTAL_LW_QUERY_TOKEN_TYPES; i++)
      {
        if (query->tokenlist[i] != NULL)
        {
          g_list_foreach (query->tokenlist[i], (GFunc) g_free, NULL);
          query->tokenlist[i] = NULL; 
        }
      }
      g_free (query->tokenlist); query->tokenlist = NULL;
    }
}


void
lw_query_init_tokens (LwQuery *query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    lw_query_clear_tokens (query);
    query->tokenlist = g_new0 (GList*, TOTAL_LW_QUERY_TOKEN_TYPES);
}


static void 
lw_query_clear_regexgroup (LwQuery *query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    //Declarations
    gint i;

    if (query->regexgroup != NULL)
    {
      for (i = 0; i < TOTAL_LW_QUERY_REGEX_TYPES; i++)
      {
        if (query->regexgroup[i] != NULL)
        {
          lw_regexgroup_free (query->regexgroup[i]);
          query->regexgroup[i] = NULL; 
        }
      }
      g_free (query->regexgroup); query->regexgroup = NULL;
    }
}


void
lw_query_init_regexgroup (LwQuery *query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    lw_query_clear_regexgroup (query);
    query->regexgroup = g_new0 (LwRegexGroup*, TOTAL_LW_QUERY_REGEX_TYPES);
}


void 
lw_query_clear (LwQuery* query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    lw_query_clear_tokens (query);
    lw_query_clear_regexgroup (query);

    query->parsed = FALSE;
}


const gchar* 
lw_query_get_text (LwQuery *query)
{
    return query->text;
}


gboolean 
lw_query_is_parsed (LwQuery *query)
{
    return (query->parsed);
}

/*
gboolean 
lw_query_is_sane (const char* query)
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
*/

