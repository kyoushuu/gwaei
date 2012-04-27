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

static void lw_query_clear_tokens (LwQuery *);

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

    lw_query_clean (query);
    if (query->text != NULL) g_free (query->text); query->text = NULL;

    g_free (query);
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
lw_query_clean (LwQuery* query)
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
#ifdef WITH_MECAB
//!
//! @brief Construct a regexp for getting the morphologically deduced base forms of words
//!
//! @param ql Queryline whose ql->string has been initialized
//! @returns Newly allocated string, or NULL if nothing to add
//!
//! The regexp is of form ^WORD1$|^WORD2$|...
//!
static GRegex*
lw_query_build_morphology_regexp (gchar* text)
{
   LwMorphology *morph;
   GList *it;
   gchar *result;
   const char *ptr;

   if (ql->morphology)
   {
     g_free (ql->morphology);
     ql->morphology = NULL;
   }

   result = NULL;

   // Do analysis only on alpha-kana-kanji strings
   for (ptr = ql->string; *ptr != '\0'; ptr = g_utf8_next_char (ptr))
   {
       gunichar character;
       GUnicodeScript script;
       character = g_utf8_get_char (ptr);
       script = g_unichar_get_script (character);
       if (script != G_UNICODE_SCRIPT_HAN &&
           script != G_UNICODE_SCRIPT_HIRAGANA &&
           script != G_UNICODE_SCRIPT_KATAKANA &&
           !g_unichar_isalnum(character) &&
           !g_unichar_isspace(character))
           return result;
   }

   morph = lw_morphology_new ();
   lw_morphology_analize (lw_morphologyengine_get_default (), morph, ql->string);

   for (it = morph->items; it; it = it->next) {
       LwMorphologyItem *item = (LwMorphologyItem *)it->data;
       char *temp;

       if (it == morph->items && it->next == NULL
               && strcmp(item->base_form, ql->string) == 0) {
           // Don't add any results for a single word in base form
           break;
       }

       if (item->base_form) {
           if (result == NULL) {
               result = g_strdup_printf("^%s$", item->base_form);
           }
           else {
               temp = g_strdup_printf ("%s|^%s$", result, item->base_form);
               g_free (result);
               result = temp;
           }
       }
       if (item->explanation) {
           if (ql->morphology == NULL) {
               ql->morphology = g_strdup (item->explanation);
           }
           else {
               temp = g_strdup_printf ("%s + %s", ql->morphology, item->explanation);
               g_free (ql->morphology);
               ql->morphology = temp;
           }
       }
   }

   lw_morphology_free (morph);

   return result;
}
#endif


static GRegex*** _compile_and_allocate_number_search_regex (const char* subject, const LwRegexDataIndex INDEX, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return NULL;

    //Declarations
    GRegex ***re;
    GRegex ***iter;
    GMatchInfo *match_info;
    gboolean all_regex_built;
    char *match_text;
    char *expression;
    int i;

    //Initializations
    all_regex_built = TRUE;

    //Search the query text for the specfic search terms
    g_regex_match (lw_re[INDEX], subject, 0, &match_info);

    //Allocate the memory depending on how many were found
    re = _query_allocate_pointers (g_match_info_get_match_count (match_info));

    //Investigate the matches and compile the new regexes
    for (iter = re; all_regex_built && iter - re < LW_QUERYLINE_MAX_ATOMS && g_match_info_matches (match_info); iter++)
    {
      match_text = g_match_info_fetch (match_info, 0);
      expression = g_strdup_printf("\\b%s\\b", match_text + 1);

      for (i = 0; all_regex_built && i < LW_RELEVANCE_TOTAL; i++)
        if (((*iter)[i] = lw_regex_new (expression, LW_DICTTYPE_KANJI, i, error)) == NULL) all_regex_built = FALSE;

      g_free (expression);
      g_free (match_text);

      g_match_info_next (match_info, NULL);
    }
    g_match_info_free (match_info);
    (*iter) = NULL;

    //Cleanup if there was an error
    for (iter = re; !all_regex_built && iter - re < LW_QUERYLINE_MAX_ATOMS; iter++)
    {
      for (i = 0; i < LW_RELEVANCE_TOTAL; i++)
      {
        if ((*iter)[i] != NULL) g_regex_unref ((*re)[i]);
        (*iter)[i] = NULL;
      }
    }

    //Finish
    return re;
}



*/

