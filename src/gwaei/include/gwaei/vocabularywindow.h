#ifndef GW_VOCABULARYWINDOW_INCLUDED
#define GW_VOCABULARYWINDOW_INCLUDED

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwVocabularyWindow GwVocabularyWindow;
typedef struct _GwVocabularyWindowClass GwVocabularyWindowClass;
typedef struct _GwVocabularyWindowPrivate GwVocabularyWindowPrivate;

#define GW_TYPE_VOCABULARYWINDOW              (gw_vocabularywindow_get_type())
#define GW_VOCABULARYWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_VOCABULARYWINDOW, GwVocabularyWindow))
#define GW_VOCABULARYWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_VOCABULARYWINDOW, GwVocabularyWindowClass))
#define GW_IS_VOCABULARYWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_VOCABULARYWINDOW))
#define GW_IS_VOCABULARYWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_VOCABULARYWINDOW))
#define GW_VOCABULARYWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_VOCABULARYWINDOW, GwVocabularyWindowClass))

#define GW_VOCABULARYWINDOW_KEEP_SEARCHING_MAX_DELAY 3

struct _GwVocabularyWindow {
  GwWindow window;
  GwVocabularyWindowPrivate *priv;
};

struct _GwVocabularyWindowClass {
  GwWindowClass parent_class;
};

GtkWindow* gw_vocabularywindow_new (GtkApplication *application);
GType gw_vocabularywindow_get_type (void) G_GNUC_CONST;

#include "vocabularywindow-callbacks.h"

G_END_DECLS

#endif