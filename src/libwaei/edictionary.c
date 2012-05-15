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
//!  @file edictionary.c
//!
//!  @brief LwEDictionary objects represent a loaded dictionary that the program
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

G_DEFINE_TYPE (LwEDictionary, lw_edictionary, LW_TYPE_DICTIONARY)

static gchar* FIRST_DEFINITION_PREFIX_STR = "(1)";
static gboolean lw_edictionary_parse_query (LwDictionary*, LwQuery*, const gchar*, GError**);
static gboolean lw_edictionary_parse_result (LwDictionary*, LwResult*, FILE*);
static gboolean lw_edictionary_compare (LwDictionary*, LwQuery*, LwResult*, const LwRelevance);
static gboolean lw_edictionary_installer_postprocess (LwDictionary*, gchar**, gchar**, LwIoProgressCallback, gpointer, GError**);

const static gchar *kanji_high = "^(無|不|非|お|御|)(%s)";
const static gchar *kanji_medium = "^(お|を|に|で|は|と|)(%s)(で|が|の|を|に|で|は|と|$)";
const static gchar *kanji_low = "(%s)";

const static gchar *furigana_high = "^(お|)(%s)$";
const static gchar *furigana_medium = "(^お|を|に|で|は|と)(%s)(で|が|の|を|に|で|は|と|$)";
const static gchar *furigana_low = "(%s)";

const static gchar *romaji_high = "(^|\\)|/|^to |\\) )(%s)(\\(|/|$|!| \\()";
const static gchar *romaji_medium = "(\\) |/)((\\bto )|(\\bto be )|(\\b))(%s)(( \\([^/]+\\)/)|(/))";
const static gchar *romaji_low = "(%s)";


LwDictionary* lw_edictionary_new (const gchar *FILENAME)
{
    g_return_val_if_fail (FILENAME != NULL, NULL);

    //Declarations
    LwDictionary *dictionary;

    //Initializations
    dictionary = LW_DICTIONARY (g_object_new (LW_TYPE_EDICTIONARY,
                                "dictionary-filename", FILENAME,
                                NULL));

    return dictionary;
}


static void 
lw_edictionary_init (LwEDictionary *dictionary)
{
}


static void
lw_edictionary_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (lw_edictionary_parent_class)->constructed (object);
    }

    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;

    dictionary = LW_DICTIONARY (object);
    priv = dictionary->priv;

    if (strncmp(priv->filename, "English", strlen("English")) == 0)
    {
      if (priv->longname != NULL) g_free (priv->longname); priv->longname = NULL;
      if (priv->shortname != NULL) g_free (priv->shortname); priv->shortname = NULL;

      priv->longname = g_strdup (gettext("English Dictionary"));
      priv->shortname = g_strdup (gettext("English"));
    }
}


static void 
lw_edictionary_finalize (GObject *object)
{
    G_OBJECT_CLASS (lw_edictionary_parent_class)->finalize (object);
}


static void
lw_edictionary_class_init (LwEDictionaryClass *klass)
{
    //Declarations
    GObjectClass *object_class;
    LwDictionaryClass *dictionary_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = lw_edictionary_finalize;
    object_class->constructed = lw_edictionary_constructed;

    dictionary_class = LW_DICTIONARY_CLASS (klass);
    dictionary_class->parse_query = lw_edictionary_parse_query;
    dictionary_class->parse_result = lw_edictionary_parse_result;
    dictionary_class->compare = lw_edictionary_compare;
    dictionary_class->installer_postprocess = lw_edictionary_installer_postprocess;
}


static void
lw_edictionary_tokenize_query (LwDictionary *dictionary, LwQuery *query)
{
    //Declarations
    gchar *temp;
    gchar *delimited;
    gchar **tokens;
    const gchar *delimitor;
    gboolean split_script_changes, split_whitespace;
    gint i;
    
    //Initializations
    delimitor = "|";
    delimited = lw_util_prepare_query (lw_query_get_text (query), TRUE);
    split_script_changes = split_whitespace = TRUE;

    if (split_script_changes)
    {
      temp = lw_util_delimit_script_changes (delimitor, delimited);
      g_free (delimited); delimited = temp; temp = NULL;
    }

    if (split_whitespace)
    {
      temp = lw_util_delimit_whitespace (delimitor, delimited);
      g_free (delimited); delimited = temp; temp = NULL;
    }

    tokens = g_strsplit (delimitor, delimited, -1);

    if (tokens != NULL)
    {
      for (i = 0; tokens[i] != NULL; i++)
      {
        if (lw_util_is_kanji_ish_str (tokens[i]))
        {
          query->tokenlist[LW_QUERY_TOKEN_TYPE_KANJI] = g_list_append (query->tokenlist[LW_QUERY_TOKEN_TYPE_KANJI], tokens[i]);
/*
          if (get_japanese_morphology)
          {
            lw_morphology_get_stem ()
            query->tokenlist[LW_QUERY_TOKEN_TYPE_KANJI] = g_list_append (query->tokenlist[LW_QUERY_TOKEN_TYPE_KANJI], tokens[i]);
          }
*/
        }
        else if (lw_util_is_furigana_str (tokens[i]))
        {
          query->tokenlist[LW_QUERY_TOKEN_TYPE_FURIGANA] = g_list_append (query->tokenlist[LW_QUERY_TOKEN_TYPE_FURIGANA], tokens[i]);
/*
          if (get_japanese_morphology)
          {
            lw_morphology_get_stem ()
            query->tokenlist[LW_QUERY_TOKEN_TYPE_KANJI] = g_list_append (query->tokenlist[LW_QUERY_TOKEN_TYPE_KANJI], tokens[i]);
          }
*/
        }
        else if (lw_util_is_romaji_str (tokens[i]))
        {
          query->tokenlist[LW_QUERY_TOKEN_TYPE_ROMAJI] = g_list_append (query->tokenlist[LW_QUERY_TOKEN_TYPE_ROMAJI], tokens[i]);
        }
        else
        {
          g_free (tokens[i]);
        }
      }
      g_free (tokens); tokens = NULL;
    }
}


static void
lw_edictionary_build_kanji_regex (LwDictionary *dictionary, LwQuery *query, GError **error)
{
    //Sanity checks
    g_return_if_fail (dictionary != NULL);
    g_return_if_fail (query != NULL);
    g_return_if_fail (query->regexgroup != NULL);
    g_return_if_fail (query->tokenlist != NULL);
    g_return_if_fail (error != NULL);
    if (error != NULL && *error != NULL) return;

    //Declarations
    GList* tokenlist;
    GList *link;
    gchar *text;
    GRegex *regex;
    LwRegexGroup *regexgroup;
    gboolean errored;

    //Initializations
    regexgroup = lw_regexgroup_new ();
    tokenlist = query->tokenlist[LW_QUERY_TOKEN_TYPE_KANJI];
    errored = FALSE;

    for (link = tokenlist; link != NULL && !errored; link = link->next)
    {
      text = (gchar*) link->data;

      if (text != NULL)
      {
        regex = lw_regex_new (kanji_high, text, error);
        if (regex != NULL) regexgroup->high = g_list_append (regexgroup->high, regex);

        regex = lw_regex_new (kanji_medium, text,error);
        if (regex != NULL) regexgroup->medium = g_list_append (regexgroup->medium, regex);

        regex = lw_regex_new (kanji_low, text, error);
        if (regex != NULL) regexgroup->low = g_list_append (regexgroup->low, regex);
      }

      if (error != NULL && *error != NULL) errored = TRUE;
    }

    query->regexgroup[LW_QUERY_REGEX_TYPE_KANJI] = regexgroup;
}


static void
lw_edictionary_build_furigana_regex (LwDictionary *dictionary, LwQuery *query, GError **error)
{
    //Sanity checks
    g_return_if_fail (dictionary != NULL);
    g_return_if_fail (query != NULL);
    g_return_if_fail (query->regexgroup != NULL);
    g_return_if_fail (query->tokenlist != NULL);
    g_return_if_fail (error != NULL);
    if (error != NULL && *error != NULL) return;

    //Declarations
    GList* tokenlist;
    GList *link;
    gchar *text;
    GRegex *regex;
    LwRegexGroup *regexgroup;
    gboolean errored;

    //Initializations
    regexgroup = lw_regexgroup_new ();
    tokenlist = query->tokenlist[LW_QUERY_TOKEN_TYPE_FURIGANA];

    for (link = tokenlist; link != NULL && !errored; link = link->next)
    {
      text = (gchar*) link->data;

      if (text != NULL)
      {
        regex = lw_regex_new (furigana_high, text, error);
        if (regex != NULL) regexgroup->high = g_list_append (regexgroup->high, regex);

        regex = lw_regex_new (furigana_medium, text, error);
        if (regex != NULL) regexgroup->medium = g_list_append (regexgroup->medium, regex);

        regex = lw_regex_new (furigana_low, text, error);
        if (regex != NULL) regexgroup->low = g_list_append (regexgroup->low, regex);

/*
        if (get_japanese_morphology)
        {
          regex = lw_regex_new (furigana_medium_prefix, text, furigana_medium_suffix, error);
          regexgroup->medium = g_list_append (regexgroup->low, regex);
        }
*/
      }

      if (error != NULL && *error != NULL) errored = TRUE;
    }

    query->regexgroup[LW_QUERY_REGEX_TYPE_FURIGANA] = regexgroup;
}


static void
lw_edictionary_build_romaji_regex (LwDictionary *dictionary, LwQuery *query, GError **error)
{
    //Sanity checks
    g_return_if_fail (dictionary != NULL);
    g_return_if_fail (query != NULL);
    g_return_if_fail (query->regexgroup != NULL);
    g_return_if_fail (query->tokenlist != NULL);
    g_return_if_fail (error != NULL);
    if (error != NULL && *error != NULL) return;

    //Declarations
    GList* tokenlist;
    GList *link;
    gchar *text;
    GRegex *regex;
    LwRegexGroup *regexgroup;
    gboolean errored;

    //Initializations
    regexgroup = lw_regexgroup_new ();
    tokenlist = query->tokenlist[LW_QUERY_TOKEN_TYPE_ROMAJI];

    for (link = tokenlist; link != NULL && !errored; link = link->next)
    {
      text = (gchar*) link->data;

      if (text != NULL)
      {
        regex = lw_regex_new (romaji_high, text, error);
        if (regex != NULL) regexgroup->high = g_list_append (regexgroup->high, regex);

        regex = lw_regex_new (romaji_medium, text, error);
        if (regex != NULL) regexgroup->medium = g_list_append (regexgroup->medium, regex);

        regex = lw_regex_new (romaji_low, text, error);
        if (regex != NULL) regexgroup->low = g_list_append (regexgroup->low, regex);
/*
        if (get_english_stemp)
        {
          romaji_stem
          regex = lw_regex_new (romaji_medium_prefix, text, romaji_medium_suffix, error);
          regexgroup->medium = g_list_append (regexgroup->low, regex);
        }
*/
      }

      if (error != NULL && *error != NULL) errored = TRUE;
    }

    query->regexgroup[LW_QUERY_REGEX_TYPE_ROMAJI] = regexgroup;
}


static gboolean 
lw_edictionary_parse_query (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT, GError **error)
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
 
    lw_edictionary_tokenize_query (dictionary, query);

    lw_edictionary_build_kanji_regex (dictionary, query, error);
    lw_edictionary_build_furigana_regex (dictionary, query, error);
    lw_edictionary_build_romaji_regex (dictionary, query, error);

    return (error == NULL || *error == NULL);
}


//!
//! @brief, Retrieve a line from FILE, parse it according to the LwEDictionary rules and put the results into the LwResult
//!
static gint 
lw_edictionary_parse_result (LwDictionary *dictionary, LwResult *result, FILE *fd)
{
    gchar *ptr = NULL;
    gchar *next = NULL;
    gchar *nextnext = NULL;
    gchar *nextnextnext = NULL;
    gchar *temp = NULL;
    gint bytes_read = 0;

    //Read the next line
    do {
      ptr = fgets(result->text, LW_IO_MAX_FGETS_LINE, fd);
      if (ptr != NULL) bytes_read += strlen(result->text);
    } while (ptr != NULL && *ptr == '#');

    if (ptr == NULL) return bytes_read;
    bytes_read += strlen(result->text);

    //Remove the final line break
    if ((temp = g_utf8_strchr (result->text, -1, '\n')) != NULL)
    {
        temp--;
        *temp = '\0';
    }

    //Set the kanji pointers
    result->kanji_start = ptr;
    ptr = g_utf8_strchr (ptr, -1, L' ');
    *ptr = '\0';

    //Set the furigana pointer
    ptr++;
    if (g_utf8_get_char(ptr) == L'[' && g_utf8_strchr (ptr, -1, L']') != NULL)
    {
      ptr = g_utf8_next_char(ptr);
      result->furigana_start = ptr;
      ptr = g_utf8_strchr (ptr, -1, L']');
      *ptr = '\0';
    }
    else
    {
      result->furigana_start = NULL;
      ptr--;
    }

    //Find if there is a type description classification
    temp = ptr;
    temp++;
    temp = g_utf8_strchr (temp, -1, L'/');
    if (temp != NULL && g_utf8_get_char(temp + 1) == L'(')
    {
      result->classification_start = temp + 2;
      temp = g_utf8_strchr (temp, -1, L')');
      *temp = '\0';
      ptr = temp;
    }

    //Set the definition pointers
    ptr++;
    ptr = g_utf8_next_char(ptr);
    result->def_start[0] = ptr;
    result->number[0] = FIRST_DEFINITION_PREFIX_STR;
    gint i = 1;

    temp = ptr;
    while ((temp = g_utf8_strchr(temp, -1, L'(')) != NULL && i < 50)
    {
      next = g_utf8_next_char (temp);
      nextnext = g_utf8_next_char (next);
      nextnextnext = g_utf8_next_char (nextnext);
      if (*next != '\0' && *nextnext != '\0' &&
          *next == L'1' && *nextnext == L')')
      {
         result->def_start[0] = result->def_start[0] + 4;
      }
      else if (*next != '\0' && *nextnext != '\0' && *nextnextnext != '\0' &&
               *next >= L'1' && *next <= L'9' && (*nextnext == L')' || *nextnextnext == L')'))
      {
         *(temp - 1) = '\0';
         result->number[i] = temp;
         temp = g_utf8_strchr (temp, -1, L')');
         *(temp + 1) = '\0';
         result->def_start[i] = temp + 2;
         i++;
      }
      temp = temp + 2;
    }
    result->def_total = i;
    result->def_start[i] = NULL;
    result->number[i] = NULL;
    i--;

    //Get the importance
    //temp = result->def_start[i] + strlen(result->def_start[i]) - 4;
    if ((temp = g_utf8_strrchr (result->def_start[i], -1, L'(')) != NULL)
    {
      result->important = (*temp == '(' && *(temp + 1) == 'P' && *(temp + 2) == ')');
      if (result->important) 
      {
        *(temp - 1) = '\0';
      }
    }

    return bytes_read;
}


gchar**
lw_edictionary_tolkenize_query (LwDictionary *dictionary, LwQuery *query)
{
/*
    const gchar *TEXT;
    gchar *output;
    GUnicodeScript script, next_script;
    const gchar *sourcptr;
    gchar *targetptr;
    gboolean is_tolken_break_point;
    gunichar c;

    TEXT = lw_query_get_text (query);
    output = g_new(gchar, strlen(TEXT) * sizeof(gchar) * 2 + 1);
    *output = '\0';
    sourceptr = TEXT;
    is_tolken_break_point = FALSE;
    script = next_script = G_UNICODE_SCRIPT_INVALID_CODE;

    if (does_not_have_regex_characters (query))
    {
      lw_dictionary_tolkenize_script_changes (dictionary, text);
      if (tolkenize_whitespace)
        lw_dictionary_tolkenize_whitespace (dictionary, text);
      if (tolkenize_japanese)
        lw_dictionary_tolkenize_japanese (dictionary, text);
    }

    japanese_normalization (verb forms etc)
    furigana_romaji_conversion
    english_normalization (verb forms etc)
*/

    
/* 
    if (output != NULL)
    {
      targetptr = output;
      while (*sourceptr != \'0')
      {
        c = g_utf8_get_char (sourceptr);
        next_script = g_unichar_get_script (c);
        script_changed = next_script != script;
        is_space = g_unichar_type (sourceptr) == G_UNICODE_SPACE_SEPARATOR;

        if (script_changed || is_space

        script = next_script;
        sourceptr = g_utf8_next_char (sourceptr);
      }
      
      g_free (output); output = NULL;
    }
*/
    return NULL;
}


static gboolean 
lw_edictionary_compare (LwDictionary *dictionary, LwQuery *query, LwResult *result, const LwRelevance RELEVANCE)
{
/*
    //Declarations
    int j;
    GRegex *re;
    GRegex ***iter;

    //Compare kanji atoms
    if (result->kanji_start != NULL)
    {
      for (iter = query->re_kanji; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, result->kanji_start, 0, NULL) == FALSE) break;
      }
      if (query->re_kanji[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;
    }

    //Compare furigana atoms
    if (result->furigana_start != NULL)
    {
      for (iter = query->re_furi; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, result->furigana_start, 0, NULL) == FALSE) break;
      }
      if (query->re_furi[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;
    }

    if (result->kanji_start != NULL)
    {
      for (iter = query->re_furi; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, result->kanji_start, 0, NULL) == FALSE) break;
      }
      if (query->re_furi[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;
    }


    //Compare romaji atoms
    for (j = 0; result->def_start[j] != NULL; j++)
    {
      for (iter = query->re_roma; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, result->def_start[j], 0, NULL) == FALSE) break;
      }
      if (query->re_roma[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;
    }

    //Compare mix atoms
    if (result->string != NULL)
    {
      for (iter = query->re_mix; *iter != NULL && **iter != NULL; iter++)
      {
        re = (*iter)[RELEVANCE];
        if (g_regex_match (re, result->string, 0, NULL) == FALSE) break;
      }
      if (query->re_roma[0][RELEVANCE] != NULL && *iter == NULL) return TRUE;
    }
*/
    return FALSE;
}


static gboolean
lw_edictionary_installer_postprocess (LwDictionary *dictionary, 
                                      gchar **sourcelist, 
                                      gchar **targetlist, 
                                      LwIoProgressCallback cb,
                                      gpointer data,
                                      GError **error)
{
    g_return_val_if_fail (dictionary != NULL, FALSE);
    g_return_val_if_fail (g_strv_length (sourcelist) < 1, FALSE);
    g_return_val_if_fail (g_strv_length (targetlist) < 2, FALSE);

    return lw_io_split_places_from_names_dictionary (targetlist[0], targetlist[1], sourcelist[0], cb, data, error);
}

