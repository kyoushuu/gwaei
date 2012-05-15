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
    GList *link;

    if (regexgroup->high != NULL)
    {
      for (link = regexgroup->high; link !=NULL; link = link->next)
      {
        if (link->data != NULL) g_regex_unref (link->data);
        link->data = NULL;
      }
      g_list_free (regexgroup->high); regexgroup->high = NULL;
    }

    if (regexgroup->medium != NULL)
    {
      for (link = regexgroup->medium; link !=NULL; link = link->next)
      {
        if (link->data != NULL) g_regex_unref (link->data);
        link->data = NULL;
      }
      g_list_free (regexgroup->medium); regexgroup->medium = NULL;
    }

    if (regexgroup->low != NULL)
    {
      for (link = regexgroup->low; link !=NULL; link = link->next)
      {
        if (link->data != NULL) g_regex_unref (link->data);
        link->data = NULL;
      }
      g_list_free (regexgroup->low); regexgroup->low = NULL;
    }



    g_free (regexgroup);
}

