#ifndef LW_DICTIONARY_INCLUDED
#define LW_DICTIONARY_INCLUDED

#include <libwaei/result.h>
#include <libwaei/query.h>

G_BEGIN_DECLS

//Boilerplate
typedef struct _LwDictionary LwDictionary;
typedef struct _LwDictionaryClass LwDictionaryClass;
typedef struct _LwDictionaryPrivate LwDictionaryPrivate;

#define LW_TYPE_DICTIONARY              (lw_dictionary_get_type())
#define LW_DICTIONARY(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), LW_TYPE_DICTIONARY, LwDictionary))
#define LW_DICTIONARY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), LW_TYPE_DICTIONARY, LwDictionaryClass))
#define LW_IS_DICTIONARY(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), LW_TYPE_DICTIONARY))
#define LW_IS_DICTIONARY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), LW_TYPE_DICTIONARY))
#define LW_DICTIONARY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), LW_TYPE_DICTIONARY, LwDictionaryClass))

struct _LwDictionary {
  GObject object;
  LwDictionaryPrivate *priv;
};

struct _LwDictionaryClass {
  GObjectClass parent_class;
  gchar*   (*get_uri) (LwDictionary *dictionary);
  gboolean (*parse_query) (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT);
  gboolean (*parse_result) (LwDictionary *dictionary, LwResult *result, FILE *fd);
  gchar*   (*get_typename) (LwDictionary *dictionary);
};

//Methods
GType lw_dictionary_get_type (void) G_GNUC_CONST;
gboolean lw_dictionary_uninstall (LwDictionary*, LwIoProgressCallback, GError**);
gchar* lw_dictionary_get_uri (LwDictionary*);

gint lw_dictionary_get_load_position (LwDictionary*);
void lw_dictionary_set_load_position (LwDictionary*, gint);

const gchar* lw_dictionary_get_filename (LwDictionary*);
const gchar* lw_dictionary_get_typename (LwDictionary*);
gboolean lw_dictionary_parse_query (LwDictionary*, LwQuery*, const gchar*);
gboolean lw_dictionary_parse_result (LwDictionary*, LwResult*, FILE*);

G_END_DECLS

#endif
