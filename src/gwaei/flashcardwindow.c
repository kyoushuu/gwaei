/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file flashcardwindow.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/flashcardwindow-private.h>


//Static declarations
static void gw_flashcardwindow_attach_signals (GwFlashCardWindow*);
//static void gw_flashcardwindow_remove_signals (GwFlashCardWindow*);
static gboolean gw_flashcardwindow_finalize_inner_path (GtkTreeModel*, GtkTreePath*, GtkTreeIter*, gpointer);

enum {
  COLUMN_QUESTION,
  COLUMN_ANSWER,
  COLUMN_IS_COMPLETED,
  COLUMN_TREE_PATH,
  TOTAL_COLUMNS
};

static void gw_flashcardwindow_init_accelerators (GwFlashCardWindow*);

G_DEFINE_TYPE (GwFlashCardWindow, gw_flashcardwindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_flashcardwindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwFlashCardWindow *window;

    //Initializations
    window = GW_FLASHCARDWINDOW (g_object_new (GW_TYPE_FLASHCARDWINDOW,
                                            "type",        GTK_WINDOW_TOPLEVEL,
                                            "application", GW_APPLICATION (application),
                                            "ui-xml",      "flashcardwindow.ui",
                                            NULL));

    return GTK_WINDOW (window);
}


static void 
gw_flashcardwindow_init (GwFlashCardWindow *window)
{
    window->priv = GW_FLASHCARDWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwFlashCardWindowPrivate));

/*
    GwFlashCardWindowPrivate *priv;
    priv = window->priv;
*/
}


static void 
gw_flashcardwindow_finalize (GObject *object)
{
    GwFlashCardWindow *window;
    GwFlashCardWindowPrivate *priv;

    window = GW_FLASHCARDWINDOW (object);
    priv = window->priv;

    if (priv->list_name != NULL) g_free (priv->list_name); priv->list_name = NULL;
    if (priv->question_title != NULL) g_free (priv->question_title); priv->question_title = NULL;
    if (priv->question != NULL) g_free (priv->question); priv->question = NULL;
    if (priv->answer != NULL) g_free (priv->answer); priv->answer = NULL;

    gtk_tree_model_foreach (priv->model, gw_flashcardwindow_finalize_inner_path, NULL);

    gw_window_save_size (GW_WINDOW (window));

    G_OBJECT_CLASS (gw_flashcardwindow_parent_class)->finalize (object);
}


static void 
gw_flashcardwindow_constructed (GObject *object)
{
    //Declarations
    GwFlashCardWindow *window;
    GwFlashCardWindowPrivate *priv;
    GtkStyleContext *context;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_flashcardwindow_parent_class)->constructed (object);
    }

    //Initializations
    window = GW_FLASHCARDWINDOW (object);
    priv = window->priv;

    //Set up the gtkbuilder links
    priv->content_box = GTK_BOX (gw_window_get_object (GW_WINDOW (window), "content_box"));
    priv->finished_box = GTK_BOX (gw_window_get_object (GW_WINDOW (window), "finished_box"));

    priv->card_toolbar = GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "card_toolbar"));
    priv->card_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "card_label"));
    priv->card_scrolledwindow = GTK_SCROLLED_WINDOW (gw_window_get_object (GW_WINDOW (window), "card_scrolledwindow"));
    priv->check_answer_toolbutton = GTK_TOOL_BUTTON (gw_window_get_object (GW_WINDOW (window), "submit_toolbutton"));
    priv->next_card_toolbutton = GTK_TOOL_BUTTON (gw_window_get_object (GW_WINDOW (window), "next_toolbutton"));
    priv->dont_know_toolbutton = GTK_TOOL_BUTTON (gw_window_get_object (GW_WINDOW (window), "dont_know_toolbutton"));
    priv->answer_entry = GTK_ENTRY (gw_window_get_object (GW_WINDOW (window), "submit_entry"));
    priv->correct_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "right_label"));
    priv->incorrect_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "wrong_label"));
    priv->status_progressbar = GTK_PROGRESS_BAR (gw_window_get_object (GW_WINDOW (window), "status_progressbar"));
    priv->status_label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "status_label"));

    //Set up the gtk window
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
    gtk_window_set_default_size (GTK_WINDOW (window), 450, 300);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_has_resize_grip (GTK_WINDOW (window), FALSE);
    gw_window_set_is_important (GW_WINDOW (window), TRUE);
    gw_window_load_size (GW_WINDOW (window));

    context = gtk_widget_get_style_context (GTK_WIDGET (priv->card_toolbar));
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_TOP);
    gtk_style_context_add_class (context, "primary-toolbar");
    gtk_widget_reset_style (GTK_WIDGET (priv->card_toolbar));

    context = gtk_widget_get_style_context (GTK_WIDGET (priv->card_scrolledwindow));
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);

    gw_flashcardwindow_init_accelerators (window);
    gw_flashcardwindow_attach_signals (window);

    priv->time = g_get_monotonic_time ();
}


static void
gw_flashcardwindow_class_init (GwFlashCardWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gw_flashcardwindow_constructed;
  object_class->finalize = gw_flashcardwindow_finalize;

  g_type_class_add_private (object_class, sizeof (GwFlashCardWindowPrivate));
}


static void
gw_flashcardwindow_init_accelerators (GwFlashCardWindow *window)
{
/*
    GtkWidget *widget;
    GtkAccelGroup *accelgroup;

    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));
    //Set menu accelerators
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "new_window_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_N), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
*/
}


static void
gw_flashcardwindow_attach_signals (GwFlashCardWindow *window)
{
    g_signal_connect (G_OBJECT (window), "key-press-event", G_CALLBACK (gw_flashcardwindow_key_press_event_cb), window);
}

/*
static void 
gw_flashcardwindow_remove_signals (GwFlashCardWindow *window)
{
}
*/


void
gw_flashcardwindow_update_title (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    gchar *title;

    priv = window->priv;
    title = g_strdup_printf (gettext("%s - gWaei %s Flashcards"), priv->list_name, priv->flash_cards_type);
    if (title != NULL)
    {
      gtk_window_set_title (GTK_WINDOW (window), title);
      g_free (title);
    }
}


gboolean
gw_flashcardwindow_set_model (GwFlashCardWindow *window, 
                              GtkTreeModel      *model, 
                              const gchar       *flash_cards_type,
                              const gchar       *list_name,
                              const gchar       *question_title,
                              gint               question_column,
                              gint               answer_column,
                              gboolean           randomize      )
{
    if (model == NULL) return FALSE;

    GwFlashCardWindowPrivate *priv;
    GRand *random_generator;
    GtkTreeIter source_iter, target_iter;
    GtkTreePath *path;
    gchar *answer, *question;
    gboolean valid;
    gint position;

    priv = window->priv;
    random_generator = g_rand_new ();
    if (priv->model != NULL) g_object_unref (model);
    priv->model = GTK_TREE_MODEL (gtk_list_store_new (
        TOTAL_COLUMNS, 
        G_TYPE_STRING,  //COLUMN_QUESTION
        G_TYPE_STRING,  //COLUMN_ANSWER
        G_TYPE_BOOLEAN, //COLUMN_IS_COMPLETED
        G_TYPE_POINTER  //COLUMN_TREE_PATH
    ));

    valid = gtk_tree_model_get_iter_first (model, &source_iter);

    priv->source_question_column = question_column;
    priv->source_answer_column = answer_column;
    priv->source_model = model;
    priv->list_name = g_strdup (list_name);
    priv->question_title = g_strdup (question_title);
    priv->flash_cards_type = g_strdup (flash_cards_type);

    gw_flashcardwindow_update_title (window);

    while (valid)
    {
      gtk_tree_model_get (model, &source_iter, question_column, &question, answer_column, &answer, -1);
      path = gtk_tree_model_get_path (model, &source_iter);
      if (question != NULL && strlen (question) && answer != NULL && strlen (answer))
      {
        if (randomize)
        {
          if (priv->total_cards == 0)
            position = 0;
          else
            position = g_rand_int_range (random_generator, 0, priv->total_cards);
          gtk_list_store_insert (GTK_LIST_STORE (priv->model), &target_iter, position);
        }
        else
        {
          gtk_list_store_append (GTK_LIST_STORE (priv->model), &target_iter);
        }
        gtk_list_store_set (GTK_LIST_STORE (priv->model), &target_iter,
            COLUMN_QUESTION, question,
            COLUMN_ANSWER, answer,
            COLUMN_TREE_PATH, path,
            COLUMN_IS_COMPLETED, FALSE,
            -1);
        priv->total_cards++;
      }
      if (question != NULL) g_free (question); question = NULL;
      if (answer != NULL) g_free (answer); answer = NULL;
      valid = gtk_tree_model_iter_next (model, &source_iter);
    }

    if (random_generator != NULL) g_rand_free (random_generator);

    if (priv->total_cards == 0)
    {
      g_object_unref (priv->model); priv->model = NULL;
      return FALSE;
    }

    gtk_tree_model_get_iter_first (priv->model, &(priv->iter));
    priv->cards_left = priv->total_cards;
    priv->correct_guesses = -1;
    priv->incorrect_guesses = -1;
    gw_flashcardwindow_increment_correct_guesses (window);
    gw_flashcardwindow_increment_incorrect_guesses (window);
    gw_flashcardwindow_update_progress (window);

    gw_flashcardwindow_load_iterator (window, FALSE, FALSE);

    return TRUE;
}


gboolean
gw_flashcardwindow_user_answer_is_correct (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    const gchar *user_answer;

    priv = window->priv;
    user_answer = gtk_entry_get_text (priv->answer_entry);

    return (strlen (user_answer) > 0 && strstr (priv->answer, user_answer) != NULL);
}


void
gw_flashcardwindow_load_iterator (GwFlashCardWindow *window, gboolean show_answer, gboolean answer_correct)
{
    GwFlashCardWindowPrivate *priv;
    gchar *markup;

    priv = window->priv;

    if (priv->model == NULL) return;

    if (priv->question != NULL) g_free (priv->question);
    if (priv->answer != NULL) g_free (priv->answer);

    if (show_answer)
    {
      gtk_widget_set_sensitive (GTK_WIDGET (priv->answer_entry), !show_answer);
      gtk_widget_set_sensitive (GTK_WIDGET (priv->check_answer_toolbutton), !show_answer);
      gtk_widget_set_sensitive (GTK_WIDGET (priv->dont_know_toolbutton), !show_answer);
    }
    else
    {
      gtk_widget_set_sensitive (GTK_WIDGET (priv->answer_entry), !show_answer);
      gtk_widget_set_sensitive (GTK_WIDGET (priv->check_answer_toolbutton), !show_answer);
      gtk_widget_set_sensitive (GTK_WIDGET (priv->dont_know_toolbutton), !show_answer);
      gtk_entry_set_text (priv->answer_entry, "");
      gtk_widget_grab_focus (GTK_WIDGET (priv->answer_entry));
    }

    gtk_tree_model_get (priv->model, &(priv->iter), COLUMN_QUESTION, &priv->question, COLUMN_ANSWER, &priv->answer, -1);

    if (priv->question != NULL && priv->answer != NULL)
    {
      if (show_answer && answer_correct)
        markup = g_markup_printf_escaped (
          "<big><b>%s</b>\n%s"
          "<small>\n\n</small>"
          "  <span foreground=\"green\" size=\"larger\" weight=\"semibold\">%s</span>"
          "<small>\n</small>"
          "  <span size=\"smaller\" weight=\"semibold\">%s</span>"
          "<small>\n\n</small>"
          "<b>%s:</b>\n%s</big>\n",
          priv->question_title, priv->question, 
          gettext("You were correct!"),
          gettext("Press [Enter] to continue..."),
          gettext("Answer"), priv->answer
        );
      else if (show_answer && !answer_correct)
        markup = g_markup_printf_escaped (
          "<big><b>%s</b>\n%s"
          "<small>\n\n</small>"
          "  <span foreground=\"red\" size=\"larger\" weight=\"semibold\">%s</span>"
          "<small>\n</small>"
          "  <span size=\"smaller\" weight=\"semibold\">%s</span>"
          "<small>\n\n</small>"
          "<b>%s:</b>\n%s</big>\n",
          priv->question_title, priv->question, 
          gettext("You were incorrect!"),
          gettext("Press [Enter] to continue..."),
          gettext("Answer"), priv->answer
        );
      else
        markup = g_markup_printf_escaped ("<big><b>%s</b>\n%s</big>\n\n\n\n\n\n\n\n\n", 
            priv->question_title, priv->question
        );
      if (markup != NULL)
      {
        gtk_label_set_markup (priv->card_label, markup);
        g_free (markup);
      }
    }
}


gboolean
gw_flashcardwindow_iterate (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    gboolean valid;
    gboolean completed;

    priv = window->priv;

    if (priv->model == NULL) return FALSE;

    g_assert (priv->total_cards != 0);

    if (priv->cards_left == 0)
    {
      gw_flashcardwindow_set_finished (window);
      return FALSE;
    }

    valid = TRUE;
    completed = TRUE;

    while (valid && completed)
    {
      valid = gtk_tree_model_iter_next (priv->model, &(priv->iter));
      if (!valid) valid = gtk_tree_model_get_iter_first (priv->model, &(priv->iter));
      gtk_tree_model_get (priv->model, &(priv->iter), COLUMN_IS_COMPLETED, &completed, -1);
    }

    gw_flashcardwindow_load_iterator (window, FALSE, FALSE);

    return (priv->cards_left > 0);
}


void
gw_flashcardwindow_set_card_completed (GwFlashCardWindow *window, gboolean completed)
{
    GwFlashCardWindowPrivate *priv;
    priv = window->priv;

    if (priv->model == NULL) return;

    gtk_list_store_set (GTK_LIST_STORE (priv->model), &(priv->iter), COLUMN_IS_COMPLETED, completed, -1);
}



static void
gw_flashcardwindow_increment_source_incorrect_guesses (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    GtkTreePath *path;
    GtkTreeIter iter;
    gboolean valid;
    gint incorrect_guesses;
    gchar *source_answer, *source_question;

    priv = window->priv;

    gtk_tree_model_get (priv->model, &(priv->iter), COLUMN_TREE_PATH, &path, -1);

    if (path == NULL) return;

    valid = gtk_tree_model_get_iter (priv->source_model, &iter, path);
    if (valid)
    {
        gtk_tree_model_get (priv->source_model, &iter, 
          priv->source_answer_column, &source_answer,
          priv->source_question_column, &source_question, 
          -1);
        incorrect_guesses = gw_vocabularywordstore_get_incorrect_guesses_by_iter (GW_VOCABULARYWORDSTORE (priv->source_model), &iter);

        if (source_answer != NULL && strcmp(priv->answer, source_answer) == 0 && 
            source_question != NULL && strcmp(priv->question, source_question) == 0)
        {
          incorrect_guesses++;
          gw_vocabularywordstore_set_incorrect_guesses_by_iter (GW_VOCABULARYWORDSTORE (priv->source_model), &iter, incorrect_guesses);
          gw_vocabularywordstore_set_has_changes (GW_VOCABULARYWORDSTORE (priv->source_model), TRUE);
          gw_vocabularywordstore_save (GW_VOCABULARYWORDSTORE (priv->source_model), NULL);
        }

        if (source_answer != NULL) g_free (source_answer);
        if (source_question != NULL) g_free (source_question);
    }
}


void
gw_flashcardwindow_increment_incorrect_guesses (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    gchar *text;

    priv = window->priv;
    priv->incorrect_guesses++;
    text = g_strdup_printf ("%d", priv->incorrect_guesses);
    if (text != NULL)
    {
      gtk_label_set_text (priv->incorrect_label, text);
      g_free (text);
    }

    if (priv->incorrect_guesses > 0) 
      gw_flashcardwindow_increment_source_incorrect_guesses (window);
}


static void
gw_flashcardwindow_increment_source_correct_guesses (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    GtkTreePath *path;
    GtkTreeIter iter;
    gboolean valid;
    gint correct_guesses;
    gchar *source_answer, *source_question;

    priv = window->priv;

    gtk_tree_model_get (priv->model, &(priv->iter), COLUMN_TREE_PATH, &path, -1);

    if (path == NULL) return;

    valid = gtk_tree_model_get_iter (priv->source_model, &iter, path);
    if (valid)
    {
        gtk_tree_model_get (priv->source_model, &iter, 
          priv->source_answer_column, &source_answer,
          priv->source_question_column, &source_question, 
          -1);
        correct_guesses = gw_vocabularywordstore_get_correct_guesses_by_iter (GW_VOCABULARYWORDSTORE (priv->source_model), &iter);

        if (source_answer != NULL && strcmp(priv->answer, source_answer) == 0 && 
            source_question != NULL && strcmp(priv->question, source_question) == 0)
        {
          correct_guesses++;
          gw_vocabularywordstore_set_correct_guesses_by_iter (GW_VOCABULARYWORDSTORE (priv->source_model), &iter, correct_guesses);
          gw_vocabularywordstore_set_has_changes (GW_VOCABULARYWORDSTORE (priv->source_model), TRUE);
          gw_vocabularywordstore_save (GW_VOCABULARYWORDSTORE (priv->source_model), NULL);
        }

        if (source_answer != NULL) g_free (source_answer);
        if (source_question != NULL) g_free (source_question);
    }
}


void
gw_flashcardwindow_increment_correct_guesses (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    gchar *text;

    priv = window->priv;
    priv->correct_guesses++;
    text = g_strdup_printf ("%d", priv->correct_guesses);
    if (text != NULL)
    {
      gtk_label_set_text (priv->correct_label, text);
      g_free (text);
    }

    if (priv->correct_guesses > 0) 
      gw_flashcardwindow_increment_source_correct_guesses (window);
}


void
gw_flashcardwindow_update_progress (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    gchar *text;
    gdouble fraction;

    priv = window->priv;
    fraction = (gdouble) (priv->total_cards - priv->cards_left) / (gdouble) priv->total_cards;
    text = g_strdup_printf ("%d/%d", priv->cards_left, priv->total_cards);

    gtk_progress_bar_set_fraction (priv->status_progressbar, fraction);
    gtk_label_set_text (priv->status_label, text);

    if (text != NULL) g_free (text);
}


void
gw_flashcardwindow_check_answer (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    gboolean correct;

    priv = window->priv;
    correct = gw_flashcardwindow_user_answer_is_correct (window);

    if (correct)
    {
      if (priv->cards_left > 0) priv->cards_left--;
      gw_flashcardwindow_set_card_completed (window, TRUE);
      gw_flashcardwindow_increment_correct_guesses (window);
      gw_flashcardwindow_update_progress (window);
    }
    else
    {
      gw_flashcardwindow_increment_incorrect_guesses (window);
    }

    gw_flashcardwindow_load_iterator (window, TRUE, correct);
}


static gboolean
gw_flashcardwindow_finalize_inner_path (GtkTreeModel *model,
                                        GtkTreePath  *path,
                                        GtkTreeIter  *iter,
                                        gpointer      data  )
{
    GtkTreePath *inner_path;

    gtk_tree_model_get (model, iter, COLUMN_TREE_PATH, &inner_path, -1);

    if (inner_path != NULL) gtk_tree_path_free (inner_path);

    return TRUE;
}


void
gw_flashcardwindow_set_finished (GwFlashCardWindow *window)
{
    GwFlashCardWindowPrivate *priv;
    GtkWidget *finished_label;
    gchar *markup;
    gint64 time;
    gint64 difference;
    gint hours, minutes, seconds;
    gint percent;

    priv = window->priv;
    finished_label = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "finished_label"));
    time = g_get_monotonic_time ();
    difference = (time - priv->time) / 1000000;
    hours = difference / 60 / 60;
    minutes = difference / 60 % 60;
    seconds = difference % 60 % 60;
    percent = (priv->correct_guesses * 100) / (priv->correct_guesses + priv->incorrect_guesses);

    markup = g_markup_printf_escaped (
      "<big><b>%s</b></big>\n\n"
      "<b>%s:</b> %d%%\n"
      "<b>%s:</b> %02d:%02d:%02d\n",
      gettext("You've finished this flash card deck!"), 
      gettext("Your Grade"), percent, 
      gettext("Completion Time"), hours, minutes, seconds);
    if (markup != NULL)
    {
      gtk_label_set_markup (GTK_LABEL (finished_label), markup);
      g_free (markup);
    }

    gtk_widget_hide (GTK_WIDGET (priv->content_box));
    gtk_widget_show (GTK_WIDGET (priv->finished_box));
}
