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


/*install dictionary code*/


LwDictionaryInstall*
lw_installdictionary_new ()
{
		LwInstallDictionary *installdictionary;

    install = g_new0(LwDictionaryInstall, 1);

    if (install != NULL)
    {
      priv->uri_group_index = -1;
      priv->uri_atom_index = -1;

    }

    return install;
}


static void
lw_installdictionary_free (LwDictionaryInstall *install)
{
    //Declarations
		int i;

    //Initalizations

    if (install->preferences != NULL && install->listenerid != 0)
    {
      lw_preferences_remove_change_listener_by_schema (install->preferences, install->schema, install->listenerid);
      install->listenerid = 0;
    }

    g_strfreev (install->current_source_uris); install->current_source_uris = NULL;
    g_strfreev (install->current_target_uris); install->current_target_uris = NULL;

    for (i = 0; i < LW_INSTALLDICTIONARY_TOTAL_URIS; i++)
    {
      g_free(install->uri[i]); install->uri[i] = NULL;
    }

    if (install->schema != NULL) g_free (install->schema); install->schema = NULL;
    if (install->key != NULL) g_free (install->key); install->key = NULL;

    g_mutex_clear (&install->mutex);
}



/*
    pirv->schema = g_strdup (schema);
    priv->key = g_strdup (key);
    priv->listenerid = lw_preferences_add_change_listener_by_schema (preferences, schema, key, gw_installdictionary_update_source_uri_cb, dictionary);
    priv->preferences = preferences;
*/


//!
//! @brief A callback that updates the LwInstallDictionary source uri when the pref changes
//! @param setting A GSetting object
//! @param KEY The key of the pref
//! @param data User data passed to the preference listener
//!
void gw_installdictionary_update_source_uri_cb (GSettings *settings, gchar* key, gpointer data)
{
    //Declarations
    LwInstallDictionary *dictionary;
		LwInstallDictionaryPrivate *priv;
    gchar source_uri[200];

    //Initialiations
    dictionary = LW_INSTALLDICTIONARY (data);
		priv = dictionary->priv;
    lw_preferences_get_string (source_uri, settings, key, 200);

    if (priv->uri[LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING] != NULL)
      g_free (priv->uri[LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING]);
    priv->uri[LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING] = g_strdup (source_uri);
}


//!
//! @brief Updates the engine of the LwInstallDictionary
//! @param dictionary The LwInstallDictionary object to set the DICTTYPE to
//! @param DICTTYPE the engine that you want to set
//!
void 
lw_installdictionary_set_engine (LwInstallDictionary *dictionary, GType type)
{
		LwInstallDictionaryPrivate *priv;

		priv = dictionary->priv;
    priv->type = type;

    lw_installdictionary_regenerate_save_target_uris (dictionary);
}


//!
//! @brief Updates the encoding of the LwInstallDictionary
//! @param dictionary The LwDictInfo object to set the ENCODING to
//! @param ENCODING Tells the LwDictInfo object what the initial character encoding of the downloaded file will be
//!
void 
lw_installdictionary_set_encoding (LwInstallDictionary *dictionary, const LwEncoding ENCODING)
{
		LwInstallDictionaryPrivate *priv;

		priv = dictionary->priv;
    priv->encoding = ENCODING;

    lw_installdictionary_regenerate_save_target_uris (dictionary);
}


//!
//! @brief Updates the compression of the LwInstallDictionary
//! @param dictionary The LwDictInfo objcet to set the COMPRESSION variable on
//! @param COMPRESSION Tells the LwDictInfo object what kind of compression the downloaded dictionary file will have.
//!
void 
lw_installdictionary_set_compression (LwInstallDictionary *dictionary, const LwCompression COMPRESSION)
{
		LwInstallDictionaryPrivate *priv;

		priv = dictionary->priv;
    priv->compression = COMPRESSION;

    lw_installdictionary_regenerate_save_target_uris (dictionary);
}




//!
//! @brief Updates the download source of the LwInstallDictionary object
//! @param dictionary The LwDictInfo objcet to set the SOURCE variable on
//! @param SOURCE The source string to copy to the LwInstallDictionary object.
//!
void 
lw_installdictionary_set_download_source (LwInstallDictionary *dictionary, const gchar *SOURCE)
{
		LwInstallDictionaryPrivate *priv;

		priv = dictionary->priv;

    if (priv->uri[LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING] != NULL)
      g_free (priv->uri[LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING]);
    priv->uri[LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING] = g_strdup (SOURCE);
}


//!
//! @brief Updates the merge state of the LwInstallDictionary.
//! @param dictionary The LwDictInfo objcet to set the MERGE variable on.
//! @param MERGE The merge setting to copy to the LwInstallDictionary.
//!
void 
lw_installdictionary_set_merge (LwInstallDictionary *dictionary, const gboolean MERGE)
{
		LwInstallDictionaryPrivate *priv;

		priv = dictionary->priv;
    priv->merge = MERGE;

    lw_installdictionary_regenerate_save_target_uris (dictionary);
}


//!
//! @brief Updates the split state of the LwInstallDictionary
//! @param dictionary The LwDictInfo objcet to set the SPLIT variable on
//! @param SPLIT The split setting to copy to the LwInstallDictionary.
//!
void 
lw_installdictionary_set_split (LwInstallDictionary *dictionary, const gboolean SPLIT)
{
		LwInstallDictionaryPrivate *priv;

		priv = dictionary->priv;
    priv->split = SPLIT;

    lw_installdictionary_regenerate_save_target_uris (dictionary);
}


//!
//! @brief This method should be called after the filename, engine, compression,
//!        or encoding members of the LwInstallDictionary is changed to sync the new paths
//! @dictionary The LwDictInfo object to regenerate the save target uris of.
//!
void 
lw_installdictionary_regenerate_save_target_uris (LwInstallDictionary *dictionary)
{
    //Sanity check
    g_assert (dictionary != NULL);

    //Declarations
		LwInstallDictionaryPrivate *priv;
    const gchar* filename;
    gchar *cache_filename;
    const gchar *compression_ext;
    const gchar *encoding_ext;
    gchar *temp[2][LW_INSTALLDICTIONARY_TOTAL_URIS];
    gchar *radicals_cache_filename;
    gint i, j;
	
		priv = dictionary->priv;

    //Remove the previous contents
    g_free (priv->uri[LW_INSTALLDICTIONARY_NEEDS_DECOMPRESSION]);
    g_free (priv->uri[LW_INSTALLDICTIONARY_NEEDS_TEXT_ENCODING]);
    g_free (priv->uri[LW_INSTALLDICTIONARY_NEEDS_POSTPROCESSING]);
    g_free (priv->uri[LW_INSTALLDICTIONARY_NEEDS_FINALIZATION]);
    g_free (priv->uri[LW_INSTALLDICTIONARY_NEEDS_NOTHING]);

    //Initialize the array
    for (i = 0; i < 2; i++)
      for (j = 1; j < LW_INSTALLDICTIONARY_TOTAL_URIS; j++)
        temp[i][j] = NULL;

    //Initializations
    filename = lw_dictionary_get_filename (LW_DICTIONARY (dictionary));
    cache_filename = lw_util_build_filename (LW_PATH_CACHE, filename);
    compression_ext = lw_util_get_compression_name (priv->compression);
    encoding_ext = lw_util_get_encoding_name (priv->encoding);

    temp[0][LW_INSTALLDICTIONARY_NEEDS_DECOMPRESSION] =  g_strjoin (".", cache_filename, compression_ext, NULL);
    temp[0][LW_INSTALLDICTIONARY_NEEDS_TEXT_ENCODING] =   g_strjoin (".", cache_filename, encoding_ext, NULL);
    temp[0][LW_INSTALLDICTIONARY_NEEDS_POSTPROCESSING] =   g_strjoin (".", cache_filename, "UTF8", NULL);
    temp[0][LW_INSTALLDICTIONARY_NEEDS_FINALIZATION] =  g_strdup (cache_filename);
    temp[0][LW_INSTALLDICTIONARY_NEEDS_NOTHING] =  g_strdup (filename);

    //Adjust the uris for the split dictionary exception case
    if (priv->split)
    {
      g_free (temp[0][LW_INSTALLDICTIONARY_NEEDS_FINALIZATION]);
      temp[0][LW_INSTALLDICTIONARY_NEEDS_FINALIZATION] = lw_util_build_filename (LW_PATH_CACHE, "Names");
      temp[1][LW_INSTALLDICTIONARY_NEEDS_FINALIZATION] = lw_util_build_filename (LW_PATH_CACHE, "Places");

      g_free (temp[0][LW_INSTALLDICTIONARY_NEEDS_NOTHING]);
      temp[0][LW_INSTALLDICTIONARY_NEEDS_NOTHING] = lw_util_build_filename_by_dicttype (priv->type, "Names");
      temp[1][LW_INSTALLDICTIONARY_NEEDS_NOTHING] = lw_util_build_filename_by_dicttype (priv->type, "Places");
    }
    //Adjust the uris for the merge dictionary exception case
    else if (priv->merge)
    {
      radicals_cache_filename = lw_util_build_filename (LW_PATH_CACHE, "Radicals");
      temp[1][LW_INSTALLDICTIONARY_NEEDS_DECOMPRESSION] =  g_strjoin (".", radicals_cache_filename, "gz", NULL);
      temp[1][LW_INSTALLDICTIONARY_NEEDS_TEXT_ENCODING] =   g_strjoin (".", radicals_cache_filename, "EUC-JP", NULL);
      temp[1][LW_INSTALLDICTIONARY_NEEDS_POSTPROCESSING] =   g_strjoin (".", radicals_cache_filename, "UTF8", NULL);
      g_free (radicals_cache_filename);
    }

    //Join the strings if appropriate
    for (i = 1; i < LW_INSTALLDICTIONARY_TOTAL_URIS; i++)
    {
      priv->uri[i] = g_strjoin (";", temp[0][i], temp[1][i], NULL);
    }

    //Cleanup
    for (i = 0; i < 2; i++)
      for (j = 1; j < LW_INSTALLDICTIONARY_TOTAL_URIS; j++)
        if (temp[i][j] != NULL) g_free (temp[i][j]);

    g_free (cache_filename);

/*
    for (i = 1; i < LW_INSTALLDICTIONARY_TOTAL_URIS; i++)
      printf("%s\n", priv->uri[i]);
    printf("\n");
*/
}


//!
//! @brief Tells the installer mechanism if it is going to fail if it tries
//!        installing because of missing info
//! @param dictionary The LwInstallDictionary objcet to check the validity of the urls of.  This tells you
//!        if the LWInstallDictionary object can have the lw_installdictionary_install method
//!        called without crashing.
//!
gboolean 
lw_installdictionary_data_is_valid (LwInstallDictionary *dictionary)
{
    //Declarations
		LwInstallDictionaryPrivate *priv;
    const gchar *ptr;
    gchar **temp_string_array;
    gint total_download_arguments;

		priv = dictionary->priv;
    ptr = lw_dictionary_get_filename (LW_DICTIONARY (dictionary));
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = priv->uri[LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    //Make sure the correct number of download arguments are available
    temp_string_array = g_strsplit (ptr, ";", -1);
    total_download_arguments = g_strv_length (temp_string_array);
    g_strfreev (temp_string_array);

    if (priv->merge && total_download_arguments != 2) return FALSE;
    if (!priv->merge && total_download_arguments != 1) return FALSE;

    ptr = priv->uri[LW_INSTALLDICTIONARY_NEEDS_DECOMPRESSION];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = priv->uri[LW_INSTALLDICTIONARY_NEEDS_TEXT_ENCODING];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = priv->uri[LW_INSTALLDICTIONARY_NEEDS_POSTPROCESSING];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = priv->uri[LW_INSTALLDICTIONARY_NEEDS_FINALIZATION];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = priv->uri[LW_INSTALLDICTIONARY_NEEDS_NOTHING];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    //if (priv->type < 0 || priv->type >= TOTAL_LW_DICTTYPES) return FALSE;
    if (priv->compression < 0 || priv->compression >= LW_COMPRESSION_TOTAL) return FALSE;
    if (priv->encoding < 0 || priv->encoding >= LW_ENCODING_TOTAL) return FALSE;

    return TRUE;
}


//!
//! @brief Downloads or copies the file to the dictionary directory to be worked on
//!        This function should normally only be used in the lw_installdictionary_install method.
//! @param dictionary The LwInstallDictionary object to use to download the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the download is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
gboolean 
lw_installdictionary_download (LwInstallDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
    LwInstallDictionaryPrivate *priv;
    gchar *source;
    gchar *target;
    gint i;
    LwInstallDictionaryUri group_index;

    //Initializations
    priv = dictionary->priv;
    group_index = LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING;
    i = 0;

    if (priv->cancel) return FALSE;

    while ((source = lw_installdictionary_get_source_uri (dictionary, group_index, i)) != NULL &&
           (target = lw_installdictionary_get_target_uri (dictionary, group_index, i)) != NULL
          )
    {
      //File is located locally so copy it
      if (g_file_test (source, G_FILE_TEST_IS_REGULAR))
        lw_io_copy (source, target, (LwIoProgressCallback) cb, data, error);
      //Download the file
      else
        lw_io_download (source, target, cb, data, error);
      i++;
    }

    return (*error == NULL);
}


//!
//! @brief Detects the compression scheme of a file and decompresses it using the approprate function.
//!        This function should normally only be used in the lw_installdictionary_install function.
//! @param dictionary The LwInstallDictionary object to use to decompress the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the decompression is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
gboolean 
lw_installdictionary_decompress (LwInstallDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
		LwInstallDictionaryPrivate *priv;
    gchar *source;
    gchar *target;
    LwInstallDictionaryUri group_index;

    //Initializations
		priv = dictionary->priv;
    group_index = LW_INSTALLDICTIONARY_NEEDS_DECOMPRESSION;
    if (priv->cancel) return FALSE;

    if ((source = lw_installdictionary_get_source_uri (dictionary, group_index, 0)) != NULL &&
        (target = lw_installdictionary_get_target_uri (dictionary, group_index, 0)) != NULL
       )
    {
      //Sanity check
      g_assert (g_file_test (source, G_FILE_TEST_IS_REGULAR));

      //Preform the correct decompression
      switch (priv->compression)
      {
        case LW_COMPRESSION_GZIP:
          lw_io_gunzip_file (source, target, cb, data, error);
          break;
        case LW_COMPRESSION_NONE:
          lw_io_copy (source, target, cb, data, error);
          break;
        default:
          break;
      }

      //If there is another path, it is assumed to be the radicals dictionary
      if ((source = lw_installdictionary_get_source_uri (dictionary, group_index, 1)) != NULL &&
          (target = lw_installdictionary_get_target_uri (dictionary, group_index, 1)) != NULL
         )
      {
        //Sanity check
        g_assert (g_file_test (source, G_FILE_TEST_IS_REGULAR));

        lw_io_gunzip_file (source, target, cb, data, error);
      }
    }

    return (error == NULL && *error == NULL);
}


//!
//! @brief Converts the encoding to UTF8 for the file
//!        This function should normally only be used in the lw_installdictionary_install function.
//! @param dictionary The LwInstallDictionary object to use to convert the text encoding the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the text encoding conversion is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
gboolean 
lw_installdictionary_convert_encoding (LwInstallDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
		LwInstallDictionaryPrivate *priv;
    gchar *source;
    gchar *target;
    const gchar *encoding_name;
    LwInstallDictionaryUri group_index;

    //Initializations
		priv = dictionary->priv;
    group_index = LW_INSTALLDICTIONARY_NEEDS_TEXT_ENCODING;
    encoding_name = lw_util_get_encoding_name (priv->encoding);

    if ((source = lw_installdictionary_get_source_uri (dictionary, group_index, 0)) != NULL &&
        (target = lw_installdictionary_get_target_uri (dictionary, group_index, 0)) != NULL
       )
    {
      if (priv->encoding == LW_ENCODING_UTF8)
        lw_io_copy (source, target, cb, data, error);
      else
        lw_io_copy_with_encoding (source, target, encoding_name, "UTF-8", cb, data, error);

      //If there is another path, it is assumed to be the radicals dictionary
      if ((source = lw_installdictionary_get_source_uri (dictionary, group_index, 1)) != NULL &&
          (target = lw_installdictionary_get_target_uri (dictionary, group_index, 1)) != NULL
         )
        lw_io_copy_with_encoding (source, target, "EUC-JP", "UTF-8", cb, data, error);
    }

    return (*error == NULL);
}


//!
//! @brief does the required postprocessing on a dictionary
//!        This function should normally only be used in the lw_installdictionary_install function.
//! @param dictionary The LwInstallDictionary object to use for postprocessing the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the postprocessing is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
gboolean 
lw_installdictionary_postprocess (LwInstallDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
    gchar *source;
    gchar *source2;
    gchar *target;
    gchar *target2;
		LwInstallDictionaryPrivate *priv;
    LwInstallDictionaryUri group_index;

    //Initializations
		priv = dictionary->priv;
    group_index = LW_INSTALLDICTIONARY_NEEDS_POSTPROCESSING;

    if (priv->cancel) return FALSE;

    //Rebuild the mix dictionary
    if (priv->merge)
    {
      if ((source = lw_installdictionary_get_source_uri (dictionary, group_index, 0)) != NULL &&
          (source2 = lw_installdictionary_get_source_uri (dictionary, group_index, 1)) != NULL &&
          (target = lw_installdictionary_get_target_uri (dictionary, group_index, 0)) != NULL
         )
      {
      lw_io_create_mix_dictionary (target, source, source2, cb, data, error);
      }
    }

    //Rebuild the names dictionary
    else if(priv->split)
    {
      if ((source = lw_installdictionary_get_source_uri (dictionary, group_index, 0)) != NULL &&
          (target = lw_installdictionary_get_target_uri (dictionary, group_index, 0)) != NULL &&
          (target2 = lw_installdictionary_get_target_uri (dictionary, group_index, 1)) != NULL
         )
      lw_io_split_places_from_names_dictionary (target, target2, source, cb, data, error);
    }

    //Just copy the file no postprocessing required
    else
    {
      if ((source = lw_installdictionary_get_source_uri (dictionary, group_index, 0)) != NULL &&
          (target = lw_installdictionary_get_target_uri (dictionary, group_index, 0)) != NULL
         )
      lw_io_copy (source, target, cb, data, error);
    }

    //Finish
    return (*error == NULL);
}


//!
//! @brief does the required postprocessing on a dictionary
//!        This function should normally only be used in the lw_installdictionary_install function.
//! @param dictionary The LwInstallDictionary object to use for finalizing the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the finalization is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
gboolean 
lw_installdictionary_finish (LwInstallDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
    LwInstallDictionaryPrivate *priv;
    gchar *source;
    gchar *target;
    LwInstallDictionaryUri group_index;
    gint i;

    //Initializations
    group_index = LW_INSTALLDICTIONARY_NEEDS_FINALIZATION;
    i = 0;

    if (priv->cancel) return FALSE;

    while ((source = lw_installdictionary_get_source_uri (dictionary, group_index, i)) != NULL &&
           (target = lw_installdictionary_get_target_uri (dictionary, group_index, i)) != NULL
          )
    {
      lw_io_copy (source, target, cb, data, error);
      i++;
    }

    //Finish
    return (*error == NULL);
}


//!
//! @brief removes temporary files created by installation in the dictionary cache folder
//! @param dictionary The LwInstallDictionary object to use to clean the files.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the finalization is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//!
void 
lw_installdictionary_clean (LwInstallDictionary *dictionary, LwIoProgressCallback cb, gpointer data)
{
    //Declarations
		LwInstallDictionaryPrivate *priv;
    LwInstallDictionaryUri group_index;
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
}


//!
//! @brief Installs a LwInstallDictionary object using the provided gui update callback
//!        This function should normally only be used in the lw_installdictionary_install function.
//! @param dictionary The LwInstallDictionary object to use for installing the dictionary with.
//! @param cb A LwIoProgressCallback used to giver user feedback on how far the installation is.
//! @param data A gpointer to data to pass to the LwIoProgressCallback.
//! @param error A pointer to a GError object to pass errors to or NULL.
//! @see lw_installdictionary_download
//! @see lw_installdictionary_convert_encoding
//! @see lw_installdictionary_postprocess
//! @see lw_installdictionary_install
//!
gboolean 
lw_installdictionary_install (LwInstallDictionary *dictionary, LwIoProgressCallback cb, gpointer data, GError **error)
{
    g_assert (*error == NULL && dictionary != NULL);

    lw_installdictionary_download (dictionary, cb, data, error);
    lw_installdictionary_decompress (dictionary, cb, data, error);
    lw_installdictionary_convert_encoding (dictionary, cb, data, error);
    lw_installdictionary_postprocess (dictionary, cb, data, error);
    lw_installdictionary_finish (dictionary, cb, data, error);
    lw_installdictionary_clean (dictionary, cb, data);

    return (*error == NULL);
}


//!
//! @brief Returns a status string describing the current process being taken
//!        on a LwInstallDictionary object
//! @param dictionary A LwInstallDictionary object to get the status of.
//! @param long_form Whether you want the long or short form of the status messages.
//! @returns An allocated string that should be freed with gfree when finished
//!
gchar* 
lw_installdictionary_get_status_string (LwInstallDictionary *dictionary, gboolean long_form)
{
    //Declarations
		LwInstallDictionaryPrivate *priv;
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
          text = g_strdup_printf (gettext("Converting the encoding of %s from %s to UTF-8..."), longname, lw_util_get_encoding_name (priv->encoding));
        else
          text = g_strdup_printf (gettext("Converting the encoding to UTF-8..."));
        break;
      case LW_INSTALLDICTIONARY_NEEDS_DECOMPRESSION:
        if (long_form)
          text = g_strdup_printf (gettext("Decompressing %s from %s file..."), longname, lw_util_get_compression_name (priv->compression));
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

    return text;
}


//!
//! @brief Returns the percent progress as a double for only the current LwInstallDictionary
//! @param dictionary The LwInstallDictionary to get the progress of
//! @param fraction The fraction percentage of the current process
//! @param The fraction percentage of all the LwInstallDictionary processes together.
//!
double lw_installdictionary_get_process_progress (LwInstallDictionary *dictionary, double fraction)
{
    //Declarations
    gdouble current;
    gdouble final;
    gdouble output_fraction;
    gchar *ptr;
		LwInstallDictionaryPrivate *priv;
    
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

    return output_fraction;
}


//!
//! @brief Gets the total possible progress for the LwInstallDictionary.  It should be
//!        divided by the current progress to get the appropriate fraction
//! @param dictionary The LwDictInfo object to get the total progress of
//! @param fraction The fraction progress of the current process.
//!
double 
lw_installdictionary_get_total_progress (LwInstallDictionary *dictionary, double fraction)
{
    //Declarations
		LwInstallDictionaryPrivate *priv;
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

    return output_fraction;
}


//!
//! @brief Gets the uris saved in the LwDictInfo in the form of an array.  It should not be freed.
//! @param dictionary The LwDictInfo object to get the source uri of.
//! @param GROUP_INDEX The group index designates the process step in the install
//! @param ATOM_INDEX The desired atom if there are multiple files being acted upon in a step
//! @returns An allocated string internal to the LwInstallDictionary that should not be freed
//!
gchar* 
lw_installdictionary_get_source_uri (LwInstallDictionary *dictionary, const LwInstallDictionaryUri GROUP_INDEX, const gint ATOM_INDEX)
{
    //Sanity check
    g_assert (GROUP_INDEX >= 0 && GROUP_INDEX < LW_INSTALLDICTIONARY_NEEDS_NOTHING);

    //Declarations
		LwInstallDictionaryPrivate *priv;
    gchar *uri;

		priv = dictionary->priv;

    //Set up the backbone if it isn't already
    if (GROUP_INDEX != priv->uri_group_index)
    {
      if (priv->current_source_uris != NULL) g_strfreev (priv->current_source_uris);
      if (priv->current_target_uris != NULL) g_strfreev (priv->current_target_uris);
      priv->current_source_uris = g_strsplit (priv->uri[GROUP_INDEX], ";", -1);
      priv->current_target_uris = g_strsplit (priv->uri[GROUP_INDEX + 1], ";", -1);
    }

    //Get the information we came here for
    if (ATOM_INDEX >= 0 && ATOM_INDEX < g_strv_length (priv->current_source_uris))
    {
      priv->uri_group_index = GROUP_INDEX;
      priv->progress = 0.0;
      priv->uri_atom_index = ATOM_INDEX;
      uri = priv->current_source_uris[ATOM_INDEX];
    }
    else
    {
      uri = NULL;
    }

    return uri;
}


//!
//! @brief Gets the uris saved in the LwDictInfo in the form of an array.  It should not be freed.
//! @param dictionary The LwDictInfo object to get the source uri of.
//! @param GROUP_INDEX The group index designates the process step in the install
//! @param ATOM_INDEX The desired atom if there are multiple files being acted upon in a step
//! @returns An allocated string internal to the LwInstallDictionary that should not be freed
//!
gchar* 
lw_installdictionary_get_target_uri (LwInstallDictionary *dictionary, const LwInstallDictionaryUri GROUP_INDEX, const gint ATOM_INDEX)
{
    //Sanity check
    g_assert (GROUP_INDEX >= 0 && GROUP_INDEX < LW_INSTALLDICTIONARY_NEEDS_NOTHING);

    //Declarations
		LwInstallDictionaryPrivate *priv;
    gchar *uri;

		priv = dictionary->priv;

    //Set up the backbone if it isn't already
    if (GROUP_INDEX != priv->uri_group_index)
    {
      priv->uri_group_index = GROUP_INDEX;
      //priv->progress = 0.0;
      if (priv->current_source_uris != NULL) g_strfreev (priv->current_source_uris);
      if (priv->current_target_uris != NULL) g_strfreev (priv->current_target_uris);
      priv->current_source_uris = g_strsplit (priv->uri[GROUP_INDEX], ";", -1);
      priv->current_target_uris = g_strsplit (priv->uri[GROUP_INDEX + 1], ";", -1);
    }

    //Get the information we came here for
    if (ATOM_INDEX >= 0 && ATOM_INDEX < g_strv_length (priv->current_target_uris))
    {
      //priv->uri_atom_index = ATOM_INDEX; //Only source sets this variable.  not target.
      uri = priv->current_target_uris[ATOM_INDEX];
    }
    else
    {
      uri = NULL;
    }

    return uri;
}

//!
//! @brief Used to tell the LwInstallDictionary installer to stop installation.
//! @param dictionary The LwInstallDictionary object to stop or prevent the install on.
//! @param state Whether to turn on the requested cancel operation or not.
//!
void 
lw_installdictionary_set_cancel_operations (LwInstallDictionary *dictionary, gboolean state)
{
    LwInstallDictionaryPrivate *priv;

    priv = dictionary->priv;
    priv->cancel = state;
    lw_io_set_cancel_operations (state);
}

