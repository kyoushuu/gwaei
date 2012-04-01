#ifndef LW_DICTIONARY_PRIVATE_INCLUDED
#define LW_DICTIONARY_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _LwDictionaryPrivate {
    gchar *filename;
    gchar *longname;
    gchar *shortname;
    gint load_position;  //!< load position in the GUI
    size_t length;       //!< Length of the file
};

#define LW_DICTIONARY_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), LW_TYPE_DICTIONARY, LwDictionaryPrivate));

G_END_DECLS

#endif
