#ifndef GW_SPELLCHECK_PRIVATE_INCLUDED
#define GW_SPELLCHECK_PRIVATE_INCLUDED

#include <enchant/enchant.h>

G_BEGIN_DECLS

struct _GwSpellcheckPrivate {
  GwApplication *application;

  GtkEntry *entry;

  EnchantBroker *broker;
  EnchantDict *dictionary;

  gchar** tolkens;           //A list of tolkens taken from the search entry
  GList *misspelled; //gchar* pointers to individual tolkens above

  guint timeout, threshold;  //Timer variables to prevent immediate spellchecks
  gint x, y; //mouse position
  guint signalid[TOTAL_GW_SPELLCHECK_SIGNALIDS];
  guint timeoutid[TOTAL_GW_SPELLCHECK_TIMEOUTIDS];
};

#define GW_SPELLCHECK_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_SPELLCHECK, GwSpellcheckPrivate))

G_END_DECLS

#endif

