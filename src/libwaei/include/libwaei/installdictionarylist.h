#ifndef LW_INSTALLDICTIONARYLIST_INCLUDED
#define LW_INSTALLDICTIONARYLIST_INCLUDED

G_BEGIN_DECLS

#define LW_INSTALLDICTIONARYLIST(object) (LwInstallDictionaryList*) object

struct _LwInstallDictionaryList {
  GList *list;
  gboolean cancel;
};
typedef struct _LwInstallDictionaryList LwInstallDictionaryList;


LwInstallDictionaryList* lw_installdictionarylist_new (LwPreferences *pm);
void lw_installdictionarylist_free (LwInstallDictionaryList*);

gboolean lw_installdictionarylist_data_is_valid (LwInstallDictionaryList*);
LwInstallDictionary* lw_installdictionarylist_get_dictinst_fuzzy (LwInstallDictionaryList*, const char*);
LwInstallDictionary* lw_installdictionarylist_get_dictinst_by_idstring (LwInstallDictionaryList*, const char*);
LwInstallDictionary* lw_installdictionarylist_get_dictinst_by_filename (LwInstallDictionaryList*, const char*);
void lw_installdictionarylist_set_cancel_operations (LwInstallDictionaryList*, gboolean);

G_END_DECLS

#endif
