#ifndef GW_MATCHWINDOW_PRIVATE_INCLUDED
#define GW_MATCHWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

typedef enum {
  GW_MATCHWINDOW_TIMEOUTID_UNUSED,
  TOTAL_GW_MATCHWINDOW_TIMEOUTIDS
} GwMatchWindowTimeoutId;

typedef enum {
  GW_MATCHWINDOW_SIGNALID_UNUSED,
  TOTAL_GW_MATCHWINDOW_SIGNALIDS
} GwMatchWindowSignalId;

struct _GwMatchWindowPrivate {
  GtkTreeView *left_treeview;
  GtkTreeView *right_treeview;
  GtkLabel *correct_label;
  GtkLabel *incorrect_label;
  GtkProgressBar *progressbar;
};

#define GW_MATCHWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_MATCHWINDOW, GwMatchWindowPrivate))

G_END_DECLS

#endif
