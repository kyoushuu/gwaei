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

static gchar* lw_unknowndictionary_get_uri (LwDictionary*);
static gboolean lw_unknowndictionary_parse_query (LwDictionary*, LwQuery*, const gchar*);
static gboolean lw_unknowndictionary_parse_result (LwDictionary*, LwResult*, FILE*);
static const gchar* lw_unknowndictionary_get_typename (LwDictionary*);


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
    dictionary_class->get_uri = lw_unknowndictionary_get_uri;
    dictionary_class->get_typename = lw_unknowndictionary_get_typename;
}


static gchar*
lw_unknowndictionary_get_uri (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);
    g_return_val_if_fail (dictionary->priv->filename != NULL, NULL);

    //Declarations
    LwDictionaryPrivate *priv;
    gchar *path;

    //Initializations
    priv = dictionary->priv;
    path = lw_util_build_filename (LW_PATH_DICTIONARY_UNKNOWN, priv->filename);
  
    return path;
}


//!
//! @brief Parses a string for an unknown format string
//! @param rl The Resultline object this method works on
//!
static gboolean
lw_unknowndictionary_parse_result (LwDictionary *dictionary, LwResult *result, FILE *file)
{
    return FALSE;
}


static gboolean
lw_unknowndictionary_parse_query (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT)
{
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
    return FALSE;
}


const gchar*
lw_unknowndictionary_get_typename (LwDictionary *dictionary)
{
  return "unknown";
}

