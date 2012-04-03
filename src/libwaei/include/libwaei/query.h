#ifndef LW_QUERYLINE_INCLUDED
#define LW_QUERYLINE_INCLUDED

G_BEGIN_DECLS

#define LW_QUERYLINE(object) (LwQuery*) object


typedef enum {
  LW_QUERY_RELEVANCE_UNSET,
  LW_QUERY_RELEVANCE_LOW,
  LW_QUERY_RELEVANCE_MEDIUM,
  LW_QUERY_RELEVANCE_HIGH,
  TOTAL_QUERY_RELEVANCE
} LwQueryRelevance;


typedef enum {
  LW_QUERY_REGEX_KANJI,
  LW_QUERY_REGEX_FURIGANA,
  LW_QUERY_REGEX_ROMAJI,
  LW_QUERY_REGEX_MIX,
  LW_QUERY_REGEX_STROKES,
  LW_QUERY_REGEX_GRADE,
  LW_QUERY_REGEX_JLPT,
  TOTAL_LW_QUERY_REGEX,
} LwQueryRegexType;

struct _LwQuery {
    gchar *text;
    GList *tokens;
    GList **regexlist;
    gboolean parsed;
};
typedef struct _LwQuery LwQuery;


LwQuery* lw_query_new (const gchar*);
void lw_query_free (LwQuery*);
void lw_query_clean (LwQuery*);
const gchar* lw_query_get_text (LwQuery*);
GList* lw_query_get_regexlist (LwQuery*, LwQueryRegexType);
gboolean lw_query_is_parsed (LwQuery*);


struct _LwRegexGroup {
    GRegex* regex[TOTAL_QUERY_RELEVANCE];
};
typedef struct _LwRegexGroup LwRegexGroup;


LwRegexGroup* lw_regex_group_new (void);
GRegex* lw_regex_group_get_regex (LwRegexGroup*, LwRelevance);
void lw_regex_group_free (LwRegexGroup*);


G_END_DECLS

#endif
