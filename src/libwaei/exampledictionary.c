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

static gchar* lw_exampledictionary_get_uri (LwDictionary*);
static gboolean lw_exampledictionary_parse_query (LwDictionary*, LwQuery*, const gchar*, GError**);
static gboolean lw_exampledictionary_parse_result (LwDictionary*, LwResult*, FILE*);
static const gchar* lw_exampledictionary_get_typename (LwDictionary*);

LwDictionary* lw_exampledictionary_new (const gchar *FILENAME)
{
    g_return_val_if_fail (FILENAME != NULL, NULL);

    //Declarations
    LwDictionary *dictionary;

    //Initializations
    dictionary = LW_DICTIONARY (g_object_new (LW_TYPE_EXAMPLEDICTIONARY,
                                "dictionary-filename", FILENAME,
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
      priv->load_position = 2;
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

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->finalize = lw_exampledictionary_finalize;
    object_class->constructed = lw_exampledictionary_constructed;

    dictionary_class = LW_DICTIONARY_CLASS (klass);
    dictionary_class->parse_query = lw_exampledictionary_parse_query;
    dictionary_class->parse_result = lw_exampledictionary_parse_result;
    dictionary_class->get_uri = lw_exampledictionary_get_uri;
    dictionary_class->get_typename = lw_exampledictionary_get_typename;
}


static gchar*
lw_exampledictionary_get_uri (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    LwDictionaryPrivate *priv;
    gchar *path;

    //Initializations
    priv = LW_DICTIONARY (dictionary)->priv;
    g_return_val_if_fail (priv->filename != NULL, NULL);
    path = lw_util_build_filename (LW_PATH_DICTIONARY_EXAMPLE, priv->filename);
  
    return path;
}


static gboolean 
lw_exampledictionary_parse_query (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT, GError **error)
{
    if (error != NULL && *error != NULL) return;

    //Sanity check
    g_return_val_if_fail (dictionary != NULL && query != NULL && TEXT != NULL, FALSE);

    //Free previously used memory
    lw_query_clean (query);

    return (error == NULL || *error == NULL);
}


//!
//! @brief, Retrieve a line from FILE, parse it according to the LwExampleDictionary rules and put the results into the LwResult
//!
static gboolean
lw_exampledictionary_parse_result (LwDictionary *dictionary, LwResult *result, FILE *fd)
{
    return TRUE;
}


static const gchar*
lw_exampledictionary_get_typename (LwDictionary *dictionary)
{
    return "example";
}


gchar**
lw_exampledictionary_tolkenize_query (LwDictionary *dictionary, LwQuery *query)
{
    return NULL;
}



