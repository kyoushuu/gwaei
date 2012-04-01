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
  gint max;
};
typedef struct _LwDictionaryList LwDictionaryList;


LwDictionaryList* lw_dictionarylist_new (const int);
void lw_dictionarylist_free (LwDictionaryList*);
void lw_dictionarylist_init (LwDictionaryList*, const int);
void lw_dictionarylist_deinit (LwDictionaryList*);

void lw_dictionarylist_add_dictionary (LwDictionaryList*, GType, const gchar*);

LwDictionary* lw_dictionarylist_get_dictionary (LwDictionaryList*, GType, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_by_filename (LwDictionaryList*, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_by_idstring (LwDictionaryList*, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_fuzzy (LwDictionaryList*, const gchar*);
LwDictionary* lw_dictionarylist_get_dictionary_by_load_position (LwDictionaryList*, gint);
gboolean lw_dictionarylist_check_if_loaded (LwDictionaryList*, GType, const gchar*);
void lw_dictionarylist_update_load_orders (LwDictionaryList*);
int lw_dictionarylist_get_total (LwDictionaryList*);
void lw_dictionarylist_clear (LwDictionaryList*);

void lw_dictionarylist_preform_postprocessing_by_name (LwDictionaryList*, gchar*, GError**);
void lw_dictionarylist_load_order (LwDictionaryList*, LwPreferences*);
void lw_dictionarylist_save_order (LwDictionaryList*, LwPreferences*);

void lw_dictionarylist_reload (LwDictionaryList*);
void lw_dictionarylist_sort_and_normalize_order (LwDictionaryList*);

G_END_DECLS


#endif
