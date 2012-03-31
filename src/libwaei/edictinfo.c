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
//!  @file edictinfo.c
//!
//!  @brief LwDictInfo objects represent a loaded dictionary that the program
//!         can use to carry out searches.  You can uninstall dictionaries
//!         by using the object, but you cannot install them. LwDictInst
//!         objects exist for that purpose.
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <libwaei/libwaei.h>
#include <libwaei/dictinfo-private.h>

static gchar* FIRST_DEFINITION_PREFIX_STR = "(1)";
static gchar* lw_edictinfo_get_uri (LwDictInfo*);
static gboolean lw_edictinfo_parse_query (LwDictInfo*, LwQuery*, const gchar*);
static gboolean lw_edictinfo_parse_result (LwDictInfo*, LwResult*, FILE*);


G_DEFINE_TYPE (LwEdictInfo, lw_edictinfo, LW_TYPE_DICTINFO)


LwDictInfo* lw_edictinfo_new (const gchar *FILENAME)
{
    g_return_val_if_fail (FILENAME != NULL, NULL);

    //Declarations
    LwDictInfo *di;

    //Initializations
    di = LW_DICTINFO (g_object_new (LW_TYPE_EDICTINFO,
                                    "dictionary-filename", FILENAME,
                                    NULL));

    return di;
}


static void 
lw_edictinfo_init (LwEdictInfo *di)
{
}


static void
lw_edictinfo_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (lw_edictinfo_parent_class)->constructed (object);
    }

    LwDictInfo *di;
    LwDictInfoPrivate *priv;

    di = LW_DICTINFO (object);
    priv = di->priv;

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
lw_edictinfo_finalize (GObject *object)
{
    G_OBJECT_CLASS (lw_edictinfo_parent_class)->finalize (object);
}


static void
lw_edictinfo_class_init (LwEdictInfoClass *klass)
{
    //Declarations
    GObjectClass *object_class;
    LwDictInfoClass *dictinfo_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = lw_edictinfo_finalize;
    object_class->constructed = lw_edictinfo_constructed;

    dictinfo_class = LW_DICTINFO_CLASS (klass);
    dictinfo_class->parse_query = lw_edictinfo_parse_query;
    dictinfo_class->parse_result = lw_edictinfo_parse_result;
    dictinfo_class->get_uri = lw_edictinfo_get_uri;
}


static gchar*
lw_edictinfo_get_uri (LwDictInfo *di)
{
    //Sanity checks
    g_return_val_if_fail (di != NULL, NULL);
    g_return_val_if_fail (di->priv->filename != NULL, NULL);

    //Declarations
    LwDictInfoPrivate *priv;
    gchar *path;

    //Initializations
    priv = di->priv;
    path = lw_util_build_filename (LW_PATH_DICTIONARY_EDICT, priv->filename);
  
    return path;
}


static gboolean 
lw_edictinfo_parse_query (LwDictInfo *di, LwQuery *query, const gchar *TEXT)
{
   //Sanity check
   g_return_val_if_fail (di != NULL && query != NULL && TEXt != NULL, FALSE);

   //Free previously used memory
   lw_query_clean (query);

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


   tokens = lw_edictinfo_get_tokens (di, text);

   if (tokens != NULL)
   {
     for (i = 0; tokens[i] != NULL; i++)
     {
        if (lw_util_is_romaji_str (tokens[i])
          lw_edictinfo_append_romaji_regex (di, query, tokens[i]);
        else if (lw_util_is_kanji_ish_str (tokens[i])
          lw_edictinfo_append_kanji_regex (di, query, tokens[i]);
        else if (lw_util_is_furigana_str (tokens[i])
          lw_edictinfo_append_furigana_regex (di, query, tokens[i]);
        else
          lw_edictinfo_append_mix_regex (di, query, tokens[i]);
     }
     g_strfreev (tokens); tokens = NULL;
   }

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
//! @brief, Retrieve a line from FILE, parse it according to the LwDictInfo rules and put the results into the LwResult
//!
static gboolean
lw_edictinfo_parse_result (LwDictInfo *di, LwResult *result, FILE *fd)
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

