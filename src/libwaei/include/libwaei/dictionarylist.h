#ifndef LW_DICTIONARYLIST_INCLUDED
#define LW_DICTIONARYLIST_INCLUDED

#include <libwaei/dictionary.h>

G_BEGIN_DECLS

#define LW_DICTIONARYLIST(object) (LwDictionaryList*) object

//!
//! @brief Primitive for storing lists of dictionaries
//!
struct _LwDictionaryList
{
  GList *list;
  GMutex mutex;
};
typedef struct _LwDictionaryList LwDictionaryList;


LwDictionaryList* lw_dictionarylist_new ();
void lw_dictionarylist_free (LwDictionaryList*);

void lw_dictionarylist_add_dictionary (LwDictionaryList*, LwDictionary*);

LwDictionary* lw_dictionarylist_get_dictionary (LwDictionaryList*, GType, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_by_filename (LwDictionaryList*, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_by_id_string (LwDictionaryList*, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_fuzzy (LwDictionaryList*, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_by_position (LwDictionaryList*, gint);

gint lw_dictionarylist_get_dictionary_position (LwDictionaryList*, LwDictionary*);
gboolean lw_dictionarylist_dictionary_exists (LwDictionaryList*, LwDictionary*);

int lw_dictionarylist_get_total (LwDictionaryList*);

void lw_dictionarylist_load_order (LwDictionaryList*, LwPreferences*);
void lw_dictionarylist_save_order (LwDictionaryList*, LwPreferences*);

void lw_dictionarylist_load_installed (LwDictionaryList*);
void lw_dictionarylist_load_installable (LwDictionaryList*);
void lw_dictionarylist_clear (LwDictionaryList*);


G_END_DECLS


#endif
