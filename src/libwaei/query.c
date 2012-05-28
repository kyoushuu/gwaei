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
        if (query->tokenlist[i] != NULL) g_free (query->tokenlist[i]); 
        query->tokenlist[i] = NULL;
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
lw_query_clear_rangelist (LwQuery *query)
{
    gint i;

    if (query->rangelist != NULL) 
    {
      for (i = 0; i < TOTAL_LW_QUERY_RANGE_TYPES; i++)
      {
        lw_range_free (query->rangelist[i]); query->rangelist[i] = NULL;
      }
      g_free (query->rangelist); query->rangelist = NULL;
    }
}


void
lw_query_init_rangelist (LwQuery *query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    lw_query_clear_rangelist (query);
    query->rangelist = g_new0 (LwRange*, TOTAL_LW_QUERY_RANGE_TYPES);
}


void 
lw_query_clear (LwQuery* query)
{
    //Sanity check
    g_return_if_fail (query != NULL);

    lw_query_clear_tokens (query);
    lw_query_clear_regexgroup (query);
    lw_query_clear_rangelist (query);

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
lw_query_tokenlist_append (LwQuery *query, LwQueryType type, LwRelevance relevance, gboolean primary, const gchar *token)
{
    //Sanity checks
    g_return_if_fail (query != NULL);
    //g_return_if_fail (type > -1 && type < TOTAL_LW_QUERY_TYPES);
    ///g_return_if_fail (relevance > -1 && relevance < TOTAL_LW_RELEVANCE);
    g_return_if_fail (token != NULL);

    //Declarations
    gchar number_character;
    gchar primary_character;
    gchar *combined;
    static const gchar DELIMITOR = '|';

    //Initializations
    number_character = '0' + (gchar) relevance;
    primary_character = '0' + (gchar) primary;

    if (query->tokenlist[type] == NULL)
    {
      combined = g_strdup_printf ("%c%c%s", number_character, primary_character, token);
      if (combined == NULL) return;
      query->tokenlist[type] = combined;
    } 
    else
    {
      combined = g_strdup_printf ("%s%c%c%c%s", query->tokenlist[type], DELIMITOR, number_character, primary_character, token);
      if (combined == NULL) return;
      g_free (query->tokenlist[type]); query->tokenlist[type] = combined;
    }
    printf("BREAK updated tokenlist %s\n", query->tokenlist[type]);
}


gchar*
lw_query_get_tokenlist (LwQuery *query, LwQueryType type, LwRelevance relevance_filter, gboolean only_primary)
{
    gchar *buffer, *bufferptr;
    gchar **tokeniter, **tokenlist, *tokenptr;
    const static gchar *DELIMITOR = "|";
    LwRelevance relevance;
    gboolean primary;

    if (query->tokenlist[type] == NULL) return NULL;
    bufferptr = buffer = g_new (gchar, strlen (query->tokenlist[type]) + 1);
    if (buffer == NULL) return NULL;
    tokeniter = tokenlist = g_strsplit (query->tokenlist[type], DELIMITOR, -1);

    if (tokenlist != NULL)
    {
      while (*tokeniter != NULL)
      {
        tokenptr = *tokeniter;
        relevance = (gint) (*tokenptr - '0');
        tokenptr++;
        primary = (gint) (*tokenptr - '0');
        tokenptr++;
        if (relevance >= relevance_filter && (!only_primary || primary))
        {   
          if (bufferptr > buffer) *(bufferptr++) = '|';
          while (*tokenptr != '\0') *(bufferptr++) = *(tokenptr++);
        }
        tokeniter++;
      }
    }
    *bufferptr = '\0';
    
    g_strfreev (tokenlist); tokenlist = tokeniter = NULL; tokenptr = NULL;
    
    return buffer;
}


void
lw_query_rangelist_set (LwQuery *query, LwQueryRangeType type, LwRange *range)
{
    //Sanity checks
    g_return_if_fail (query != NULL);
    g_return_if_fail (range != NULL);
    g_return_if_fail (query->rangelist != NULL);

    if (query->rangelist[type] != NULL) lw_range_free (query->rangelist[type]);
    query->rangelist[type] = range;
}


LwRange*
lw_query_rangelist_get (LwQuery *query, LwQueryRangeType type)
{
    //Sanity checks
    g_return_val_if_fail (query != NULL, NULL);
    g_return_val_if_fail (query->rangelist != NULL, NULL);

    return query->rangelist[type];
}
