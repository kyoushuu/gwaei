#ifndef LW_REGEXGROUP_INCLUDED
#define LW_REGEXGROUP_INCLUDED

G_BEGIN_DECLS

struct _LwRegexGroup {
    GList *high;
    GList *medium;
    GList *low;
};
typedef struct _LwRegexGroup LwRegexGroup;

LwRegexGroup *lw_regexgroup_new ();
void lw_regexgroup_free (LwRegexGroup*);


G_END_DECLS

#endif