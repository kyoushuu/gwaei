#ifndef LW_DICTINFO_PRIVATE_INCLUDED
#define LW_DICTINFO_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _LwDictInfoPrivate {
    EXTENDS_LW_DICT
    gint load_position;         //!< load position in the GUI
    size_t length;              //!< Length of the file
};

#define LW_DICTINFO_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), LW_TYPE_DICTINFO, LwDictInfoPrivate))

G_END_DECLS

#endif
