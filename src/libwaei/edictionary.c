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
static gchar* lw_edictionary_get_uri (LwDictionary*);
static gboolean lw_edictionary_parse_query (LwDictionary*, LwQuery*, const gchar*);
static gboolean lw_edictionary_parse_result (LwDictionary*, LwResult*, FILE*);
static const gchar* lw_edictionary_get_typename (LwDictionary*);
static gboolean lw_edictionary_compare (LwDictionary*, LwQuery*, LwResult*, const LwRelevance);

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
      priv->load_position = 1;
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
    dictionary_class->get_uri = lw_edictionary_get_uri;
    dictionary_class->get_typename = lw_edictionary_get_typename;
    dictionary_class->compare = lw_edictionary_compare;
}


static gchar*
lw_edictionary_get_uri (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    LwDictionaryPrivate *priv;
    gchar *path;

    //Initializations
    priv = LW_DICTIONARY (dictionary)->priv;
    g_return_val_if_fail (priv->filename != NULL, NULL);
    path = lw_util_build_filename (LW_PATH_DICTIONARY_EDICT, priv->filename);
  
    return path;
}


static gboolean 
lw_edictionary_parse_query (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT)
{
   //Sanity check
   g_return_val_if_fail (dictionary != NULL && query != NULL && TEXT != NULL, FALSE);

   //Free previously used memory
   lw_query_clean (query);
/*

   //Declarations
   char **atoms;
   char **iter;
   char *atom;
   char *temp;
   char *expression;
   char *half;
#ifdef WITH_MECAB
   char *morpho_expression;
#endif
   char *expression_low;
   char buffer[300];
   int rk_conv_pref;
   gboolean want_rk_conv;
   gboolean want_hk_conv;
   gboolean want_kh_conv;
   gboolean all_regex_built;
   int length;
   GRegex ***re;
   int i;

   tokens = lw_edictionary_get_tokens (dictionary, text);

   if (tokens != NULL)
   {
     for (i = 0; tokens[i] != NULL; i++)
     {
        if (lw_util_is_romaji_str (tokens[i])
          lw_edictionary_append_romaji_regex (dictionary, query, tokens[i]);
        else if (lw_util_is_kanji_ish_str (tokens[i])
          lw_edictionary_append_kanji_regex (dictionary, query, tokens[i]);
        else if (lw_util_is_furigana_str (tokens[i])
          lw_edictionary_append_furigana_regex (dictionary, query, tokens[i]);
        else
          lw_edictionary_append_mix_regex (dictionary, query, tokens[i]);
     }
     g_strfreev (tokens); tokens = NULL;
   }
*/

/*
   //Memory initializations
   all_regex_built = TRUE;

   if (pm != NULL)
   {
     rk_conv_pref = lw_preferences_get_int_by_schema (pm, LW_SCHEMA_BASE, LW_KEY_ROMAN_KANA);
     want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !lw_util_is_japanese_locale()));
     want_hk_conv = lw_preferences_get_boolean_by_schema (pm, LW_SCHEMA_BASE, LW_KEY_HIRA_KATA);
     want_kh_conv = lw_preferences_get_boolean_by_schema (pm, LW_SCHEMA_BASE, LW_KEY_KATA_HIRA);
   }
   else
   {
     rk_conv_pref = 1;
     want_rk_conv = TRUE;
     want_hk_conv = TRUE;
     want_kh_conv = TRUE;
   }

   //Start analysis
   atoms = _query_initialize_pointers (ql, STRING);

#ifdef WITH_MECAB
   morpho_expression = _query_get_morphology_regexp(ql);
#endif

   length = g_strv_length (atoms);

   //Setup the expression to be used in the base of the regex for kanji-ish strings
   re = ql->re_kanji;
   for (iter = atoms; *iter != NULL && re < (ql->re_kanji + length); iter++)
   {
     atom = *iter;
     expression = NULL;
     expression_low = NULL;

     if (lw_util_is_kanji_ish_str (atom) || lw_util_is_kanji_str (atom)) //Figures out if the string may contain hiragana
     {
       expression = g_strdup_printf ("(%s)", atom);

       if (lw_util_is_yojijukugo_str (atom))  //Check for yojijukugo
       {
          //First half of the yojijukugo
          half = g_strndup (atom, g_utf8_next_char(g_utf8_next_char(atom)) - atom);
          if (strcmp(half, "..") != 0)
          {
            temp = g_strdup_printf ("%s|(%s)", expression, half);
            g_free (expression);
            expression = temp;
          }
          g_free (half);

          //Second half of the yojijukugo
          half = g_strdup (g_utf8_next_char(g_utf8_next_char(atom)));
          if (strcmp(half, "..") != 0)
          {
            temp = g_strdup_printf ("%s|(%s)", expression, half);
            g_free (expression);
            expression = temp;
          }
          g_free (half);
       }
     }

#ifdef WITH_MECAB
     if (morpho_expression && iter == atoms) {
         // Stuff morphology regexp to the first atom
         if (expression == NULL) {
             expression_low = g_strdup (morpho_expression);
         }
         else {
             expression_low = g_strdup_printf ("%s|%s", expression, morpho_expression);
         }
     }
#endif

     if (expression_low && expression == NULL) {
         expression = g_strdup ("----------------");
     }

     if (expression) {
       //Compile the regexes
       temp = expression;
       for (i = 0; i < LW_RELEVANCE_TOTAL; i++) {
         if (expression_low && i == LW_RELEVANCE_LOW)
             temp = expression_low;
         if (((*re)[i] = lw_regex_kanji_new (temp, LW_DICTTYPE_EDICT, i, error)) == NULL) all_regex_built = FALSE;
       }

       g_free (expression);
       if (expression_low)
           g_free (expression_low);
       re++;
     }
   }


   //Setup the expression to be used in the base of the regex for furigana strings
   re = ql->re_furi;
   for (iter = atoms; *iter != NULL && re < (ql->re_furi + length); iter++)
   {
     atom = *iter;
     expression = NULL;
     expression_low = NULL;

     if (lw_util_is_furigana_str (atom))
     {
       expression = g_strdup_printf ("(%s)", atom);

       if (want_hk_conv && lw_util_is_hiragana_str (atom))
       {
         temp = g_strdup (atom);
         lw_util_str_shift_hira_to_kata (temp);
         g_free (expression);
         expression = g_strdup_printf("(%s)|(%s)", atom, temp);
         g_free (temp);
       }
       else if (want_kh_conv && lw_util_is_katakana_str (atom))
       {
         temp = g_strdup (atom);
         lw_util_str_shift_kata_to_hira (temp);
         g_free (expression);
         expression = g_strdup_printf("(%s)|(%s)", atom, temp);
         g_free (temp);
       }
     }
     else if (lw_util_is_romaji_str (atom) && lw_util_str_roma_to_hira (atom, buffer, 300) && want_rk_conv)
     {
       expression = g_strdup_printf("(%s)", buffer);

       if (want_hk_conv)
       {
         temp = g_strdup (buffer);
         lw_util_str_shift_hira_to_kata (temp);
         g_free (expression);
         expression = g_strdup_printf("(%s)|(%s)", buffer, temp);
         g_free (temp);
       }
     }

#ifdef WITH_MECAB
     if (morpho_expression && iter == atoms) {
         // Stuff morphology regexp to the first atom
         if (expression == NULL) {
             expression_low = g_strdup(morpho_expression);
         }
         else {
             expression_low = g_strdup_printf("%s|%s", expression, morpho_expression);
         }
     }
#endif

     if (expression_low && expression == NULL) {
         expression = g_strdup("----------------");
     }

     if (expression) {
       //Compile the regexes
       temp = expression;
       for (i = 0; i < LW_RELEVANCE_TOTAL; i++) {
         if (expression_low && i == LW_RELEVANCE_LOW)
             temp = expression_low;
         if (((*re)[i] = lw_regex_furi_new (temp, LW_DICTTYPE_EDICT, i, error)) == NULL) all_regex_built = FALSE;
       }

       g_free (expression);
       if (expression_low)
           g_free (expression_low);
       re++;
     }
   }


   //Setup the expression to be used in the base of the regex
   re = ql->re_roma;
   for (iter = atoms; *iter != NULL && re < (ql->re_roma + length); iter++)
   {
     atom = *iter;
     g_strstrip(atom);
     if (strlen(atom) > 0 && lw_util_is_romaji_str (atom) && g_regex_match (lw_re[LW_RE_NUMBER], atom, 0, NULL) == FALSE)
     {
       expression = g_strdup (atom);

       //Compile the regexes
       for (i = 0; i < LW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = lw_regex_romaji_new (expression, LW_DICTTYPE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
   }  


   //Setup the expression to be used in the base of the regex
   re = ql->re_mix;
   for (iter = atoms; *iter != NULL && re < (ql->re_roma + length); iter++)
   {
     atom = *iter;
     if (!lw_util_is_kanji_ish_str (atom) &&
         !lw_util_is_kanji_str (atom)     && 
         !lw_util_is_furigana_str (atom)  &&
         !lw_util_is_romaji_str (atom)      )
     {
       expression = g_strdup (atom);

       //Compile the regexes
       for (i = 0; i < LW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = lw_regex_mix_new (expression, LW_DICTTYPE_EDICT, i, error)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
   }

   //Cleanup
   g_strfreev (atoms);
   atoms = NULL;

#ifdef WITH_MECAB
   if (morpho_expression)
       g_free(morpho_expression);
#endif
*/

  return TRUE;
}


//!
//! @brief, Retrieve a line from FILE, parse it according to the LwEDictionary rules and put the results into the LwResult
//!
static gboolean
lw_edictionary_parse_result (LwDictionary *dictionary, LwResult *result, FILE *fd)
{
    gchar *ptr = result->text;
    gchar *next = NULL;
    gchar *nextnext = NULL;
    gchar *nextnextnext = NULL;
    gchar *temp = NULL;

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

    return TRUE;
}


static const gchar*
lw_edictionary_get_typename (LwDictionary *dictionary)
{
    return "edict";
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

