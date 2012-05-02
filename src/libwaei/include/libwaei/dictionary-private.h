#ifndef LW_DICTIONARY_PRIVATE_INCLUDED
#define LW_DICTIONARY_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _LwDictionaryPrivate {
    gchar *filename;
    gchar *longname;
    gchar *shortname;
    gchar *description;
    gdouble progress;
    size_t length;       //!< Length of the file
    GMutex mutex;
    LwDictionaryInstall *install;
};

struct _LwDictionaryInstall {
  gchar *uri[LW_INSTALLDICTIONARY_TOTAL_URIS];
  gboolean selected;
  LwPreferences *preferences;
  gchar *schema;
  gchar *key;
  gboolean builtin;
  gulong listenerid;            //!< An id to hold the g_signal_connect value when the source copy uri pref is set
  LwCompression compression;    //!< Path to the gziped dictionary file
  LwEncoding encoding;          //!< Path to the raw unziped dictionary file
  LwInstallDictionaryUri uri_group_index;
  gint uri_atom_index;
  gchar **current_source_uris;
  gchar **current_target_uris;
  gboolean split;
  gboolean merge;
  gboolean cancel;
};

#define LW_DICTIONARY_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), LW_TYPE_DICTIONARY, LwDictionaryPrivate));

G_END_DECLS

#endif
