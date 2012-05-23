#ifndef LW_QUERYLINE_INCLUDED
#define LW_QUERYLINE_INCLUDED

#include <libwaei/regexgroup.h>

G_BEGIN_DECLS

#define LW_QUERYLINE(object) (LwQuery*) object


typedef enum {
  LW_REGEX_TYPE_KANJI,
  LW_REGEX_TYPE_FURIGANA,
  LW_REGEX_TYPE_ROMAJI,
  LW_REGEX_TYPE_MIX,
  LW_REGEX_TYPE_STROKES,
  LW_REGEX_TYPE_GRADE,
  LW_REGEX_TYPE_JLPT,
  TOTAL_LW_REGEX_TYPES
} LwRegexType;

typedef enum {
  LW_QUERY_TOKEN_TYPE_ROMAJI,
  LW_QUERY_TOKEN_TYPE_FURIGANA,
  LW_QUERY_TOKEN_TYPE_KANJI,
  TOTAL_LW_QUERY_TOKEN_TYPES
} LwQueryTokenType;


struct _LwQuery {
    gchar *text;
    GString *tokenlist;
    GString *supplimenttokenlist;
    LwRegexGroup **regexgroup;
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

G_END_DECLS

#endif
