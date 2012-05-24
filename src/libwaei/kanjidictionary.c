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
//!  @file kanjidictionary.c
//!
//!  @brief LwKanjiDictionary objects represent a loaded dictionary that the program
//!         can use to carry out searches.  You can uninstall dictionaries
//!         by using the object, but you cannot install them. LwDictInst
//!         objects exist for that purpose.
//!

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <libwaei/gettext.h>
#include <libwaei/libwaei.h>
#include <libwaei/dictionary-private.h>

G_DEFINE_TYPE (LwKanjiDictionary, lw_kanjidictionary, LW_TYPE_DICTIONARY)

static gboolean lw_kanjidictionary_parse_query (LwDictionary*, LwQuery*, const gchar*, GError **);
static gint lw_kanjidictionary_parse_result (LwDictionary*, LwResult*, FILE*);
static gboolean lw_kanjidictionary_compare (LwDictionary *dictionary, LwQuery*, LwResult*, const LwRelevance);
static gboolean lw_kanjidictionary_installer_postprocess (LwDictionary*, gchar**, gchar**, LwIoProgressCallback, gpointer, GError**);

static void lw_kanjidictionary_tokenize_query (LwDictionary*, LwQuery*);


LwDictionary* lw_kanjidictionary_new (const gchar *FILENAME)
{
    g_return_val_if_fail (FILENAME != NULL, NULL);

    //Declarations
    LwDictionary *dictionary;

    //Initializations
    dictionary = LW_DICTIONARY (g_object_new (LW_TYPE_KANJIDICTIONARY,
                                "filename", FILENAME,
                                NULL));

    return dictionary;
}


static void 
lw_kanjidictionary_init (LwKanjiDictionary *dictionary)
{
}


static void
lw_kanjidictionary_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (lw_kanjidictionary_parent_class)->constructed (object);
    }

    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;

    dictionary = LW_DICTIONARY (object);
    priv = dictionary->priv;

    if (strncmp(priv->filename, "Kanji", strlen("Kanji")) == 0)
    {
      if (priv->longname != NULL) g_free (priv->longname); priv->longname = NULL;
      if (priv->shortname != NULL) g_free (priv->shortname); priv->shortname = NULL;

      priv->longname = g_strdup (gettext("Kanji Dictionary"));
      priv->shortname = g_strdup (gettext("Kanji"));
    }
}


static void 
lw_kanjidictionary_finalize (GObject *object)
{
    G_OBJECT_CLASS (lw_kanjidictionary_parent_class)->finalize (object);
}


static void
lw_kanjidictionary_class_init (LwKanjiDictionaryClass *klass)
{
    //Declarations
    GObjectClass *object_class;
    LwDictionaryClass *dictionary_class;
    gint i;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = lw_kanjidictionary_finalize;
    object_class->constructed = lw_kanjidictionary_constructed;

    dictionary_class = LW_DICTIONARY_CLASS (klass);
    dictionary_class->parse_query = lw_kanjidictionary_parse_query;
    dictionary_class->parse_result = lw_kanjidictionary_parse_result;
    dictionary_class->compare = lw_kanjidictionary_compare;
    dictionary_class->installer_postprocess = lw_kanjidictionary_installer_postprocess;

    dictionary_class->patterns = g_new0 (gchar**, TOTAL_LW_QUERY_TYPES + 1);
    for (i = 0; i < TOTAL_LW_QUERY_TYPES; i++)
    {
      dictionary_class->patterns[i] = g_new0 (gchar*, TOTAL_LW_RELEVANCE + 1);
    }

    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_MEDIUM] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_HIGH] = "^(%s)$";

    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_LOW] = "(^|\\s)(%s)(\\s|$)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_MEDIUM] = "(^|\\s)(%s)(\\s|$)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_HIGH] = "(^|\\s)(%s)(\\s|$)";

    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_MEDIUM] = "\\b(%s)\\b";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_HIGH] =  "\\{(%s)\\}";
}


static gboolean 
lw_kanjidictionary_parse_query (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT, GError **error)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, FALSE);
    g_return_val_if_fail (query != NULL, FALSE);
    g_return_val_if_fail (query->regexgroup != NULL, FALSE);
    g_return_val_if_fail (query->tokenlist != NULL, FALSE);
    g_return_val_if_fail (TEXT != NULL, FALSE);
    g_return_val_if_fail (error != NULL, FALSE);
    if (error != NULL && *error != NULL) return FALSE;

    //Sanity check
    g_return_val_if_fail (dictionary != NULL && query != NULL && TEXT != NULL, FALSE);
 
    lw_kanjidictionary_tokenize_query (dictionary, query);

/*
    lw_edictionary_build_kanji_regex (dictionary, query, error);
    lw_edictionary_build_furigana_regex (dictionary, query, error);
    lw_edictionary_build_romaji_regex (dictionary, query, error);
*/

    return (error == NULL || *error == NULL);
}


//!
//! @brief, Retrieve a line from FILE, parse it according to the LwKanjiDictionary rules and put the results into the LwResult
//!
static gint
lw_kanjidictionary_parse_result (LwDictionary *dictionary, LwResult *result, FILE *fd)
{
    return TRUE;
}



static gboolean 
lw_kanjidictionary_compare (LwDictionary *dictionary, LwQuery *query, LwResult *result, const LwRelevance RELEVANCE)
{
/*
    //Declarations
    gboolean strokes_check_passed;
    gboolean frequency_check_passed;
    gboolean grade_check_passed;
    gboolean jlpt_check_passed;
    gboolean romaji_check_passed;
    gboolean furigana_check_passed;
    gboolean kanji_check_passed;
    gboolean radical_check_passed;
    int kanji_index;
    int radical_index;

    GRegex ***iter;
    GRegex *re;

    int i;

    //Initializations
    strokes_check_passed = TRUE;
    frequency_check_passed = TRUE;
    grade_check_passed = TRUE;
    jlpt_check_passed = TRUE;
    romaji_check_passed = TRUE;
    furigana_check_passed = TRUE;
    kanji_check_passed = TRUE;
    radical_check_passed = TRUE;
    kanji_index = -1;
    radical_index = -1;

    //Calculate the strokes check
    if (result->strokes != NULL)
    {
      for (iter = query->re_strokes; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, result->strokes, 0, NULL) == FALSE) 
          strokes_check_passed = FALSE;
      }
    }
    else
      if (query->re_strokes != NULL && *(query->re_strokes) != NULL)
        strokes_check_passed = FALSE;

    //Calculate the frequency check
    if (result->frequency != NULL)
    {
      for (iter = query->re_frequency; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, result->frequency, 0, NULL) == FALSE) 
          frequency_check_passed = FALSE;
      }
    }
    else
      if (query->re_frequency != NULL && *(query->re_frequency) != NULL)
        frequency_check_passed = FALSE;


    //Calculate the grade check
    if (result->grade != NULL)
    {
      for (iter = query->re_grade; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, result->grade, 0, NULL) == FALSE) 
          grade_check_passed = FALSE;
      }
    }
    else
      if (query->re_grade != NULL && *(query->re_grade) != NULL)
        grade_check_passed = FALSE;


    //Calculate the jlpt check
    if (result->jlpt != NULL)
    {
      for (iter = query->re_jlpt; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, result->jlpt, 0, NULL) == FALSE) 
          jlpt_check_passed = FALSE;
      }
    }
    else
      if (query->re_jlpt != NULL && *(query->re_jlpt) != NULL)
        jlpt_check_passed = FALSE;



    //Calculate the romaji check
    if (result->meanings != NULL)
    {
      for (iter = query->re_roma; query->re_roma[0] != NULL && iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL) romaji_check_passed = FALSE;
      }
      //if (query->re_roma[0] != NULL && query->re_roma[0][0] != NULL) romaji_check_passed = FALSE;

      for (iter = query->re_roma; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];

        if (re != NULL && g_regex_match (re, result->meanings, 0, NULL) == TRUE) 
        {
          romaji_check_passed = TRUE;
        }
      }
    }


    //Calculate the furigana check
    if (*(query->re_furi) != NULL && (result->readings[0] != NULL || result->readings[1] != NULL || result->readings[2] != NULL))
    {
       furigana_check_passed = FALSE;
    }
    for (i = 0; i < 3; i++)
    {
      if (result->readings[i] == NULL) continue;

      for (iter = query->re_furi; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, result->readings[i], 0, NULL) == TRUE) 
        {
          furigana_check_passed = TRUE;
        }
      }
    }

    //Calculate the kanji check
    if (*(query->re_kanji) != NULL && result->kanji != NULL)
    {
      kanji_index = 0;
      for (iter = query->re_kanji; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, result->kanji, 0, NULL) == FALSE) 
        {
          kanji_check_passed = FALSE;
          kanji_index = -1;
        }
        if (kanji_check_passed == TRUE) kanji_index++;
      }
    }

    //Calculate the radical check
    if (result->radicals != NULL)
    {
      radical_index = 0;
      for (iter = query->re_kanji; iter != NULL && *iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (re != NULL && g_regex_match (re, result->radicals, 0, NULL) == FALSE) 
        {
          if (radical_index != kanji_index) //Make sure the radical wasn't found as a kanji before setting false
             radical_check_passed = FALSE;
        }
        radical_index++;
      }
    }

    //Return our results
    return (strokes_check_passed &&
            frequency_check_passed &&
            grade_check_passed &&
            jlpt_check_passed &&
            romaji_check_passed &&
            furigana_check_passed &&
            (radical_check_passed || kanji_check_passed));
*/
    return FALSE;
}


static gboolean
lw_kanjidictionary_installer_postprocess (LwDictionary *dictionary, 
                                          gchar **sourcelist, 
                                          gchar **targetlist, 
                                          LwIoProgressCallback cb,
                                          gpointer data,
                                          GError **error)
{
    g_return_val_if_fail (dictionary != NULL, FALSE);
    g_return_val_if_fail (g_strv_length (sourcelist) < 2, FALSE);
    g_return_val_if_fail (g_strv_length (targetlist) < 1, FALSE);

    return lw_io_create_mix_dictionary (targetlist[0], sourcelist[0], sourcelist[1], cb, data, error);
}


static void
lw_kanjidictionary_tokenize_query (LwDictionary *dictionary, LwQuery *query)
{
    //Declarations
    gchar *temp;
    gchar *delimited;
    gchar **tokens;
    static const gchar *DELIMITOR = "|";
    gboolean split_script_changes, split_whitespace;
    gint i;
    
    //Initializations
    delimited = lw_util_prepare_query (lw_query_get_text (query), TRUE);
    split_script_changes = split_whitespace = TRUE;

    if (split_script_changes)
    {
      temp = lw_util_delimit_script_changes (DELIMITOR, delimited, TRUE);
      g_free (delimited); delimited = temp; temp = NULL;
    }

    if (split_whitespace)
    {
      temp = lw_util_delimit_whitespace (DELIMITOR, delimited);
      g_free (delimited); delimited = temp; temp = NULL;
    }

    tokens = g_strsplit (delimited, DELIMITOR, -1);

    if (tokens != NULL)
    {
      for (i = 0; tokens[i] != NULL; i++)
      {
        if (lw_util_is_furigana_str (tokens[i]))
        {
          printf("adding furigana token  %s\n", tokens[i]);
          query->tokenlist[LW_QUERY_TYPE_FURIGANA] = g_list_append (query->tokenlist[LW_QUERY_TYPE_FURIGANA], tokens[i]);
/*
          if (get_japanese_morphology)
          {
            lw_morphology_get_stem ()
            query->tokenlist[LW_QUERY_TYPE_KANJI] = g_list_append (query->tokenlist[LW_QUERY_TYPE_KANJI], tokens[i]);
          }
*/
        }
        else if (lw_util_is_kanji_ish_str (tokens[i]))
        {
          printf("adding kanjiish token  %s\n", tokens[i]);
          query->tokenlist[LW_QUERY_TYPE_KANJI] = g_list_append (query->tokenlist[LW_QUERY_TYPE_KANJI], tokens[i]);
/*
          if (get_japanese_morphology)
          {
            lw_morphology_get_stem ()
            query->tokenlist[LW_QUERY_TYPE_KANJI] = g_list_append (query->tokenlist[LW_QUERY_TYPE_KANJI], tokens[i]);
          }
*/
        }
        else if (lw_util_is_romaji_str (tokens[i]))
        {
          printf("adding romaji token  %s\n", tokens[i]);
          query->tokenlist[LW_QUERY_TYPE_ROMAJI] = g_list_append (query->tokenlist[LW_QUERY_TYPE_ROMAJI], tokens[i]);
        }
        else
        {
          g_free (tokens[i]);
        }
      }
      g_free (tokens); tokens = NULL;
    }
}
