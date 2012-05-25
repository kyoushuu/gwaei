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
      i = 0;
      while (i < TOTAL_LW_QUERY_TYPES)
      {
        if (query->tokenlist[i] != NULL) g_free (query->tokenlist[i]); query->tokenlist = NULL;
        i++;
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
    query->tokenlist = g_new0 (gchar*, TOTAL_LW_QUERY_TYPES);
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
      for (i = 0; i < TOTAL_LW_QUERY_TYPES; i++)
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
    query->regexgroup = g_new0 (GRegex**, TOTAL_LW_QUERY_TYPES);
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


void
lw_query_tokenlist_append (LwQuery *query, LwQueryType type, LwRelevance relevance, const gchar *token)
{
    //Sanity checks
    g_return_if_fail (query != NULL);
printf("BREAK type %s %d %d\n", token, type, TOTAL_LW_QUERY_TYPES);
    //g_return_if_fail (type > -1 && type < TOTAL_LW_QUERY_TYPES);
printf("BREAK type %s %d %d\n", token, type, TOTAL_LW_QUERY_TYPES);
    ///g_return_if_fail (relevance > -1 && relevance < TOTAL_LW_RELEVANCE);
    g_return_if_fail (token != NULL);

    //Declarations
    gchar number;
    gchar *combined;
    static const gchar DELIMITOR = '|';
printf("BREAK2\n");
    //Initializations
    if (relevance < 10)
      number = '0' + (gchar) relevance;
    else
      number = '0'; 

    if (query->tokenlist[type] == NULL)
    {
      combined = g_strdup_printf ("%c%s", number, token);
      if (combined == NULL) return;
      query->tokenlist[type] = combined;
    } 
    else
    {
      combined = g_strdup_printf ("%s%c%c%s", query->tokenlist[type], DELIMITOR, number, token);
      if (combined == NULL) return;
      g_free (query->tokenlist[type]); query->tokenlist[type] = combined;
    }
    printf("BREAK updated tokenlist %s\n", query->tokenlist[type]);
}


gchar*
lw_query_get_tokenlist (LwQuery *query, LwQueryType type, LwRelevance relevance)
{
    if (type == LW_QUERY_TYPE_ROMAJI)
      return g_strdup ("fish");
    return NULL;
}

