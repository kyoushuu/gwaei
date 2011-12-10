#ifndef GW_MATCHWINDOW_INCLUDED
#define GW_MATCHWINDOW_INCLUDED

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwMatchWindow GwMatchWindow;
typedef struct _GwMatchWindowClass GwMatchWindowClass;
typedef struct _GwMatchWindowPrivate GwMatchWindowPrivate;

#define GW_TYPE_MATCHWINDOW              (gw_matchwindow_get_type())
#define GW_MATCHWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_MATCHWINDOW, GwMatchWindow))
#define GW_MATCHWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_MATCHWINDOW, GwMatchWindowClass))
#define GW_IS_MATCHWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_MATCHWINDOW))
#define GW_IS_MATCHWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_MATCHWINDOW))
#define GW_MATCHWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_MATCHWINDOW, GwMatchWindowClass))

#define GW_MATCHWINDOW_KEEP_SEARCHING_MAX_DELAY 3

struct _GwMatchWindow {
  GwWindow window;
  GwMatchWindowPrivate *priv;
};

struct _GwMatchWindowClass {
  GwWindowClass parent_class;
};

GtkWindow* gw_matchwindow_new (GtkApplication *application);
GType gw_matchwindow_get_type (void) G_GNUC_CONST;

#include "flashcardwindow-callbacks.h"

G_END_DECLS

#endif
