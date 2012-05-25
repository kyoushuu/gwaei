#ifndef LW_QUERYLINE_INCLUDED
#define LW_QUERYLINE_INCLUDED

#include <libwaei/regexgroup.h>

G_BEGIN_DECLS

#define LW_QUERYLINE(object) (LwQuery*) object


typedef enum {
  LW_QUERY_TYPE_MIX,
  LW_QUERY_TYPE_KANJI,
  LW_QUERY_TYPE_FURIGANA,
  LW_QUERY_TYPE_ROMAJI,
  LW_QUERY_TYPE_STROKES,
  LW_QUERY_TYPE_GRADE,
  LW_QUERY_TYPE_JLPT,
  LW_QUERY_TYPE_FREQUENCY,
  TOTAL_LW_QUERY_TYPES
} LwQueryType;


struct _LwQuery {
    gchar *text;
    gchar **tokenlist;
    GRegex ***regexgroup;
    gboolean parsed;
};
typedef struct _LwQuery LwQuery;


LwQuery* lw_query_new ();
void lw_query_free (LwQuery*);

void lw_query_clean (LwQuery*);
const gchar* lw_query_get_text (LwQuery*);
gboolean lw_query_is_parsed (LwQuery*);

void lw_query_init_regexgroup (LwQuery*);
void lw_query_init_tokens (LwQuery*);

void lw_query_clear (LwQuery*);

void lw_query_tokenlist_append (LwQuery*, LwQueryType, LwRelevance, const gchar*);
gchar* lw_query_get_tokenlist (LwQuery*, LwQueryType, LwRelevance);

G_END_DECLS

#endif
