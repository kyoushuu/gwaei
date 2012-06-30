#ifndef LW_QUERYLINE_INCLUDED
#define LW_QUERYLINE_INCLUDED

#include <libwaei/regexgroup.h>
#include <libwaei/range.h>

G_BEGIN_DECLS

#define LW_QUERY(object) (LwQuery*) object
#define LW_QUERY_DELIMITOR_CHARACTER '&'
#define LW_QUERY_DELIMITOR_STRING "&"

typedef enum {
  LW_QUERY_TYPE_MIX,
  LW_QUERY_TYPE_KANJI,
  LW_QUERY_TYPE_FURIGANA,
  LW_QUERY_TYPE_ROMAJI,
  TOTAL_LW_QUERY_TYPES
} LwQueryType;

typedef enum {
  LW_QUERY_RANGE_TYPE_STROKES,
  LW_QUERY_RANGE_TYPE_GRADE,
  LW_QUERY_RANGE_TYPE_JLPT,
  LW_QUERY_RANGE_TYPE_FREQUENCY,
  TOTAL_LW_QUERY_RANGE_TYPES
} LwQueryRangeType;

struct _LwQuery {
    gchar *text;
    gchar **tokenlist;
    GList ***regexgroup;
    LwRange **rangelist;
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
void lw_query_init_rangelist (LwQuery*);

void lw_query_clear (LwQuery*);

void lw_query_tokenlist_append (LwQuery*, LwQueryType, LwRelevance, gboolean, const gchar*);
gchar** lw_query_get_tokenlist (LwQuery*, LwQueryType, LwRelevance, gboolean);

void lw_query_rangelist_set (LwQuery*, LwQueryRangeType, LwRange*);
LwRange* lw_query_rangelist_get (LwQuery*, LwQueryRangeType);

GList* lw_query_regexgroup_get_list (LwQuery*, LwQueryType, LwRelevance);
void lw_query_regexgroup_append (LwQuery*, LwQueryType, LwRelevance, GRegex*);

G_END_DECLS

#endif
