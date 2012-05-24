#include <string.h>
#include <glib.h>

#include <libwaei/libwaei.h>

GRegex**
lw_regexgroup_new ()
{
    return g_new0 (GRegex*, TOTAL_LW_RELEVANCE);
}


void
lw_regexgroup_free (GRegex **regexgroup)
{
    //Sanity checks
    if (regexgroup == NULL) return;

    //Declarations
    LwRelevance relevance;

    //Initializations
    relevance = 0;

    while (relevance < TOTAL_LW_RELEVANCE)
    {
      if (regexgroup[relevance] != NULL) g_regex_unref (regexgroup[relevance]); 
      relevance++;
    }
    memset(regexgroup, 0, sizeof(GRegex*) * TOTAL_LW_RELEVANCE);

    g_free (regexgroup);
}

