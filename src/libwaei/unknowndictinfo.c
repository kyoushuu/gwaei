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
//!  @file unknowndictinfo.c
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
static gchar* lw_unknowndictinfo_get_uri (LwDictInfo*);
static gboolean lw_unknowndictinfo_parse_query (LwDictInfo*, LwQuery*, const gchar*);
static gboolean lw_unknowndictinfo_parse_result (LwDictInfo*, LwResult*, FILE*);


G_DEFINE_TYPE (LwUnknowndictInfo, lw_unknowndictinfo, LW_TYPE_DICTINFO)


LwDictInfo* lw_unknowndictinfo_new (const gchar *FILENAME)
{
    g_return_val_if_fail (FILENAME != NULL, NULL);

    //Declarations
    LwDictInfo *di;

    //Initializations
    di = LW_DICTINFO (g_object_new (LW_TYPE_UNKNOWNDICTINFO,
                                    "dictionary-filename", FILENAME,
                                    NULL));

    return di;
}


static void 
lw_unknowndictinfo_init (LwUnknowndictInfo *di)
{
}


static void
lw_unknowndictinfo_constructed (GObject *object)
{
    //Chain the parent class
    {
      G_OBJECT_CLASS (lw_unknowndictinfo_parent_class)->constructed (object);
    }

/*
    LwDictInfo *di;
    LwDictInfoPrivate *priv;

    di = LW_DICTINFO (object);
    priv = di->priv;
*/
}


static void 
lw_unknowndictinfo_finalize (GObject *object)
{
    G_OBJECT_CLASS (lw_unknowndictinfo_parent_class)->finalize (object);
}


static void
lw_unknowndictinfo_class_init (LwUnknowndictInfoClass *klass)
{
    //Declarations
    GObjectClass *object_class;
    LwDictInfoClass *dictinfo_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = lw_unknowndictinfo_finalize;
    object_class->constructed = lw_unknowndictinfo_constructed;

    dictinfo_class = LW_DICTINFO_CLASS (klass);
    dictinfo_class->parse_query = lw_unknowndictinfo_parse_query;
    dictinfo_class->parse_result = lw_unknowndictinfo_parse_result;
    dictinfo_class->get_uri = lw_unknowndictinfo_get_uri;
}


static gchar*
lw_unknowndictinfo_get_uri (LwDictInfo *di)
{
    //Sanity checks
    g_return_val_if_fail (di != NULL, NULL);
    g_return_val_if_fail (di->priv->filename != NULL, NULL);

    //Declarations
    LwDictInfoPrivate *priv;
    gchar *path;

    //Initializations
    priv = di->priv;
    path = lw_util_build_filename (LW_PATH_DICTIONARY_UNKNOWN, priv->filename);
  
    return path;
}


//!
//! @brief Parses a string for an unknown format string
//! @param rl The Resultline object this method works on
//!
static gboolean
lw_unknowdictinfo_parse_result (LwDictInfo *di, LwResult *result, FILE *file)
{
    return FALSE;
}


static gboolean
lw_unknowndict_parse_query (LwDictInfo *di, LwQuery *query, const gchar *TEXT)
{
    tokens = lw_unknowninfo_get_tokens (di, text);

    if (tokens != NULL)
    {
      for (i = 0; tokens[i] != NULL; i++)
      {
        if (lw_util_is_romaji_str (tokens[i])
          lw_unknowninfo_append_romaji_regex (di, query, tokens[i]);
        else if (lw_util_is_kanji_ish_str (tokens[i])
          lw_unknowninfo_append_kanji_regex (di, query, tokens[i]);
        else if (lw_util_is_furigana_str (tokens[i])
          lw_unknowninfo_append_furigana_regex (di, query, tokens[i]);
        else
          lw_unknowninfo_append_mix_regex (di, query, tokens[i]);
      }
      g_strfreev (tokens); tokens = NULL;
    }


    return FALSE;
}

