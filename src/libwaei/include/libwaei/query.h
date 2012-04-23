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
  TOTAL_LW_QUERY_REGEX
} LwQueryRegexType;

typedef enum {
  LW_QUERY_TOKEN_TYPE_ROMAJI,
  LW_QUERY_TOKEN_TYPE_FURIGANA,
  LW_QUERY_TOKEN_TYPE_KANJI,
  TOTAL_LW_QUERY_TOKEN_TYPES
} LwQueryTokenType;


struct _LwQuery {
    gchar *text;
    GList *tokens[TOTAL_LW_QUERY_TOKEN_TYPES];
    LwReGroup *regroup[TOTAL_LW_QUERY_REGEX_TYPES];
    gboolean parsed;
};
typedef struct _LwQuery LwQuery;


LwQuery* lw_query_new ();
void lw_query_free (LwQuery*);
void lw_query_clean (LwQuery*);
const gchar* lw_query_get_text (LwQuery*);
gboolean lw_query_is_parsed (LwQuery*);


G_END_DECLS

#endif
