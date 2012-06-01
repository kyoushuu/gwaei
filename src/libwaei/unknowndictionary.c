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
//!  @file unknowndictionary.c
//!
//!  @brief LwDictionary objects represent a loaded dictionary that the program
//!         can use to carry out searches.  You can uninstall dictionaries
//!         by using the object, but you cannot install them. LwDictInst
//!         objects exist for that purpose.
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <libwaei/libwaei.h>
#include <libwaei/dictionary-private.h>

static gboolean lw_unknowndictionary_parse_query (LwDictionary*, LwQuery*, const gchar*, GError**);
static gint lw_unknowndictionary_parse_result (LwDictionary*, LwResult*, FILE*);
static gboolean lw_unknowndictionary_compare (LwDictionary*, LwQuery*, LwResult*, const LwRelevance);


G_DEFINE_TYPE (LwUnknownDictionary, lw_unknowndictionary, LW_TYPE_DICTIONARY)


LwDictionary* lw_unknowndictionary_new (const gchar *FILENAME)
{
    g_return_val_if_fail (FILENAME != NULL, NULL);

    //Declarations
    LwDictionary *dictionary;

    //Initializations
    dictionary = LW_DICTIONARY (g_object_new (LW_TYPE_UNKNOWNDICTIONARY,
                                "filename", FILENAME,
                                 NULL));

    return dictionary;
}


static void 
lw_unknowndictionary_init (LwUnknownDictionary *dictionary)
{
}


static void
lw_unknowndictionary_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (lw_unknowndictionary_parent_class)->constructed (object);
    }

/*
    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;

    dictionary = LW_DICTIONARY (object);
    priv = dictionary->priv;
*/
}


static void 
lw_unknowndictionary_finalize (GObject *object)
{
    G_OBJECT_CLASS (lw_unknowndictionary_parent_class)->finalize (object);
}


static void
lw_unknowndictionary_class_init (LwUnknownDictionaryClass *klass)
{
    //Declarations
    GObjectClass *object_class;
    LwDictionaryClass *dictionary_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = lw_unknowndictionary_finalize;
    object_class->constructed = lw_unknowndictionary_constructed;

    dictionary_class = LW_DICTIONARY_CLASS (klass);
    dictionary_class->parse_query = lw_unknowndictionary_parse_query;
    dictionary_class->parse_result = lw_unknowndictionary_parse_result;
    dictionary_class->compare = lw_unknowndictionary_compare;
    dictionary_class->installer_postprocess = lw_unknowndictionary_installer_postprocess;

    dictionary_class->patterns = g_new0 (gchar**, TOTAL_LW_QUERY_TYPES + 1);
    for (i = 0; i < TOTAL_LW_QUERY_TYPES; i++)
    {
      dictionary_class->patterns[i] = g_new0 (gchar*, TOTAL_LW_RELEVANCE + 1);
    }

    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_MEDIUM] = "(^|お|を|に|で|は|と|)(%s)(で|が|の|を|に|で|は|と|$)";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_HIGH] = "^(無|不|非|お|御|)(%s)$";

    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_MEDIUM] = "(^お|を|に|で|は|と)(%s)(で|が|の|を|に|で|は|と|$)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_HIGH] = "^(お|)(%s)$";

    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_MEDIUM] = "(\\) |/)((\\bto )|(\\bto be )|(\\b))(%s)(( \\([^/]+\\)/)|(/))";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_HIGH] = "(^|\\)|/|^to |\\) )(%s)(\\(|/|$|!| \\()";
}


//!
//! @brief Parses a string for an unknown format string
//! @param rl The Resultline object this method works on
//!
static gint
lw_unknowndictionary_parse_result (LwDictionary *dictionary, LwResult *result, FILE *file)
{
    return FALSE;
}


static gboolean
lw_unknowndictionary_parse_query (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT, GError **error)
{
    if (error != NULL && *error != NULL) return FALSE;
/*
    tokens = lw_unknowninfo_get_tokens (dictionary, text);

    if (tokens != NULL)
    {
      for (i = 0; tokens[i] != NULL; i++)
      {
        if (lw_util_is_romaji_str (tokens[i])
          lw_unknowninfo_append_romaji_regex (dictionary, query, tokens[i]);
        else if (lw_util_is_kanji_ish_str (tokens[i])
          lw_unknowninfo_append_kanji_regex (dictionary, query, tokens[i]);
        else if (lw_util_is_furigana_str (tokens[i])
          lw_unknowninfo_append_furigana_regex (dictionary, query, tokens[i]);
        else
          lw_unknowninfo_append_mix_regex (dictionary, query, tokens[i]);
      }
      g_strfreev (tokens); tokens = NULL;
    }

*/
    return (error == NULL || *error == NULL);
}


static gboolean 
lw_unknowndictionary_compare (LwDictionary *dictionary, LwQuery *query, LwResult *result, const LwRelevance RELEVANCE)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, FALSE);
    g_return_val_if_fail (query != NULL, FALSE);
    g_return_val_if_fail (result != NULL, FALSE);

    //Declarations
    gint j;
    gboolean found;
    GRegex *regex;

    //Compare kanji atoms
    regex = lw_query_regexgroup_get (query, LW_QUERY_TYPE_KANJI, RELEVANCE);
    if (result->kanji_start != NULL && regex != NULL)
    {
      found = g_regex_match (regex, result->kanji_start, 0, NULL);
      if (found) return TRUE;
    }

    //Compare furigana atoms
    regex = lw_query_regexgroup_get (query, LW_QUERY_TYPE_FURIGANA, RELEVANCE);
    if (result->furigana_start != NULL && regex != NULL)
    {
      found = g_regex_match (regex, result->furigana_start, 0, NULL);
      if (found) return TRUE;
    }

    //Compare romaji atoms
    regex = lw_query_regexgroup_get (query, LW_QUERY_TYPE_ROMAJI, RELEVANCE);
    for (j = 0; result->def_start[j] != NULL && regex != NULL; j++)
    {
      found = g_regex_match (regex, result->def_start[j], 0, NULL);
      if (found == TRUE) {
        return TRUE;
      }
    }

    return FALSE;
}

