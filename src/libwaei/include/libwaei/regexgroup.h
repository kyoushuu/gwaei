#ifndef LW_REGEXGROUP_INCLUDED
#define LW_REGEXGROUP_INCLUDED

G_BEGIN_DECLS

typedef enum {
  LW_QUERY_RELEVANCE_LOW,
  LW_QUERY_RELEVANCE_MEDIUM,
  LW_QUERY_RELEVANCE_HIGH,
  TOTAL_QUERY_RELEVANCE
} LwQueryRelevance;

struct _LwRegexGroup {
    GList *relevance[TOTAL_QUERY_RELEVANCE];
};
typedef struct _LwRegexGroup LwRegexGroup;

LwRegexGroup *lw_regexgroup_new ();
void lw_regexgroup_free (LwRegexGroup*);


G_END_DECLS

#endif
