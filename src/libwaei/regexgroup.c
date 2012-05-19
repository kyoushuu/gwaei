#include <glib.h>

#include <libwaei/regexgroup.h>

LwRegexGroup*
lw_regexgroup_new ()
{
    LwRegexGroup *temp;

    temp = g_new0 (LwRegexGroup, 1);

    return temp;
}


void
lw_regexgroup_free (LwRegexGroup *regexgroup)
{
    //Sanity checks
    if (regexgroup == NULL) return;

    //Declarations
    GList **relevance;
    GList *link;
    gint i;

    i = 0;

    while (i < TOTAL_QUERY_RELEVANCE)
    {
      relevance = regexgroup->relevance;
      if (relevance[i] != NULL)
      {
        for (link = relevance[i]; link !=NULL; link = link->next)
        {
          if (link->data != NULL) g_regex_unref (link->data);
          link->data = NULL;
        }
        g_list_free (relevance[i]); relevance[i] = NULL;
      }

      i++;
    }

    g_free (regexgroup);
}

