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

    gw_flashcardwindow_increment_incorrect_guesses (window);
    gw_flashcardwindow_load_iterator (window, TRUE, FALSE);
}


G_MODULE_EXPORT gboolean
gw_flashcardwindow_key_press_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GwFlashCardWindow *window;
    GwFlashCardWindowPrivate *priv;
    GdkEventKey *eventkey;
    gboolean sensitive;
    gboolean visible;
    guint keyval;
    gint modifier;

    window = GW_FLASHCARDWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_FLASHCARDWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;
    if (priv->model == NULL) return FALSE;
    sensitive = gtk_widget_get_sensitive (GTK_WIDGET (priv->answer_entry));
    visible = gtk_widget_get_visible (GTK_WIDGET (priv->content_box));
    eventkey = (GdkEventKey*) event;
    modifier = eventkey->state;
    keyval = eventkey->keyval;

    if ((modifier & GDK_CONTROL_MASK) != 0 && keyval == GDK_KEY_w)
    {
      gw_flashcardwindow_close_cb (widget, data);
    }
    else if (visible && !sensitive && keyval == GDK_KEY_Return)
    {
      gw_flashcardwindow_iterate (window);
      return TRUE;
    }
    else if (!visible && keyval == GDK_KEY_Return)
    {
      gw_flashcardwindow_close_cb (widget, data);
    }

    return FALSE;
}


G_MODULE_EXPORT void
gw_flashcardwindow_close_cb (GtkWidget *widget, gpointer data)
{
    GwFlashCardWindow *window;
    GwApplication *application;

    window = GW_FLASHCARDWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_FLASHCARDWINDOW));
    if (window == NULL) return;
    application = gw_window_get_application (GW_WINDOW (window));

    gtk_widget_destroy (GTK_WIDGET (window));

    if (gw_application_should_quit (application))
      gw_application_quit (application);
}


G_MODULE_EXPORT void
gw_flashcardwindow_track_results_toggled_cb (GtkWidget *widget, gpointer data)
{
    GwFlashCardWindow *window;
    gboolean state;

    window = GW_FLASHCARDWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_FLASHCARDWINDOW));
    if (window == NULL) return;
    state = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    gw_flashcardwindow_set_track_results (window, state);
}
