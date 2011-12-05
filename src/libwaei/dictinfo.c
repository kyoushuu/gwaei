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

//Static declarations
static gboolean _overlay_default_builtin_dictionary_settings (LwDictInfo*);

// Use file-based indexes
#define EDICT_IDX_PERSIST 1

// EDICTIDX indexing support functions
static int _dictinfo_index_init(LwDictInfo *di, int persist);
static void _dictinfo_index_deinit(LwDictInfo *di);

//!
//! @brief Creates a new LwDictInfo object
//! @param DICTTYPE The type of LwDictInfo that will be created.  This effects the save location of the dictionary and the parsers used on the dictionary.
//! @param FILENAME The filename of the dictionary.
//! @return An allocated LwDictInfo that will be needed to be freed by lw_dictinfo_free.
//!
LwDictInfo* lw_dictinfo_new (LwDictType DICTTYPE, const char *FILENAME)
{
    //Sanity check
    g_assert (DICTTYPE >= 0 && DICTTYPE <= TOTAL_LW_DICTTYPES && FILENAME != NULL);

    LwDictInfo *temp;
    temp = (LwDictInfo*) malloc(sizeof(LwDictInfo));

    if (temp != NULL)
    {
      lw_dictinfo_init (temp, DICTTYPE, FILENAME);
    }

    //Done
    return temp;
}


//!
//! @brief Releases a LwDictInfo object from memory.
//! @param di A LwDictInfo object created by lw_dictinfo_new.
//!
void lw_dictinfo_free (LwDictInfo* di)
{
    lw_dictinfo_deinit (di);

    free (di);
}


//!
//! @brief Used to initialize the memory inside of a new LwDictInfo
//!        object.  Usually lw_dictinfo_new calls this for you.  It is also 
//!        used in class implimentations that extends LwDictInfo.
//! @param DICTTYPE The dictionary type used for the save folder and parsing engine.
//! @param FILENAME The filename of the dictionary.
//!
void lw_dictinfo_init (LwDictInfo *di, const LwDictType DICTTYPE, const char *FILENAME)
{
    //Declarations
    char *uri;

    //Initializations
    di->load_position = -1;
    di->filename = NULL;
    di->shortname = NULL;
    di->longname = NULL;
    di->type = DICTTYPE;
    di->filename = g_strdup_printf ("%s", FILENAME);

    uri = lw_dictinfo_get_uri (di);
    di->length = lw_io_get_size_for_uri (uri);
    g_free (uri);

    if (!_overlay_default_builtin_dictionary_settings (di))
    {
      di->longname = g_strdup_printf (gettext("%s Dictionary"), FILENAME);
      di->shortname = g_strdup_printf ("%s", FILENAME);
      di->load_position = -1;
    }

    di->cached_resultlines = NULL;
    di->current_resultline = NULL;

    _dictinfo_index_init(di, EDICT_IDX_PERSIST);
}

//!
//! @brief Used to free the memory inside of a LwDictInfo object.
//!         Usually lw_dictinfo_free calls this for you.  It is also used
//!         in class implimentations that extends LwDictInfo.
//! @param di The LwDictInfo object to have it's inner memory freed.
//!
void lw_dictinfo_deinit (LwDictInfo *di)
{
    _dictinfo_index_deinit(di);

    if (di->filename != NULL)
    {
      g_free (di->filename);
      di->filename = NULL;
    }

    if (di->shortname != NULL)
    {
      g_free (di->shortname);
      di->shortname = NULL;
    }

    if (di->longname != NULL)
    {
      g_free (di->longname);
      di->longname = NULL;
    }
}

#ifdef HAVE_EDICTIDX

static
int _build_index(edict_idx* idx, edict_idx_key_types_t key_type,
		 const char* idx_name, const char* dict_name)
{
	if (!idx_name)
		idx_name = "<memory index>";

	if (!idx) {
		fprintf(stderr, "Creating %s for dictionary %s failed\n",
			idx_name, dict_name);
		return -1;
	}

	fprintf(stderr, "Building %s for dictionary %s ...\n",
		idx_name, dict_name);

	if (edict_idx_build(idx, key_type, 0) < 0) {
		fprintf(stderr, "Building %s for dictionary %s failed\n",
			idx_name, dict_name);
		return -1;
	}

	return 0;
}

static
edict_idx* _open_index(edict_idx_key_types_t key_type,
		       const char* idx_fname, const char* dict_fname)
{
	edict_idx* idx;

	if (idx_fname) {
		idx = edict_idx_open(idx_fname, dict_fname, F_EDICT_IDX_OPEN);
		if (idx)
			return idx;

		idx = edict_idx_open(idx_fname, dict_fname, F_EDICT_IDX_CREATE);
	} else {
		idx = edict_idx_open(0, dict_fname,
				     F_EDICT_IDX_CREATE | F_EDICT_IDX_IN_MEMORY);
	}

	if (_build_index(idx, key_type, idx_fname, dict_fname) < 0) {
		if (idx)
			edict_idx_close(idx);
		return 0;
	}

	return idx;
}

static
edict_idx* _share_index(edict_idx_key_types_t key_type,
			const char* idx_fname, const edict_idx* parent)
{
	edict_idx* idx;

	if (idx_fname) {
		idx = edict_idx_share(idx_fname, parent, F_EDICT_IDX_OPEN);
		if (idx)
			return idx;

		idx = edict_idx_share(idx_fname, parent, F_EDICT_IDX_CREATE);
	} else {
		idx = edict_idx_share(0, parent,
				     F_EDICT_IDX_CREATE | F_EDICT_IDX_IN_MEMORY);
	}

	if (_build_index(idx, key_type, idx_fname, "<shared>") < 0) {
		if (idx)
			edict_idx_close(idx);
		    return 0;
	}

	return idx;
}

static int _dictinfo_index_init(LwDictInfo *di, int persist)
{
	char* dict_fname;

	char* kanji_idx_fname = 0;
	char* kana_idx_fname = 0;
	char* english_idx_fname = 0;

	if (di->type != LW_DICTTYPE_EDICT) {
		di->kanji_index = 0;
		di->kana_index = 0;
		di->english_index = 0;
		return 0;
	}

	dict_fname = lw_dictinfo_get_uri (di);

	if (persist) {
		char* kanji_idx_name =
			g_strdup_printf("%s.kanji.idx", di->filename);
		char* kana_idx_name =
			g_strdup_printf("%s.kana.idx", di->filename);
		char* english_idx_name =
			g_strdup_printf("%s.english.idx", di->filename);

		kanji_idx_fname =
			lw_util_build_filename (LW_PATH_CACHE, kanji_idx_name);
		kana_idx_fname =
			lw_util_build_filename (LW_PATH_CACHE, kana_idx_name);
		english_idx_fname =
			lw_util_build_filename (LW_PATH_CACHE, english_idx_name);

		g_free(kanji_idx_name);
		g_free(kana_idx_name);
		g_free(english_idx_name);
	}

	di->kanji_index = _open_index(T_EDICT_IDX_KEY_KANJI, kanji_idx_fname, dict_fname);
	di->kana_index = _share_index(T_EDICT_IDX_KEY_KANA, kana_idx_fname, di->kanji_index);
	di->english_index = _share_index(T_EDICT_IDX_KEY_ENGLISH, english_idx_fname, di->kanji_index);

	g_free(kanji_idx_fname);
	g_free(kana_idx_fname);
	g_free(english_idx_fname);
	g_free(dict_fname);

	return 0;
}

static void _dictinfo_index_deinit(LwDictInfo *di)
{
	if (di->english_index)
		edict_idx_close(di->english_index);
	if (di->kana_index)
		edict_idx_close(di->kana_index);
	if (di->kanji_index)
		edict_idx_close(di->kanji_index);
}

#else /* HAVE_EDICTIDX */

static int _dictinfo_index_init(LwDictInfo *di, int persist) {}
static void _dictinfo_index_deinit(LwDictInfo *di) {}

#endif

//!
//! @brief Function to copy in default values for built-in dictionaries.
//! @param di  The LwDictInfo object to copy the values into.
//! @returns True if you are actually working with a recognized built-in LwDictInfo.
//!
static gboolean _overlay_default_builtin_dictionary_settings (LwDictInfo *di)
{
    g_assert (di != NULL);

    if (di->type == LW_DICTTYPE_EDICT)
    {
      if (strcmp(di->filename, "English") == 0)
      {
        di->longname = g_strdup_printf ("%s", gettext("English Dictionary"));
        di->shortname = g_strdup_printf ("%s", gettext("English"));
        di->load_position = 1;
      }
      else if (strcmp(di->filename, "Names") == 0)
      {
        di->longname = g_strdup_printf ("%s", gettext("Names Dictionary"));
        di->shortname = g_strdup_printf ("%s", gettext("Names"));
        di->load_position = 3;
      }
      else if (strcmp(di->filename, "Places") == 0)
      {
        di->longname = g_strdup_printf ("%s", gettext("Places Dictionary"));
        di->shortname = g_strdup_printf ("%s", gettext("Places"));
        di->load_position = 4;
      }
    }
    else if (di->type == LW_DICTTYPE_KANJI)
    {
      if (strcmp(di->filename, "Kanji") == 0)
      {
        di->longname = g_strdup_printf ("%s", gettext("Kanji Dictionary"));
        di->shortname = g_strdup_printf ("%s", gettext("Kanji"));
        di->load_position = 2;
      }
    }
    else if (di->type == LW_DICTTYPE_EXAMPLES)
    {
      if (strcmp(di->filename, "Examples") == 0)
      {
        di->longname = g_strdup_printf ("%s", gettext("Examples Dictionary"));
        di->shortname = g_strdup_printf ("%s", gettext("Examples"));
        di->load_position = 5;
      }
    }

    return (di->load_position > -1);
}


//!
//! @brief Deletes a LwDictInfo from the harddrive.  LwDictInst objects are used
//!        for installing dictionaries that do not exist yet.  You still need to free
//!        the object after.
//! @param di An LwDictInfo object to get the paths for the dictionary file.
//! @param cb A LwIoProgresSCallback to show dictionary uninstall progress or NULL.
//! @param error A pointer to a GError object to pass errors to or NULL.
//!
gboolean lw_dictinfo_uninstall (LwDictInfo *di, LwIoProgressCallback cb, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;
    g_assert (di != NULL);

    //Declarations
    char *uri;

    //Initializations
    uri =  lw_util_build_filename_by_dicttype (di->type, di->filename);

    lw_io_remove (uri, error);
    if (cb != NULL) cb (1.0, di);

    g_free (uri);

    return (*error == NULL);
}
 

//!
//! @brief Gets the install path of an installed LwDictInfo object.
//! @param di A LwDictInfo object to get the current uri of.
//! @returns An allocated string containing the uri that must be freed with gfree.
//!
char* lw_dictinfo_get_uri (LwDictInfo *di)
{
    //Declarations
    char *path;

    //Initializations
    path = lw_util_build_filename_by_dicttype (di->type, di->filename);

    return path;
}

