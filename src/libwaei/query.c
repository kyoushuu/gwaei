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


LwRegexGroup* lw_regex_group_new (void);
GRegex* lw_regex_group_get_regex (LwRegexGroup *group, LwRelevance relevance);
void lw_regex_group_set_regex (LwRegexGroup *group, LwRelevance relevance, GRegex *regex);
void lw_regex_group_free (LwRegexGroup *group);


LwQuery* lw_query_new ()
{
    g_return_val_if_fail (TEXT != NULL, NULL);

    LwQuery *temp;

    temp = g_new0 (LwQuery, 1);

    return temp;
}


void lw_query_free (LwQuery* query)
{
    g_return_if_fail (query != NULL);

    lw_query_clean (query);
    if (query->text != NULL) g_free (query->text); query->text = NULL;
    g_free (query);
}


void lw_query_clean (LwQuery* query)
{
    g_return_if_fail (query != NULL);

    GList *iter;
    gint i;

    if (query->regexlist != NULL)
    {
      for (i = 0; i != TOTAL_LW_QUERY_REGEX; i++)
      {
        for (iter = query->regexlist[i]; iter != NULL; iter = iter->next)
        {
          g_list_free_full (query->regexlist[i], (GDestroyNotify) g_regex_unref); query->regexlist[i] = NULL;
        }
      }
      g_free (query->regexlist); query->regexlist = NULL;
    }

    query->parsed = FALSE;
}


const gchar* lw_query_get_text (LwQuery *query)
{
    return query->text;
}


GList* lw_query_get_regexlist (LwQuery *query, LwQueryRegexType type)
{
/*&
    gboolean parsable;

    if (lw_query_is_parsed (query) = FALSE) parsable = lw_query_parse (query);
    if (!parsable) return NULL;

    return query->regexlist[type];
*/
    return NULL;
}


gboolean lw_query_is_parsed (LwQuery *query)
{
    return (query->parsed);
}


/*
//This code should spit the code where there is a space,
//the script type changes (japanese to english, english to japanese)
//or mecab says to split for a node
static gchar**
lw_query_build_atoms (LwQuery *query)
{
    gchar** atoms;

    if (query->split_japanese_word_breaks)
    if (query->split_english_word_breaks)

    atoms = g_strsplit(text, " ", -1);

    return atoms;
}


static GList*
lw_query_build_kanji_regex_list (LwQuery *query)
{
    GString *text;

    gchar **atoms = lw_query_build_atoms (query);

    
    for (i = 0; atoms != NULL && atoms[i] != NULL; i++)
    {
      if (lw_util_is_kanji_ish_str (atoms[i]))
      {
        switch (query->type)
        {
          case LW_DICTTYPE_EDICT:
            query->regexlist[LW_QUERY_KANJI_REGEX] = lw_parser_edict_build_kanji_regexlist (atoms[i]);
            break;
          case LW_DICTTYPE_KANJI:
            query->regexlist[LW_QUERY_KANJI_REGEX] = lw_parser_kanjidict_build_kanji_regexlist (atoms[i]);
            break;
          case LW_DICTTYPE_NAMES:
            query->regexlist[LW_QUERY_KANJI_REGEX] = lw_parser_namesdict_build_kanji_regexlist (atoms[i]);
            break;
          default;
            query->regexlist[LW_QUERY_KANJI_REGEX] = lw_parser_unknowndict_build_kanji_regexlist (atoms[i]);
            break;
        }
      }
      
      if (query->create_japanese_stem)
      {
      }
    }
}


static GList*
lw_query_build_furigana_regex_list (LwQuery *query)
{
    GString *text;

    gchar **atoms = lw_query_build_atoms (query);

    for (i = 0; atoms != NULL && atoms[i] != NULL; i++)
    {
      if (query->romaji_to_furigana && lw_util_is_romaji_str (atoms[i]))
      {
      }
      else if (lw_util_is_furigana_str (atoms[i]))
      {
      }

      if (query->create_japanese_stem)
      {
      }
    }
}


static GList*
lw_query_build_romaji_regex_list (LwQuery *query)
{
    GString *text;

    gchar **atoms = lw_query_build_atoms (query);

    for (i = 0; atoms != NULL && atoms[i] != NULL; i++)
    {
      if (lw_util_is_romaji_str (atoms[i]))
      {
      }

      if (query->create_english_stem)
      {
      }
    }
}






















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


This is a sentence sakana 日本語が難しい
1) it should look for the original
2) it should look for each of the smallest tokens


//!
//! @brief Inserts into the string separators to denote tokens where the script changes (english/japanese)
//!
static GList* 
lw_query_tolkenize_script_changes (LwQuery *query, const gchar *separator)
{
    GList *iter;
    GList *new_list;

    query->tokens = new_list;
}


//!
//! @brief Inserts into the string separators to denote tokens where whitespace occurs
//!
static GList* 
lw_query_tolkenize_whitespace (LwDictionary *dictionary, gchar *text const gchar *separator)
{
}


//!
//! @brief uses morphology to guess where tokens should be delimited
//!
static GList*
lw_query_tolkenize_japanese_morphology (LwDictionary *dictionary, gchar *text, const gchar *separator)
{
}

