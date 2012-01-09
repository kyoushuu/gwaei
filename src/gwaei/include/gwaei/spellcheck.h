#ifndef GW_SPELLCHECK_INCLUDED
#define GW_SPELLCHECK_INCLUDED

G_BEGIN_DECLS

typedef enum {
  GW_SPELLCHECKSTATUS_IDLE,
  GW_SPELLCHECKSTATUS_CHECKING,
  GW_SPELLCHECKSTATUS_FINISHING
} GwSpellcheckStatus;

typedef enum {
  GW_SPELLCHECK_SIGNALID_DRAW,
  GW_SPELLCHECK_SIGNALID_CHANGED,
  GW_SPELLCHECK_SIGNALID_POPULATE_POPUP,
  GW_SPELLCHECK_SIGNALID_BUTTON_PRESS_EVENT,
  GW_SPELLCHECK_SIGNALID_DESTROY,
  TOTAL_GW_SPELLCHECK_SIGNALIDS
} GwSpellcheckSignalId;


typedef enum {
  GW_SPELLCHECK_TIMEOUTID_UPDATE,
  TOTAL_GW_SPELLCHECK_TIMEOUTIDS
} GwSpellcheckTimeoutid;


//Boilerplate
typedef struct _GwSpellcheck GwSpellcheck;
typedef struct _GwSpellcheckClass GwSpellcheckClass;
typedef struct _GwSpellcheckPrivate GwSpellcheckPrivate;

#define GW_TYPE_SPELLCHECK              (gw_spellcheck_get_type())
#define GW_SPELLCHECK(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_SPELLCHECK, GwSpellcheck))
#define GW_SPELLCHECK_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass),  GW_TYPE_SPELLCHECK, GwSpellcheckClass))
#define GW_IS_SPELLCHECK(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_SPELLCHECK))
#define GW_IS_SPELLCHECK_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_SPELLCHECK))
#define GW_SPELLCHECK_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj),  GW_TYPE_SPELLCHECK, GwSpellcheckClass))

struct _GwSpellcheck {
  GObject object;
  GwSpellcheckPrivate *priv;
};

struct _GwSpellcheckClass {
  GObjectClass parent_class;
};

//Methods
GwSpellcheck* gw_spellcheck_new (GwApplication*);
GwSpellcheck* gw_spellcheck_new_with_entry (GwApplication*, GtkEntry*);
GType gw_spellcheck_get_type (void) G_GNUC_CONST;

gint gw_spellcheck_get_x_offset (GwSpellcheck *spellcheck);
gint gw_spellcheck_get_y_offset (GwSpellcheck *spellcheck);

struct _SpellingReplacementData {
    GtkEntry *entry;
    int start_offset;
    int end_offset;
    char* replacement_text;
};
typedef struct _SpellingReplacementData _SpellingReplacementData;

struct _GwSpellcheckStreamWithData {
    GwSpellcheck *spellcheck;
    int stream;
    gpointer data;
    int length;
    GPid pid;
};
typedef struct _GwSpellcheckStreamWithData GwSpellcheckStreamWithData;

GwSpellcheckStreamWithData* gw_spellcheck_streamwithdata_new (GwSpellcheck*, int, const char*, int, GPid);
void gw_spellcheck_streamwithdata_free (GwSpellcheckStreamWithData*);
void gw_spellcheck_set_entry (GwSpellcheck*, GtkEntry*);
void gw_spellcheck_start_check (GwSpellcheck*);
GwSpellcheckStatus gw_spellcheck_get_status (GwSpellcheck*);
void gw_spellcheck_set_status (GwSpellcheck*, GwSpellcheckStatus);
void gw_spellcheck_set_timeout_threshold (GwSpellcheck*, guint);
void gw_spellcheck_reset (GwSpellcheck*);
void gw_spellcheck_update_cordinates (GwSpellcheck*, GdkEvent*);
void  gw_spellcheck_populate_popup (GwSpellcheck*, GtkMenu*);

#include <gwaei/spellcheck-callbacks.h>

G_END_DECLS

#endif
