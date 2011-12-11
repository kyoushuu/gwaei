#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gwaei/gwaei.h>
#include <gwaei/flashcardwindow-private.h>

G_MODULE_EXPORT void
gw_flashcardwindow_next_card_cb (GtkWidget *widget, gpointer data)
{
    GwFlashCardWindow *window;

    window = GW_FLASHCARDWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_FLASHCARDWINDOW));
    if (window == NULL) return;
    if (window->priv->model == NULL) return;

    gw_flashcardwindow_iterate (window);
}


G_MODULE_EXPORT void
gw_flashcardwindow_check_answer_cb (GtkWidget *widget, gpointer data)
{
    GwFlashCardWindow *window;

    window = GW_FLASHCARDWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_FLASHCARDWINDOW));
    if (window == NULL) return;
    if (window->priv->model == NULL) return;

    gw_flashcardwindow_check_answer (window);
}


G_MODULE_EXPORT void
gw_flashcardwindow_dont_know_cb (GtkWidget *widget, gpointer data)
{
    GwFlashCardWindow *window;
    GwFlashCardWindowPrivate *priv;

    window = GW_FLASHCARDWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_FLASHCARDWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    if (priv->model == NULL) return;

    gw_flashcardwindow_set_incorrect_guesses (window, ++priv->incorrect_guesses);
    gw_flashcardwindow_load_iterator (window, TRUE, FALSE);
}


G_MODULE_EXPORT gboolean
gw_flashcardwindow_key_press_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GwFlashCardWindow *window;
    GwFlashCardWindowPrivate *priv;
    GdkEventKey *eventkey;
    gboolean sensitive;
    guint keyval;

    window = GW_FLASHCARDWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_FLASHCARDWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;
    if (priv->model == NULL) return FALSE;
    sensitive = gtk_widget_get_sensitive (GTK_WIDGET (priv->answer_entry));
    eventkey = (GdkEventKey*) event;
    keyval = eventkey->keyval;

    if (sensitive == FALSE && keyval == GDK_KEY_Return)
    {
      gw_flashcardwindow_iterate (window);
      return TRUE;
    }

    return FALSE;
}

