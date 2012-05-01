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
//!  @file dictinfo.c
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

#include <libwaei/gettext.h>
#include <libwaei/libwaei.h>

#include <libwaei/dictionary-private.h>

G_DEFINE_TYPE (LwDictionary, lw_dictionary, G_TYPE_OBJECT)

typedef enum
{
  PROP_0,
  PROP_FILENAME
} LwDictionaryProps;


static void 
lw_dictionary_init (LwDictionary *dictionary)
{
    dictionary->priv = LW_DICTIONARY_GET_PRIVATE (dictionary);
    memset(dictionary->priv, 0, sizeof(LwDictionaryPrivate));

    dictionary->priv->length = -1;
}


static void 
lw_dictionary_finalize (GObject *object)
{
    //Declarations
    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;

    //Initalizations
    dictionary = LW_DICTIONARY (object);
    priv = dictionary->priv;

    if (priv->filename != NULL) g_free (priv->filename); priv->filename = NULL;
    if (priv->longname != NULL) g_free (priv->longname); priv->longname = NULL;
    if (priv->shortname != NULL) g_free (priv->shortname); priv->shortname = NULL;

    G_OBJECT_CLASS (lw_dictionary_parent_class)->finalize (object);
}


static void 
lw_dictionary_set_property (GObject      *object,
                            guint         property_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
    //Declarations
    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;

    //Initializations
    dictionary = LW_DICTIONARY (object);
    priv = dictionary->priv;

    switch (property_id)
    {
      case PROP_FILENAME:
        if (priv->filename != NULL) g_free (priv->filename);
        priv->filename = g_value_dup_string (value);
        if (priv->longname != NULL) g_free (priv->longname);
        priv->longname = g_strdup_printf (gettext("%s Dictionary"), priv->filename);
        if (priv->shortname != NULL) g_free (priv->shortname);
        priv->shortname = g_strdup (priv->filename);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void 
lw_dictionary_get_property (GObject      *object,
                        guint         property_id,
                        GValue       *value,
                        GParamSpec   *pspec)
{
    //Declarations
    LwDictionary *dictionary;
    LwDictionaryPrivate *priv;

    //Initializations
    dictionary = LW_DICTIONARY (object);
    priv = dictionary->priv;

    switch (property_id)
    {
      case PROP_FILENAME:
        g_value_set_string (value, priv->filename);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


size_t
lw_dictionary_get_length (LwDictionary *dictionary)
{
    g_return_val_if_fail (dictionary != NULL, -1);

    //Declarations
    LwDictionaryPrivate *priv;
    gchar *uri;

    //Initializations
    priv = dictionary->priv;

    if (priv->length < 0)
    {
      uri = lw_dictionary_get_uri (dictionary);
      if (uri != NULL)
      {
        priv->length = lw_io_get_size_for_uri (uri);
        g_free (uri); uri = NULL;
      }
    }

    return priv->length;
}


static void
lw_dictionary_class_init (LwDictionaryClass *klass)
{
    //Declarations
    GParamSpec *pspec;
    GObjectClass *object_class;
    LwDictionaryClass *dictionary_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = lw_dictionary_set_property;
    object_class->get_property = lw_dictionary_get_property;
    object_class->finalize = lw_dictionary_finalize;

    dictionary_class = LW_DICTIONARY_CLASS (klass);
    dictionary_class->get_uri = NULL;
    dictionary_class->parse_query = NULL;
    dictionary_class->parse_result = NULL;
    dictionary_class->get_typename = NULL;

    g_type_class_add_private (object_class, sizeof (LwDictionaryPrivate));

    pspec = g_param_spec_string ("filename",
                                 "Filename of the dictionary",
                                 "Set the dictionary's filename",
                                 "",
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE
    );
    g_object_class_install_property (object_class, PROP_FILENAME, pspec);
}




//!
//! @brief Deletes a LwDictionary from the harddrive.  LwDictInst objects are used
//!        for installing dictionaries that do not exist yet.  You still need to free
//!        the object after.
//! @param dictionary An LwDictionary object to get the paths for the dictionary file.
//! @param cb A LwIoProgresSCallback to show dictionary uninstall progress or NULL.
//! @param error A pointer to a GError object to pass errors to or NULL.
//!
gboolean 
lw_dictionary_uninstall (LwDictionary *dictionary, LwIoProgressCallback cb, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_assert (dictionary != NULL);

    //Declarations
    gchar *uri;

    //Initializations
    uri =  lw_dictionary_get_uri (dictionary);

    if (uri != NULL)
    {
      lw_io_remove (uri, error);
      if (cb != NULL) cb (1.0, dictionary);

      g_free (uri); uri = NULL;
    }

    return (*error == NULL);
}
 

FILE*
lw_dictionary_open (LwDictionary *dictionary)
{
    FILE *file;
    gchar *uri;

    file = NULL;
    uri = lw_dictionary_get_uri (dictionary);
    
    if (uri != NULL)
    {
      file = fopen (uri, "r");
      g_free (uri); uri = NULL;
    }

    return file;
}


gchar* 
lw_dictionary_get_uri (LwDictionary *dictionary)
{
    g_return_val_if_fail (dictionary != NULL, NULL);

    LwDictionaryClass *klass;

    klass = LW_DICTIONARY_CLASS (G_OBJECT_GET_CLASS (dictionary));

    g_return_val_if_fail (klass->get_uri != NULL, NULL);

    return klass->get_uri (dictionary);
}


gboolean 
lw_dictionary_parse_query (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT, GError **error)
{
    g_return_val_if_fail (dictionary != NULL && query != NULL && TEXT != NULL, FALSE);

    LwDictionaryClass *klass;

    klass = LW_DICTIONARY_CLASS (G_OBJECT_GET_CLASS (dictionary));

    g_return_val_if_fail (klass->parse_query != NULL, FALSE);

    if (query->text != NULL) g_free (query->text);
    query->text = g_strdup (TEXT);

    lw_query_init_tokens (query);
    lw_query_init_regexgroup (query);

    return klass->parse_query (dictionary, query, TEXT, error);
}


gboolean 
lw_dictionary_parse_result (LwDictionary *dictionary, LwResult *result, FILE *fd)
{
    g_return_val_if_fail (dictionary != NULL && result != NULL && fd != NULL, FALSE);

    LwDictionaryClass *klass;

    klass = LW_DICTIONARY_CLASS (G_OBJECT_GET_CLASS (dictionary));

    g_return_val_if_fail (klass->parse_result != NULL, FALSE);

    return klass->parse_result (dictionary, result, fd);
}


const gchar*
lw_dictionary_get_longname (LwDictionary *dictionary)
{
    LwDictionaryPrivate *priv;

    priv = dictionary->priv;

    return priv->longname;
}


const gchar*
lw_dictionary_get_filename (LwDictionary *dictionary)
{
    LwDictionaryPrivate *priv;

    priv = dictionary->priv;

    return priv->filename;
}


const gchar* 
lw_dictionary_get_typename (LwDictionary *dictionary)
{
    g_return_val_if_fail (dictionary != NULL, FALSE);

    LwDictionaryClass *klass;

    klass = LW_DICTIONARY_CLASS (G_OBJECT_GET_CLASS (dictionary));

    g_return_val_if_fail (klass->get_typename != NULL, FALSE);

    return klass->get_typename (dictionary);
}


//!
//! @brief Parses the dicttype from a string
//! @param ENGINENAME The LwDictType in string form
//! @returns A LwDictType value or -1 if it is invalid
//!
GType
lw_dictionary_get_type_from_typename (const char *TYPENAME)
{
/*
  //Declarations
  char *lower;
  LwDictType engine;

  //Initializations
  lower = g_utf8_strdown (ENGINENAME, -1);
  engine = -1;

  if (strcmp(lower, "edict") == 0)
  {
    engine = LW_DICTTYPE_EDICT;
    engine = g_type_from_name ()
  }
  else if (strcmp(lower, "kanji") == 0)
  {
    engine = LW_DICTTYPE_KANJI;
  }
  else if (strcmp(lower, "examples") == 0)
  {
    engine = LW_DICTTYPE_EXAMPLES;
  }
  else if (strcmp(lower, "unknown") == 0)
  {
    engine = LW_DICTTYPE_UNKNOWN;
  }
  else
  {
    g_assert_not_reached ();
    engine = -1;
  }

  g_free (lower);
  lower = NULL;

  return engine;
*/
  return 0;
}


//!
//! @brief Comparison function that should be moved to the LwSearch file when it matures
//! @param item A LwSearch to get search information from
//! @param RELEVANCE A LwRelevance
//! @returns Returns true according to the relevance level
//!
gboolean 
lw_dictionary_compare (LwDictionary *dictionary, LwQuery *query, LwResult *result, const LwRelevance RELEVANCE)
{
    g_return_val_if_fail (dictionary != NULL, FALSE);

    LwDictionaryClass *klass;

    klass = LW_DICTIONARY_CLASS (G_OBJECT_GET_CLASS (dictionary));

    g_return_val_if_fail (klass->compare != NULL, FALSE);

    return klass->compare (dictionary, query, result, RELEVANCE);
}


gboolean
lw_dictionary_equals (LwDictionary *dictionary1, LwDictionary *dictionary2)
{
    //Sanity checks
    g_return_val_if_fail (dictionary1 != NULL && dictionary2 != NULL, FALSE);

    //Declarations
    const gchar *FILENAME1;
    const gchar *FILENAME2;
    GType type1;
    GType type2;
    gboolean filenames_are_equal;
    gboolean types_are_equal;

    //Initializations
    FILENAME1 = lw_dictionary_get_filename (dictionary1);
    FILENAME2 = lw_dictionary_get_filename (dictionary2);
    type1 = G_OBJECT_TYPE (dictionary1);
    type2 = G_OBJECT_TYPE (dictionary2);

    filenames_are_equal = (strcmp(FILENAME1, FILENAME2) == 0);
    types_are_equal = g_type_is_a (type1, type2);

    return (filenames_are_equal && types_are_equal);
}


gchar*
lw_dictionary_build_description (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    gchar *description;
    const gchar *TYPENAME;
    const gchar *FILENAME;

    //Initializations
    TYPENAME = G_OBJECT_TYPE_NAME (dictionary);
    FILENAME = lw_dictionary_get_filename (dictionary);
    description = g_strdup_printf ("%s/%s", TYPENAME, FILENAME);

    return description;
}

