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

static void lw_dictionaryinstall_free (LwDictionaryInstall*);

G_DEFINE_TYPE (LwDictionary, lw_dictionary, G_TYPE_OBJECT)

typedef enum
{
  PROP_0,
  PROP_FILENAME
} LwDictionaryProps;


LwDictionary* lw_dictionary_installer_new (GType type)
{
    g_return_val_if_fail (g_type_is_a (type, LW_TYPE_DICTIONARY) != FALSE, NULL);

    //Declarations
    LwDictionary *dictionary;

    //Initializations
    dictionary = LW_DICTIONARY (g_object_new (type, NULL));

    return dictionary;
}


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

    if (priv->install != NULL) lw_dictionaryinstall_free (priv->install); priv->install = NULL;

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
      uri = lw_dictionary_get_path (dictionary);
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
    dictionary_class->parse_query = NULL;
    dictionary_class->parse_result = NULL;

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
    uri =  lw_dictionary_get_path (dictionary);

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
    uri = lw_dictionary_get_path (dictionary);
    
    if (uri != NULL)
    {
      file = fopen (uri, "r");
      g_free (uri); uri = NULL;
    }

    return file;
}


gchar**
lw_dictionary_installer_get_filelist (LwDictionary *dictionary)
{
    LwDictionaryClass *klass;
    gchar **list;

    klass = LW_DICTIONARY_CLASS (G_OBJECT_GET_CLASS (dictionary));

    if (klass->installer_get_filelist != NULL)
    {
      list = klass->installer_get_filelist (dictionary);
    } 

    else
    {
      list = g_new (gchar*, 2);
      list[0] = g_strdup (lw_dictionary_get_filename (dictionary));
      list[1] = NULL;
    }

    return list;
}


gchar*
lw_dictionary_get_directory (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    LwDictionaryPrivate *priv;
    gchar *path;

    //Initializations
    priv = LW_DICTIONARY (dictionary)->priv;
    g_return_val_if_fail (priv->filename != NULL, NULL);
    path = lw_util_build_filename (LW_PATH_DICTIONARY, G_OBJECT_TYPE_NAME (dictionary));
  
    return path;
}


gchar* 
lw_dictionary_get_path (LwDictionary *dictionary)
{
    g_return_val_if_fail (dictionary != NULL, NULL);

    gchar *directory;
    const gchar *filename;
    gchar *path;

    directory = lw_dictionary_get_directory (dictionary);
    filename = lw_dictionary_get_filename (dictionary);
    path = NULL;

    if (directory != NULL)
    {
      path = g_build_filename (directory, filename, NULL);
      g_free (directory); directory = NULL;
    }

    return path;
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
lw_dictionary_build_id (LwDictionary *dictionary)
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


LwDictionaryInstall*
lw_dictionaryinstall_new ()
{
		LwDictionaryInstall *install;

    install = g_new0 (LwDictionaryInstall, 1);

    return install;
}


static void
lw_dictionaryinstall_free (LwDictionaryInstall *install)
{
    if (install->download_preference != NULL) g_free (install->download_preference); install->download_preference = NULL;

    if (install->decompresslist != NULL) g_strfreev (install->decompresslist); install->decompresslist = NULL;
    if (install->encodelist) g_strfreev (install->encodelist); install->encodelist = NULL;
    if (install->postprocesslist) g_strfreev (install->postprocesslist); install->postprocesslist = NULL;
    if (install->installlist) g_strfreev (install->installlist); install->installlist = NULL;
    if (install->installedlist) g_strfreev (install->installedlist); install->installedlist = NULL;

    if (install->preferences != NULL && install->listenerid != 0)
    {
      lw_preferences_remove_change_listener_by_schema (install->preferences, install->schema, install->listenerid);
      install->listenerid = 0;
    }

    if (install->schema != NULL) g_free (install->schema); install->schema = NULL;
    if (install->key != NULL) g_free (install->key); install->key = NULL;
}



/*
    priv->schema = g_strdup (schema);
    priv->key = g_strdup (key);
    priv->listenerid = lw_preferences_add_change_listener_by_schema (
      preferences, 
      schema, 
      key, 
      gw_installdictionary_update_source_uri_cb, 
      dictionary
    );
    priv->preferences = preferences;
*/


//!
//! @brief Updates the encoding of the LwDictionary
//! @param dictionary The LwDictInfo object to set the ENCODING to
//! @param ENCODING Tells the LwDictInfo object what the initial character encoding of the downloaded file will be
//!
void 
lw_dictionary_installer_set_encoding (LwDictionary *dictionary, const LwEncoding ENCODING)
{
		LwDictionaryPrivate *priv;

		priv = dictionary->priv;
    priv->install->encoding = ENCODING;
}


//!
//! @brief Updates the download source of the LwDictionary object
//! @param dictionary The LwDictInfo objcet to set the SOURCE variable on
//! @param SOURCE The source string to copy to the LwDictionary object.
//!
void 
lw_dictionary_installer_set_downloadlist (LwDictionary *dictionary, const gchar *SOURCE)
{
		LwDictionaryPrivate *priv;

		priv = dictionary->priv;

    if (priv->install->download_preference != NULL)
      g_free (priv->install->download_preference);
    priv->install->download_preference = g_strdup (SOURCE);
}


//!
//! @brief Updates the split state of the LwDictionary
//! @param dictionary The LwDictInfo objcet to set the POSTPROCESS variable on
//! @param POSTPROCESS The split setting to copy to the LwDictionary.
//!
void 
lw_dictionary_installer_set_postprocess (LwDictionary *dictionary, const gboolean POSTPROCESS)
{
		LwDictionaryPrivate *priv;

		priv = dictionary->priv;
    priv->install->postprocess = POSTPROCESS;
}


static gchar**
lw_dictionary_installer_get_downloadlist (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;

    //Initalizations
    priv = dictionary->priv;
    install = priv->install;

    if (install->downloadlist == NULL)
    {
      install->downloadlist = g_strsplit (priv->install->download_preference, ";", -1);
    }

    return install->downloadlist;
}


static gchar**
lw_dictionary_installer_get_decompresslist (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;
    gchar **templist, **tempiter;
    gchar *filename;
    gchar *path;

    //Initializations
    priv = dictionary->priv;
    install = priv->install;
    templist = NULL;

    if (install->decompresslist == NULL)
    {
      tempiter = templist = g_strdupv (lw_dictionary_installer_get_downloadlist (dictionary));
      if (templist == NULL) goto errored;

      while (*tempiter != NULL)
      {
        filename = strrchr(*tempiter, G_DIR_SEPARATOR);
        if (filename == NULL || *(filename + 1) == '\0') goto errored;
        filename++;

        path = lw_util_build_filename (LW_PATH_CACHE, filename);
        if (path == NULL) goto errored;
        g_free(*tempiter); *tempiter = path; path = NULL;

        tempiter++;
      }

      install->decompresslist = templist; templist = NULL;
    }

    return install->decompresslist;

errored:
    if (templist != NULL) g_strfreev (templist); templist = NULL;
    return NULL;
}


static gchar**
lw_dictionary_installer_get_encodelist (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;
    gchar **templist, **tempiter;
    const gchar* encodingname;
    gchar *extension;
    gchar *path;

    //Initializations
    priv = dictionary->priv;
    install = priv->install;
    templist = NULL;

    if (install->encodelist == NULL)
    {
      tempiter = templist = g_strdupv (lw_dictionary_installer_get_decompresslist (dictionary));
      if (templist == NULL) goto errored;
      encodingname = lw_util_get_encodingname (install->encoding);

      while (*tempiter != NULL)
      {
        extension = strchr(*tempiter, '.');
        if (extension == NULL) goto errored;

        *extension = '\0';
        path = g_strjoin (".", *tempiter, encodingname, NULL);
        if (path == NULL) goto errored;
        g_free(*tempiter); *tempiter = path; path = NULL;

        tempiter++;
      }

      install->encodelist = templist; templist = NULL;
    }

    return install->encodelist;

errored:
    if (templist != NULL) g_strfreev (templist); templist = NULL;
    return NULL;
}


static gchar**
lw_dictionary_installer_get_postprocesslist (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;
    gchar **templist, **tempiter;
    const gchar* encodingname;
    gchar *extension;
    gchar *path;

    //Initializations
    priv = dictionary->priv;
    install = priv->install;
    encodingname = lw_util_get_encodingname (LW_ENCODING_UTF8);
    templist = NULL;

    if (install->postprocesslist == NULL)
    {
      tempiter = templist = g_strdupv (lw_dictionary_installer_get_encodelist (dictionary));
      if (templist == NULL) goto errored;

      while (tempiter != NULL)
      {
        extension = strchr(*tempiter, '.');
        if (extension == NULL) goto errored;

        *extension = '\0';
        path = g_strjoin (".", *tempiter, encodingname, NULL);
        if (path == NULL) goto errored;
        g_free(*tempiter); *tempiter = path; path = NULL;

        tempiter++;
      }

      install->postprocesslist = templist; templist = NULL;
    }

    return install->postprocesslist;

errored:
    if (templist != NULL) g_strfreev (templist); templist = NULL;
    return NULL;
}


static gchar**
lw_dictionary_installer_get_installlist (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;
    gchar **templist, **tempiter;
    gchar *path;
    
    //Initializations
    priv = dictionary->priv;
    install = priv->install;
    templist = NULL;

    if (install->installlist == NULL)
    {
      tempiter = templist = lw_dictionary_installer_get_filelist (dictionary);
      if (templist == NULL) goto errored;

      while (*tempiter != NULL)
      {
        path = lw_util_build_filename (LW_PATH_CACHE, *tempiter);
        if (path == NULL) goto errored;
        g_free (*tempiter); *tempiter = path; path = NULL;
    
        tempiter++;
      }

      install->installlist = templist; templist = NULL;
    }

    return install->installlist;

errored:
    if (templist != NULL) g_strfreev (templist); templist = NULL;
    return NULL;
}


static gchar**
lw_dictionary_installer_get_installedlist (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;
    gchar **templist, **tempiter;
    gchar *path;
    gchar *directory;
    
    //Initializations
    priv = dictionary->priv;
    install = priv->install;
    directory = lw_dictionary_get_directory (dictionary);
    templist = NULL;

    if (install->installedlist == NULL)
    {
      tempiter = templist = lw_dictionary_installer_get_filelist (dictionary);
      if (templist == NULL) goto errored;

      while (*tempiter != NULL)
      {
        path = g_build_filename (directory, *tempiter, NULL);
        if (path == NULL) goto errored;
        g_free (*tempiter); *tempiter = path; path = NULL;
    
        tempiter++;
      }

      install->installedlist = templist; templist = NULL;
    }

    return install->installedlist;

errored:
    if (templist != NULL) g_strfreev (templist); templist = NULL;
    if (directory != NULL) g_free (directory); directory = NULL;
    return NULL;
}


gboolean
lw_dictionary_installer_is_selected (LwDictionary *dictionary)
{
    //Sanity check
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
    LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;

    //Initializations
    priv = dictionary->priv;
    install = priv->install;

    return install->selected;
}


//!
//! @brief Tells the installer mechanism if it is going to fail if it tries
//!        installing because of missing info
//! @param dictionary The LwDictionary objcet to check the validity of the urls of.  This tells you
//!        if the LWDictionary object can have the lw_installdictionary_install method
//!        called without crashing.
//!
gboolean 
lw_dictionary_installer_is_valid (LwDictionary *dictionary)
{
    //Declarations
		LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;
    gchar **filelist;
    gchar **downloadlist;
    gboolean filesvalid, downloadsvalid, encodingvalid;

		priv = dictionary->priv;
    install = priv->install;

    downloadlist = lw_dictionary_installer_get_downloadlist (dictionary);
    downloadsvalid = (downloadlist != NULL && g_strv_length (downloadlist) > 0);
    if (downloadlist != NULL) g_strfreev (downloadlist); downloadlist = NULL;

    filelist = lw_dictionary_installer_get_filelist (dictionary);
    filesvalid = (filelist != NULL && g_strv_length (filelist) > 0);
    if (filelist != NULL) g_strfreev (filelist); filelist = NULL;

    encodingvalid = (install->encoding >= 0 && install->encoding < LW_ENCODING_TOTAL);

    return (downloadsvalid && filesvalid && encodingvalid);;
}


//!
//! @brief Downloads or copies the file to the dictionary directory to be worked on
//!        This function should normally only be used in the lw_installdictionary_install method.
//! @param dictionary The LwDictionary object to use to download the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the download is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
static gboolean 
lw_dictionary_installer_download (LwDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
    LwDictionaryPrivate *priv;
    gchar **sourcelist, **sourceiter;
    gchar **targetlist, **targetiter;

    //Initializations
    priv = dictionary->priv;
    sourceiter = sourcelist = lw_dictionary_installer_get_downloadlist (dictionary);
    targetiter = targetlist = lw_dictionary_installer_get_decompresslist (dictionary);

    if (priv->install->cancel) return FALSE;

    if (sourcelist != NULL && targetlist != NULL)
    {
      while (*sourceiter != NULL && *targetiter != NULL)
      {
        //File is located locally so copy it
        if (g_file_test (*sourceiter, G_FILE_TEST_IS_REGULAR))
          lw_io_copy (*sourceiter, *targetiter, (LwIoProgressCallback) cb, data, error);
        //Download the file
        else
          lw_io_download (*sourceiter, *targetiter, cb, data, error);

        sourceiter++;
        targetiter++;
      }
    }

    return (*error == NULL);
}


//!
//! @brief Detects the compression scheme of a file and decompresses it using the approprate function.
//!        This function should normally only be used in the lw_installdictionary_install function.
//! @param dictionary The LwDictionary object to use to decompress the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the decompression is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
static gboolean 
lw_dictionary_installer_decompress (LwDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
		LwDictionaryPrivate *priv;
    gchar **sourcelist, **sourceiter;
    gchar **targetlist, **targetiter;

    //Initializations
		priv = dictionary->priv;

    if (priv->install->cancel) return FALSE;

    sourceiter = sourcelist = lw_dictionary_installer_get_decompresslist (dictionary);
    targetiter = targetlist = lw_dictionary_installer_get_encodelist (dictionary);

    if (sourcelist != NULL && targetlist != NULL)
    {
      while (*sourceiter != NULL && *targetiter != NULL)
      {
        if (g_file_test (*sourceiter, G_FILE_TEST_IS_REGULAR))
        {
          if (g_str_has_suffix (*sourceiter, "gz") || g_str_has_suffix (*sourceiter, "gzip"))
            lw_io_gunzip_file (*sourceiter, *targetiter, cb, data, error);
          else
            lw_io_copy (*sourceiter, *targetiter, cb, data, error);
        }

        sourceiter++;
        targetiter++;
      }
    }

    return (error == NULL && *error == NULL);
}


//!
//! @brief Converts the encoding to UTF8 for the file
//!        This function should normally only be used in the lw_installdictionary_install function.
//! @param dictionary The LwDictionary object to use to convert the text encoding the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the text encoding conversion is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
static gboolean 
lw_dictionary_installer_convert_encoding (LwDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
		LwDictionaryPrivate *priv;
    gchar **sourcelist, **sourceiter;
    gchar **targetlist, **targetiter;
    const gchar *encodingname;

    //Initializations
		priv = dictionary->priv;
    sourceiter = sourcelist = lw_dictionary_installer_get_encodelist (dictionary);
    targetiter = targetlist = lw_dictionary_installer_get_postprocesslist (dictionary);
    encodingname = lw_util_get_encodingname (priv->install->encoding);

    if (sourcelist != NULL && targetlist != NULL)
    {
      while (*sourceiter != NULL && *targetiter != NULL)
      {
        if (priv->install->encoding == LW_ENCODING_UTF8)
          lw_io_copy (*sourceiter, *targetiter, cb, data, error);
        else
          lw_io_copy_with_encoding (*targetiter, *sourceiter, encodingname, "UTF-8", cb, data, error);

        sourceiter++;
        targetiter++;
      }
    }

    return (*error == NULL);
}


//!
//! @brief does the required postprocessing on a dictionary
//!        This function should normally only be used in the lw_installdictionary_install function.
//! @param dictionary The LwDictionary object to use for postprocessing the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the postprocessing is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
static gboolean 
lw_dictionary_installer_postprocess (LwDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
    LwDictionaryClass *klass;
		LwDictionaryPrivate *priv;
    gchar **sourcelist, **sourceiter;
    gchar **targetlist, **targetiter;

    //Initializations
    klass = LW_DICTIONARY_CLASS (G_OBJECT_GET_CLASS (dictionary));
		priv = dictionary->priv;
    sourceiter = sourcelist = lw_dictionary_installer_get_postprocesslist (dictionary);
    targetiter = targetlist = lw_dictionary_installer_get_installlist (dictionary);

    if (priv->install->cancel) return FALSE;

    if (sourcelist != NULL && targetlist != NULL)
    {
      if (klass->installer_postprocess != NULL)
      {
        klass->installer_postprocess (dictionary, sourcelist, targetlist, cb, data, error);
      }
      else
      {
        while (*sourceiter != NULL && *targetiter != NULL)
        {
          lw_io_copy (*sourceiter, *targetiter, cb, data, error);

          sourceiter++;
          targetiter++;
        }
      }
    }

    //Finish
    return (*error == NULL);
}


//!
//! @brief does the required postprocessing on a dictionary
//!        This function should normally only be used in the lw_installdictionary_install function.
//! @param dictionary The LwDictionary object to use for finalizing the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the finalization is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
static gboolean 
lw_dictionary_installer_install (LwDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
    LwDictionaryPrivate *priv;
    gchar **sourcelist, **sourceiter;
    gchar **targetlist, **targetiter;

    //Initializations
    priv = dictionary->priv;
    sourceiter = sourcelist = lw_dictionary_installer_get_installlist (dictionary);
    targetiter = targetlist = lw_dictionary_installer_get_installedlist (dictionary);

    if (priv->install->cancel) return FALSE;

    if (sourcelist != NULL && targetiter != NULL)
    {
      while (*sourceiter != NULL && *targetiter != NULL)
      {
        lw_io_copy (*sourceiter, *targetiter, cb, data, error);

        sourceiter++;
        targetiter++;
      }
    }

    //Finish
    return (*error == NULL);
}


//!
//! @brief removes temporary files created by installation in the dictionary cache folder
//! @param dictionary The LwDictionary object to use to clean the files.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the finalization is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//!
static void 
lw_dictionary_installer_clean (LwDictionary *dictionary, LwIoProgressCallback cb, gpointer data)
{
/*
    //Declarations
		LwDictionaryPrivate *priv;
    LwDictionaryUri group_index;
    gint i;
    gchar *source;

    //Initializations
		priv = dictionary->priv;
    group_index = 0;

    //Loop through all of the uris except the final destination
    while (group_index < LW_INSTALLDICTIONARY_NEEDS_NOTHING)
    {
      i = 0;
      while ((source = lw_installdictionary_get_source_uri (dictionary, group_index, i)) != NULL)
      {
        g_remove (source);
        i++;
      }
      group_index++;
    }

    if (priv->current_source_uris != NULL) g_strfreev (priv->current_source_uris);
    if (priv->current_target_uris != NULL) g_strfreev (priv->current_target_uris);
    priv->current_source_uris = NULL;
    priv->current_target_uris = NULL;
    priv->uri_group_index = -1;
    priv->uri_atom_index = -1;
*/
}


//!
//! @brief Installs a LwDictionary object using the provided gui update callback
//!        This function should normally only be used in the lw_installdictionary_install function.
//! @param dictionary The LwDictionary object to use for installing the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the installation is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
gboolean 
lw_dictionary_install (LwDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    g_assert (*error == NULL && dictionary != NULL);

    lw_dictionary_installer_download (dictionary, cb, data, error);
    lw_dictionary_installer_decompress (dictionary, cb, data, error);
    lw_dictionary_installer_convert_encoding (dictionary, cb, data, error);
    lw_dictionary_installer_postprocess (dictionary, cb, data, error);
    lw_dictionary_installer_install (dictionary, cb, data, error);
    lw_dictionary_installer_clean (dictionary, cb, data);

    return (*error == NULL);
}


//!
//! @brief Returns a status string describing the current process being taken
//!        on a LwDictionary object
//! @param dictionary A LwDictionary object to get the status of.
//! @param long_form Whether you want the long or short form of the status messages.
//! @returns An allocated string that should be freed with gfree when finished
//!
gchar* 
lw_installdictionary_get_status_string (LwDictionary *dictionary, gboolean long_form)
{
/*
    //Declarations
		LwDictionaryPrivate *priv;
    gchar *text;
    const gchar *longname;

		priv = dictionary->priv;
    longname = lw_dictionary_get_longname (LW_DICTIONARY (longname));

    switch (priv->uri_group_index) {
      case LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING:
        if (long_form)
          text = g_strdup_printf (gettext("Downloading %s..."), longname);
        else
          text = g_strdup_printf (gettext("Downloading..."));
        break;
      case LW_INSTALLDICTIONARY_NEEDS_TEXT_ENCODING:
        if (long_form)
          text = g_strdup_printf (gettext("Converting the encoding of %s from %s to UTF-8..."), longname, lw_util_get_encodingname (priv->install->encoding));
        else
          text = g_strdup_printf (gettext("Converting the encoding to UTF-8..."));
        break;
      case LW_INSTALLDICTIONARY_NEEDS_DECOMPRESSION:
        if (long_form)
          text = g_strdup_printf (gettext("Decompressing %s from %s file..."), longname, lw_util_get_compressionname (priv->install->compression));
        else
          text = g_strdup_printf (gettext("Decompressing..."));
        break;
      case LW_INSTALLDICTIONARY_NEEDS_POSTPROCESSING:
        if (long_form)
          text = g_strdup_printf (gettext("Doing postprocessing on %s..."), longname);
        else
          text = g_strdup_printf (gettext("Postprocessing..."));
        break;
      case LW_INSTALLDICTIONARY_NEEDS_FINALIZATION:
        text = g_strdup_printf (gettext("Finalizing installation of %s..."), longname);
        break;
      case LW_INSTALLDICTIONARY_NEEDS_NOTHING:
        text = g_strdup_printf (gettext("Installed."));
        break;
      default:
        text = g_strdup_printf (" ");
        break;
    }
*/
    gchar *text = NULL;

    return text;
}


//!
//! @brief Returns the percent progress as a double for only the current LwDictionary
//! @param dictionary The LwDictionary to get the progress of
//! @param fraction The fraction percentage of the current process
//! @param The fraction percentage of all the LwDictionary processes together.
//!
double lw_installdictionary_get_process_progress (LwDictionary *dictionary, double fraction)
{
/*
    //Declarations
    gdouble current;
    gdouble final;
    gdouble output_fraction;
    gchar *ptr;
		LwDictionaryPrivate *priv;
    
    //Initializations
    current = 0.0;
    final = 0.0;
    output_fraction = 0.0;
		priv = dictionary->priv;
    priv->progress = fraction;

    //Get the current progress
    current = fraction + ((double) priv->uri_atom_index);

    //Calculate the amount needed for the whole process to finish
    for (ptr = priv->uri[priv->uri_group_index]; ptr != NULL; ptr = strchr(ptr, ';'))
    {
      final += 1.0;
      ptr++;
    }

    if (final > 0.0)
      output_fraction = current / final;
*/
    gdouble output_fraction = 0.0;

    return output_fraction;
}


//!
//! @brief Gets the total possible progress for the LwDictionary.  It should be
//!        divided by the current progress to get the appropriate fraction
//! @param dictionary The LwDictInfo object to get the total progress of
//! @param fraction The fraction progress of the current process.
//!
double 
lw_installdictionary_get_total_progress (LwDictionary *dictionary, double fraction)
{
/*
    //Declarations
		LwDictionaryPrivate *priv;
    gdouble output_fraction, current, final;
    gint i;
    gchar *ptr;

    //Definitions
		priv = dictionary->priv;
    output_fraction = 0.0;
    current = 0.0;
    final = 0.0;
    priv->progress = fraction;
    const gdouble DOWNLOAD_WEIGHT = 3.0;

    //Calculate the already completed activities
    for (i = 0; i < priv->uri_group_index && i < LW_INSTALLDICTIONARY_NEEDS_NOTHING; i++)
    {
      for (ptr = priv->uri[i]; ptr != NULL; ptr = strchr(ptr, ';'))
      {
        if (i == LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING)
          current += 1.0 * DOWNLOAD_WEIGHT;
        else
          current += 1.0;
        ptr++;
      }
    }
    //Add the current in progress activity
    if (i == LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING)
      current += (fraction + (double) priv->uri_atom_index) * DOWNLOAD_WEIGHT;
    else
      current += fraction + (double) priv->uri_atom_index;

    //Calculate the amount needed for the whole process to finish
    for (i = 0; i < LW_INSTALLDICTIONARY_NEEDS_NOTHING; i++)
    {
      for (ptr = priv->uri[i]; ptr != NULL; ptr = strchr(ptr, ';'))
      {
        if (i == LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING)
          final += 1.0 * DOWNLOAD_WEIGHT;
        else
          final += 1.0;
        ptr++;
      }
    }

    if (final > 0.0)
      output_fraction = current / final;
*/  
    double output_fraction = 0.0;

    return output_fraction;
}


//!
//! @brief Used to tell the LwDictionary installer to stop installation.
//! @param dictionary The LwDictionary object to stop or prevent the install on.
//! @param state Whether to turn on the requested cancel operation or not.
//!
void 
lw_installdictionary_set_cancel_operations (LwDictionary *dictionary, gboolean state)
{
    LwDictionaryPrivate *priv;

    priv = dictionary->priv;
    priv->install->cancel = state;
    lw_io_set_cancel_operations (state);
}

