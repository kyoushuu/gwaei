#ifndef LW_INSTALLDICTIONARY_PRIVATE_INCLUDED
#define LW_INSTALLDICTIONARY_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _LwInstallDictionaryPrivate {
  gchar *filename; 
  gchar *longname; 
  gchar *shortname;
  gchar *description;
  gchar *uri[LW_INSTALLDICTIONARY_TOTAL_URIS];
  gdouble progress;
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
  GMutex mutex;
}

#define LW_INSTALLDICTIONARY_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), LW_TYPE_INSTALLDICTIONARY, LwInstallDictionaryPrivate));

G_END_DECLS

#endif
