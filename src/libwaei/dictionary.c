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
//!  @file dictionary.c
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

G_DEFINE_ABSTRACT_TYPE (LwDictionary, lw_dictionary, G_TYPE_OBJECT)

typedef enum
{
  PROP_0,
  PROP_FILENAME
} LwDictionaryProps;


void
lw_dictionary_set_installer (LwDictionary *dictionary,
                             const gchar *FILES,
                             const gchar *DOWNLOADS,
                             const gchar *NAME,
                             const gchar *DESCRIPTION,
                             LwEncoding encoding,
                             gboolean postprocess)
{
    g_return_if_fail (dictionary != NULL);
    g_return_if_fail (FILES != NULL && *FILES != '\0');
    g_return_if_fail (encoding > -1);

    //Declarations
    LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;

    //Initializations
    priv = dictionary->priv;
    if (priv->install != NULL) lw_dictionaryinstall_free (priv->install); 
    priv->install = lw_dictionaryinstall_new ();
    if (priv->install == NULL) return;
    install = priv->install;

    if (FILES != NULL) install->files = g_strdup (FILES);
    if (DOWNLOADS != NULL) install->downloads = g_strdup (DOWNLOADS);
    if (NAME != NULL) install->name = g_strdup (NAME);
    if (DESCRIPTION != NULL) install->description = g_strdup (DESCRIPTION);
    install->encoding = encoding;
    install->postprocess = postprocess;
}


void
lw_dictionary_set_builtin_installer (LwDictionary *dictionary,
                                     const gchar *FILES,
                                     LwPreferences *preferences,
                                     const gchar *KEY,
                                     const gchar *NAME,
                                     const gchar *DESCRIPTION,
                                     LwEncoding encoding,
                                     gboolean postprocess,
                                     gboolean buildin)
{
    g_return_if_fail (dictionary != NULL);
    g_return_if_fail (FILES != NULL);
    g_return_if_fail (preferences != NULL);
    g_return_if_fail (KEY != NULL);
    g_return_if_fail (encoding > -1);

    LwDictionaryPrivate *priv;
    LwDictionaryInstall *install;

    lw_dictionary_set_installer (dictionary, FILES, NULL, NAME, DESCRIPTION, encoding, postprocess);
    if (priv->install == NULL) return;
    install = priv->install;

    install->preferences = preferences;
    install->key = KEY;
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
    if (priv->shortname != NULL) g_free (priv->shortname); priv->shortname = NULL;
    if (priv->longname != NULL) g_free (priv->longname); priv->longname = NULL;

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
        if (priv->shortname != NULL) g_free (priv->shortname);
        priv->shortname = g_strdup (priv->filename);
        if (priv->longname != NULL) g_free (priv->longname);
        priv->longname = g_strdup_printf (gettext("%s Dictionary"), priv->shortname);
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
    gchar *path;

    file = NULL;
    path = lw_dictionary_get_path (dictionary);
    
    if (path != NULL)
    {
      file = fopen (path, "r");
      g_free (path); path = NULL;
    }

    return file;
}


gchar*
lw_dictionary_get_directoryname (GType dictionary_type)
{
    //Declarations
    const gchar* TYPENAME;
    const gchar *start;
    const gchar *end;
    gchar *name;
    gchar *lowercase;
    
    name = NULL;
    lowercase = NULL;
    TYPENAME = g_type_name (dictionary_type);
    start = TYPENAME + strlen("Lw");
    end = TYPENAME + strlen(TYPENAME) - strlen("Dictionary");
    if (strcmp(end, "Dictionary") != 0) goto errored;
    if (start > end) goto errored;

    name = g_strndup (start, end - start);
    if (name == NULL) goto errored;

    lowercase = g_ascii_strdown (name, -1);

    return lowercase;
    
errored:
    if (name != NULL) g_free (name); name = NULL;
    if (lowercase != NULL) g_free (lowercase); lowercase = NULL;

    return NULL;
}


gchar*
lw_dictionary_get_directory (GType dictionary_type)
{
    //Declarations
    gchar *path;
    gchar *directoryname;

    //Initializations
    path = NULL;
    directoryname = lw_dictionary_get_directoryname (dictionary_type);

    if (directoryname != NULL) 
    {
      path = lw_util_build_filename (LW_PATH_DICTIONARY, directoryname);
      g_free (directoryname); directoryname = NULL;
    }
  
    return path;
}


gchar* 
lw_dictionary_get_path (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    gchar *directory;
    const gchar *filename;
    gchar *path;

    //Initializations
    directory = lw_dictionary_get_directory (G_OBJECT_TYPE (dictionary));
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
    if (error != NULL && *error != NULL) return FALSE;

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
lw_dictionary_build_id_from_type (GType type, const gchar *FILENAME)
{
    //Sanity checks
    g_return_val_if_fail (FILENAME != NULL, NULL);

    //Declarations
    gchar *id;
    gchar *directoryname;

    //Initializations
    directoryname = lw_dictionary_get_directoryname (type);
    if (directoryname == NULL) return NULL;
    id = g_strdup_printf ("%s/%s", directoryname, FILENAME);
    g_free (directoryname);

    return id;
}


gchar*
lw_dictionary_directoryname_to_typename (const gchar *DIRECTORYNAME)
{
    //Sanity checks
    g_return_val_if_fail (DIRECTORYNAME != NULL, NULL);

    //Declarations
    gchar *directoryname;
    gchar *typename;

    directoryname = g_strdup (DIRECTORYNAME);
    if (directoryname == NULL || directoryname[0] == '\0') return NULL;
    directoryname[0] = g_ascii_toupper (directoryname[0]);
    typename = g_strdup_printf ("Lw%sDictionary", directoryname);
    g_free (directoryname);

    return typename;
}


gchar*
lw_dictionary_build_id (LwDictionary *dictionary)
{
    //Sanity checks
    g_return_val_if_fail (dictionary != NULL, NULL);

    //Declarations
    gchar *id;
    GType type;
    const gchar *FILENAME;

    //Initializations
    type = G_OBJECT_TYPE (dictionary);
    FILENAME = lw_dictionary_get_filename (dictionary);
    id = lw_dictionary_build_id_from_type (type, FILENAME);

    return id;
}


LwDictionaryState 
lw_dictionary_get_state (LwDictionary *dictionary)
{
    LwDictionaryPrivate *priv;

    priv = dictionary->priv;

    return priv->state;
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


gboolean
lw_dictionary_is_selected (LwDictionary *dictionary)
{
    //Sanity check
    g_return_val_if_fail (dictionary != NULL, FALSE);

    //Declarations
    LwDictionaryPrivate *priv;

    //Initializations
    priv = dictionary->priv;

    return priv->selected;
}


//!
//! @brief Used to tell the LwDictionary installer to stop installation.
//! @param dictionary The LwDictionary object to stop or prevent the install on.
//! @param state Whether to turn on the requested cancel operation or not.
//!
gboolean
lw_dictionary_is_cancelled (LwDictionary *dictionary)
{
    LwDictionaryPrivate *priv;

    priv = dictionary->priv;

    return (priv->cancel);
}


//!
//! @brief Used to tell the LwDictionary installer to stop installation.
//! @param dictionary The LwDictionary object to stop or prevent the install on.
//! @param state Whether to turn on the requested cancel operation or not.
//!
void 
lw_dictionary_cancel (LwDictionary *dictionary)
{
    g_return_if_fail (dictionary != NULL);

    LwDictionaryPrivate *priv;
    LwDictionaryState state;

    priv = dictionary->priv;
    state = lw_dictionary_get_state (dictionary);

    if (state != LW_DICTIONARY_STATE_INSTALLED && state != LW_DICTIONARY_STATE_NOT_INSTALLED)
    {
      priv->cancel = TRUE;
      lw_io_set_cancel_operations (state);
    }
}


gchar**
lw_dictionary_get_installed_idlist (GType type_filter)
{
    //Declarations
    GType* childlist;
    GType *childiter;

    const gchar* filename;
    gchar *directorypath;
    GDir *directory;

    gchar **idlist;
    gchar **iditer;

    gint length;

    childiter = childlist = g_new (GType, 5);
    childlist[0] = lw_edictionary_get_type ();
    childlist[1] = lw_kanjidictionary_get_type ();
    childlist[2] = lw_exampledictionary_get_type ();
    childlist[3] = lw_unknowndictionary_get_type ();
    childlist[4] = 0;

    if (childiter == NULL) return NULL;

    length = 0;

    //Find out how long the array has to be
    while (*childiter != 0)
    {
      directorypath = lw_dictionary_get_directory (*childiter);
printf("directorypath: %s\n", directorypath);
      directory = g_dir_open (directorypath, 0, NULL);
      if (directory != NULL)
      {
        while ((filename = g_dir_read_name (directory)) != NULL)
        {
          length++;
        }
        g_dir_close (directory); directory = NULL;
      }
      g_free (directorypath); directorypath = NULL;
      childiter++;
    }
printf("length %d\n", length);

    iditer = idlist = g_new0 (gchar*, length + 1);
    childiter = childlist;

    //Find out how long the array has to be
    while (*childiter != 0)
    {
      directorypath = lw_dictionary_get_directory (*childiter);
      directory = g_dir_open (directorypath, 0, NULL);
      if (directory != NULL)
      {
        while ((filename = g_dir_read_name (directory)) != NULL && length > 0)
        {
          *iditer = lw_dictionary_build_id_from_type (*childiter, filename);
          printf("id: %s\n", *iditer);
          
          iditer++;
          length--;
        }
        g_dir_close (directory); directory = NULL;
      }
      g_free (directorypath); directorypath = NULL;
      childiter++;
    }

    return idlist;
}


void
lw_dictionary_build_regex (LwDictionary *dictionary, LwQuery *query, GError **error)
{
    //Sanity checks
    g_return_if_fail (dictionary != NULL);
    g_return_if_fail (query != NULL);
    g_return_if_fail (query->regexgroup != NULL);
    g_return_if_fail (query->tokenlist != NULL);
    g_return_if_fail (error != NULL);
    if (error != NULL && *error != NULL) return;

    //Declarations
    LwDictionaryClass *klass;
    gchar *text;
    GRegex *regex;
    GRegex **regexgroup;
    LwRelevance relevance;
    gchar **pattern;
    LwQueryType type;

    //Initializations
    for (type = 0; type < TOTAL_LW_QUERY_TYPES; type++)
    {
      klass = LW_DICTIONARY_CLASS (G_OBJECT_GET_CLASS (dictionary));
      regexgroup = lw_regexgroup_new ();
      pattern = klass->patterns[type];

      if (regexgroup != NULL)
      {
        for (relevance = 0; relevance < TOTAL_LW_RELEVANCE; relevance++)
        {
          text = lw_query_get_tokenlist (query, type, relevance, FALSE);
          if (text != NULL)
          {
            regex = lw_regex_new (pattern[relevance], text, error);
            if (regex != NULL) regexgroup[relevance] = regex;
          }
        }

        query->regexgroup[type] = regexgroup;
      }
    }
}


