#ifndef LW_DICTIONARY_INCLUDED
#define LW_DICTIONARY_INCLUDED

#include <stdio.h>
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
  gboolean (*parse_query) (LwDictionary *dictionary, LwQuery *query, const gchar *TEXT, GError **error);
  gboolean (*parse_result) (LwDictionary *dictionary, LwResult *result, FILE *fd);
  const gchar* (*get_typename) (LwDictionary *dictionary);
  gboolean (*compare) (LwDictionary *dictionary, LwQuery *query, LwResult *result, const LwRelevance relevance);
};

//Methods
GType lw_dictionary_get_type (void) G_GNUC_CONST;
gboolean lw_dictionary_uninstall (LwDictionary*, LwIoProgressCallback, GError**);
gchar* lw_dictionary_get_uri (LwDictionary*);
gboolean lw_dictionary_compare (LwDictionary*, LwQuery*, LwResult*, const LwRelevance);

FILE* lw_dictionary_open (LwDictionary*);

const gchar* lw_dictionary_get_filename (LwDictionary*);
const gchar* lw_dictionary_get_typename (LwDictionary*);

gboolean lw_dictionary_parse_query (LwDictionary*, LwQuery*, const gchar*, GError**);
gboolean lw_dictionary_parse_result (LwDictionary*, LwResult*, FILE*);
size_t lw_dictionary_get_length (LwDictionary*);

gboolean lw_dictionary_equals (LwDictionary*, LwDictionary*);
gchar* lw_dictionary_build_description (LwDictionary*);

G_END_DECLS

#endif
