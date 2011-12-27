#ifndef GW_SPELLCHECK_PRIVATE_INCLUDED
#define GW_SPELLCHECK_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwSpellcheckPrivate {
  GwApplication *application;
  GtkEntry *entry;
  GList *corrections;
  GMutex *mutex;
  GThread *thread;
  GwSpellcheckStatus status;
  int timeout;
  guint threshold;
  guint signalid[TOTAL_GW_SPELLCHECK_SIGNALIDS];
  guint timeoutid[TOTAL_GW_SPELLCHECK_TIMEOUTIDS];
};

#define GW_SPELLCHECK_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_SPELLCHECK, GwSpellcheckPrivate))

G_END_DECLS

#endif

