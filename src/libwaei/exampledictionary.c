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
//!  @file exampledictionary.c
//!
//!  @brief LwExampleDictionary objects represent a loaded dictionary that the program
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

G_DEFINE_TYPE (LwExampleDictionary, lw_exampledictionary, LW_TYPE_DICTIONARY)

static gboolean lw_exampledictionary_parse_query (LwDictionary*, LwQuery*, const gchar*, GError**);
static gint lw_exampledictionary_parse_result (LwDictionary*, LwResult*, FILE*);
static gboolean lw_exampledictionary_compare (LwDictionary*, LwQuery*, LwResult*, const LwRelevance);

static void lw_exampledictionary_tokenize_query (LwDictionary*, LwQuery*);
static void lw_exampledictionary_add_supplimental_tokens (LwDictionary*, LwQuery*);


LwDictionary* lw_exampledictionary_new (const gchar *FILENAME)
{
    g_return_val_if_fail (FILENAME != NULL, NULL);

    //Declarations
    LwDictionary *dictionary;

    //Initializations
    dictionary = LW_DICTIONARY (g_object_new (LW_TYPE_EXAMPLEDICTIONARY,
                                "filename", FILENAME,
                                NULL));

    return dictionary;
}


static void 
lw_exampledictionary_init (LwExampleDictionary *dictionary)
{
}


static void
lw_exampledictionary_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (lw_exampledictionary_parent_class)->constructed (object);
    }

    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;

    dictionary = LW_DICTIONARY (object);
    priv = dictionary->priv;

    if (strncmp(priv->filename, "Example", strlen("Example")) == 0)
    {
      if (priv->longname != NULL) g_free (priv->longname); priv->longname = NULL;
      if (priv->shortname != NULL) g_free (priv->shortname); priv->shortname = NULL;

      priv->longname = g_strdup (gettext("Example Dictionary"));
      priv->shortname = g_strdup (gettext("Example"));
    }
}


static void 
lw_exampledictionary_finalize (GObject *object)
{
    G_OBJECT_CLASS (lw_exampledictionary_parent_class)->finalize (object);
}


static void
lw_exampledictionary_class_init (LwExampleDictionaryClass *klass)
{
    //Declarations
    GObjectClass *object_class;
    LwDictionaryClass *dictionary_class;
    gint i;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = lw_exampledictionary_finalize;
    object_class->constructed = lw_exampledictionary_constructed;

    dictionary_class = LW_DICTIONARY_CLASS (klass);
    dictionary_class->parse_query = lw_exampledictionary_parse_query;
    dictionary_class->parse_result = lw_exampledictionary_parse_result;
    dictionary_class->compare = lw_exampledictionary_compare;

    dictionary_class->patterns = g_new0 (gchar**, TOTAL_LW_QUERY_TYPES + 1);
    for (i = 0; i < TOTAL_LW_QUERY_TYPES; i++)
    {
      dictionary_class->patterns[i] = g_new0 (gchar*, TOTAL_LW_RELEVANCE + 1);
    }

    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_MEDIUM] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_KANJI][LW_RELEVANCE_HIGH] = "(%s)";

    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_MEDIUM] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_FURIGANA][LW_RELEVANCE_HIGH] = "(%s)";

    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_LOW] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_MEDIUM] = "(%s)";
    dictionary_class->patterns[LW_QUERY_TYPE_ROMAJI][LW_RELEVANCE_HIGH] =  "\\b(%s)\\b";
}


static gboolean 
lw_exampledictionary_parse_query (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT, GError **error)
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
 
    lw_exampledictionary_tokenize_query (dictionary, query);
    lw_exampledictionary_add_supplimental_tokens (dictionary, query);
    lw_dictionary_build_regex (dictionary, query, error);

    return (error == NULL || *error == NULL);

}


static gboolean
lw_exampledictionary_is_a (gchar *text)
{
    return (*text == 'A' && *(text + 1) == ':');
}


/*
static gboolean
lw_exampledictionary_is_b (gchar *text)
{
    return (*text == 'B' && *(text + 1) == ':');
}
*/


//!
//! @brief, Retrieve a line from FILE, parse it according to the LwExampleDictionary rules and put the results into the LwResult
//!
static gint
lw_exampledictionary_parse_result (LwDictionary *dictionary, LwResult *result, FILE *fd)
{
TODO
    //Declarations
    gchar *ptr;
    gint bytes_read;
    gint length;

    //Initializations
    length = bytes_read = 0;

    //Read the next line
    do {
      ptr = fgets(result->text, LW_IO_MAX_FGETS_LINE, fd);
      if (ptr != NULL)
      {
        length = strlen(result->text);
        bytes_read += length;
      }
    } while (ptr != NULL && *ptr == '#');
    if (ptr == NULL) return bytes_read;
    ptr += length;

    //Commented input in the dictionary...we should skip over it
    if (lw_exampledictionary_is_a (result->text) && (ptr = fgets(ptr, LW_IO_MAX_FGETS_LINE - length, fd)) != NULL)
    {
      ptr--;
      if (*ptr == '\n') *ptr = ':';
      ptr++;

      length = strlen(ptr);
      bytes_read += length;
      ptr += length - 1;

      if (*ptr == '\n') *ptr = '\0';
    }

    return bytes_read;
}


static gboolean 
lw_exampledictionary_compare (LwDictionary *dictionary, LwQuery *query, LwResult *result, const LwRelevance RELEVANCE)
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
    for (j = 0; result->def_start[j] != NULL; j++)
    {
      found = g_regex_match (regex, result->def_start[j], 0, NULL);
      if (found == TRUE) {
        return TRUE;
      }
    }

    return FALSE;
}


static void
lw_exampledictionary_tokenize_query (LwDictionary *dictionary, LwQuery *query)
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
      temp = lw_util_delimit_script_changes (DELIMITOR, delimited, FALSE);
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
          lw_query_tokenlist_append (query, LW_QUERY_TYPE_FURIGANA, LW_RELEVANCE_HIGH, TRUE, tokens[i]);
        else if (lw_util_is_kanji_ish_str (tokens[i]))
          lw_query_tokenlist_append (query, LW_QUERY_TYPE_KANJI, LW_RELEVANCE_HIGH, TRUE, tokens[i]);
        else if (lw_util_is_romaji_str (tokens[i]))
          lw_query_tokenlist_append (query, LW_QUERY_TYPE_ROMAJI, LW_RELEVANCE_HIGH, TRUE, tokens[i]);
        else
          g_free (tokens[i]); 
        tokens[i] = NULL;
      }
      g_free (tokens); tokens = NULL;
    }
}


static void 
lw_exampledictionary_add_supplimental_tokens (LwDictionary *dictionary, LwQuery *query)
{
/*
          if (get_japanese_morphology)
          {
            lw_morphology_get_stem ()
            query->tokenlist[LW_QUERY_TYPE_KANJI] = g_list_append (query->tokenlist[LW_QUERY_TYPE_KANJI], tokens[i]);
          }
*/

}

