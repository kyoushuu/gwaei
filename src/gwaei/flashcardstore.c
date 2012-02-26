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
//! @file flashcardstore.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>

#include <libwaei/libwaei.h>

#include <gwaei/flashcardstore-private.h>
#include <gwaei/flashcardstore.h>

G_DEFINE_TYPE (GwFlashCardStore, gw_flashcardstore, GTK_TYPE_LIST_STORE)

typedef enum {
  PROP_0,
} GwFlashCardStoreProp;


static void gw_flashcardstore_finalize_inner_paths (GwFlashCardStore*);


GtkListStore*
gw_flashcardstore_new ()
{
    //Declarations
    GwFlashCardStore *model;

    //Initializations
    model = GW_FLASHCARDSTORE (g_object_new (GW_TYPE_FLASHCARDSTORE,
                                                  NULL));
    return GTK_LIST_STORE (model);

}


static void 
gw_flashcardstore_init (GwFlashCardStore *model)
{
    GType types[] = { 
       G_TYPE_STRING,  //GW_FLASHCARDSTORE_COLUMN_QUESTION
       G_TYPE_STRING,  //GW_FLASHCARDSTORE_COLUMN_ANSWER
       G_TYPE_BOOLEAN, //GW_FLASHCARDSTORE_COLUMN_IS_COMPLETED
       G_TYPE_POINTER, //GW_FLASHCARDSTORE_COLUMN_TREE_PATH
       G_TYPE_INT,     //GW_FLASHCARDSTORE_COLUMN_WEIGHT
       G_TYPE_INT      //GW_FLASHCARDSTORE_ORDER
    };

    gtk_list_store_set_column_types (GTK_LIST_STORE (model), TOTAL_GW_FLASHCARDSTORE_COLUMNS, types);

    model->priv = GW_FLASHCARDSTORE_GET_PRIVATE (model);
    memset(model->priv, 0, sizeof(GwFlashCardStorePrivate));
}


static void 
gw_flashcardstore_finalize (GObject *object)
{
    GwFlashCardStore *store;
//    GwFlashCardStorePrivate *priv;

    store = GW_FLASHCARDSTORE (object);
//    priv = store->priv;

    gw_flashcardstore_finalize_inner_paths (store);

    G_OBJECT_CLASS (gw_flashcardstore_parent_class)->finalize (object);
}


static void 
gw_flashcardstore_set_property (GObject  *object,
                                 guint         property_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
/*
    GwFlashCardStore *store;
    GwFlashCardStorePrivate *priv;

    store = GW_FLASHCARDSTORE (object);
    priv = model->priv;
*/

    switch (property_id)
    {
/*
      case PROP_NAME:
        if (priv->name != NULL) g_free (priv->name);
        priv->name = g_strdup ((gchar*) (g_value_get_string (value)));
        break;
*/
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void 
gw_flashcardstore_get_property (GObject      *object,
                                 guint         property_id,
                                 GValue       *value,
                                 GParamSpec   *pspec)
{
/*
    GwFlashCardStore *store;
    GwFlashCardStorePrivate *priv;

    store = GW_FLASHCARDSTORE (object);
    priv = model->priv;
*/
    switch (property_id)
    {
/*
      case PROP_NAME:
        g_value_set_string (value, priv->name);
        break;
*/
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
        break;
    }
}


static void
gw_flashcardstore_class_init (GwFlashCardStoreClass *klass)
{
    //Declarations
//    GParamSpec *pspec;
    GObjectClass *object_class;

    //Initializations
    object_class = G_OBJECT_CLASS (klass);
    object_class->set_property = gw_flashcardstore_set_property;
    object_class->get_property = gw_flashcardstore_get_property;
    object_class->finalize = gw_flashcardstore_finalize;

    g_type_class_add_private (object_class, sizeof (GwFlashCardStorePrivate));

/*
    pspec = g_param_spec_string ("name",
                                 "Name of the vocabulary list",
                                 "Set vocabulary list's name.",
                                 "Vocabulary",
                                 G_PARAM_CONSTRUCT | G_PARAM_READWRITE
    );
    g_object_class_install_property (object_class, PROP_NAME, pspec);

    klass->signalid[GW_VOCABULARYWORDSTORE_CLASS_SIGNALID_CHANGED] = g_signal_new (
        "changed",
        G_OBJECT_CLASS_TYPE (object_class),
        G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET (GwFlashCardStoreClass, changed),
        NULL, NULL,
        g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0
    );
*/
}


static gint
gw_flashcardstore_calculate_weight (guint32 hours, gint incorrect, gint correct)
{
    //The effect of this is short term score is important.
    //long term, time is important for deciding weight
    gint score;
    score = (correct * 100 / (incorrect + correct));
    return ((gint) hours - score);
}


void gw_flashcardstore_set_vocabularywordstore (GwFlashCardStore      *store, 
                                                GwVocabularyWordStore *wordstore,
                                                gint                   question_column,
                                                gint                   answer_column)
{
    //Sanity Checks
    if (store == NULL || wordstore == NULL) return;
    if (question_column < 0 || question_column >= TOTAL_GW_FLASHCARDSTORE_COLUMNS) return;
    if (answer_column < 0 || answer_column >= TOTAL_GW_FLASHCARDSTORE_COLUMNS) return;

    //Clear out the old entries
    gw_flashcardstore_finalize_inner_paths (store);
    gtk_list_store_clear (GTK_LIST_STORE (store));

    //Declarations
    GtkTreeModel *source_model;
    GtkListStore *target_store;
    GtkTreeIter source_iter;
    GtkTreeIter target_iter;
    gint weight;
    gint incorrect;
    gint correct;
    guint32 hours;
    gint order;
    gchar *question, *answer;
    GtkTreePath *path;
    gboolean valid;

    //Initializations
    source_model = GTK_TREE_MODEL (wordstore);
    target_store = GTK_LIST_STORE (store);
    valid = gtk_tree_model_get_iter_first (source_model, &source_iter);
    order = 0;

    //Copy the new entries
    while (valid)
    {
      path = gtk_tree_model_get_path (source_model, &source_iter);
      gtk_tree_model_get (source_model, &source_iter, question_column, &question, answer_column, &answer,
            GW_VOCABULARYWORDSTORE_COLUMN_TIMESTAMP, &hours,
            GW_VOCABULARYWORDSTORE_COLUMN_CORRECT_GUESSES, &correct,
            GW_VOCABULARYWORDSTORE_COLUMN_INCORRECT_GUESSES, &incorrect,
          -1);


      if (path != NULL && question != NULL && strlen (question) && answer != NULL && strlen (answer))
      {
        weight = gw_flashcardstore_calculate_weight (hours, incorrect, correct);
        gtk_list_store_append (target_store, &target_iter);
        gtk_list_store_set (target_store, &target_iter,
            GW_FLASHCARDSTORE_COLUMN_QUESTION, question,
            GW_FLASHCARDSTORE_COLUMN_ANSWER, answer,
            GW_FLASHCARDSTORE_COLUMN_TREE_PATH, path,
            GW_FLASHCARDSTORE_COLUMN_IS_COMPLETED, FALSE,
            GW_FLASHCARDSTORE_COLUMN_WEIGHT, weight,
            GW_FLASHCARDSTORE_COLUMN_ORDER, order,
            -1);
        order++;
      }

      if (question != NULL) g_free (question); question = NULL;
      if (answer != NULL) g_free (answer); answer = NULL;
      //gtk_tree_path_free (path); path = NULL; //PATH SHOULD NOT BE FREED HERE. ONLY WHEN THE MODEL IS FINALIZED

      valid = gtk_tree_model_iter_next (source_model, &source_iter);
    }

}


void gw_flashcardstore_trim (GwFlashCardStore *store, gint max)
{
    //Sanity Checks
    if (store == NULL) return;
    if (max < 1) return;

    //Declarations
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    gboolean valid;
    gint children;

    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store), GW_FLASHCARDSTORE_COLUMN_WEIGHT, GTK_SORT_DESCENDING);

    model = GTK_TREE_MODEL (store);
    children = gtk_tree_model_iter_n_children (model, NULL);
    valid = gtk_tree_model_get_iter_first (model, &iter);

    while (valid && children > max)
    {
      gtk_tree_model_get (model, &iter, GW_FLASHCARDSTORE_COLUMN_TREE_PATH, &path, -1);
      if (path != NULL) gtk_tree_path_free (path); path = NULL;
      gtk_list_store_remove (GTK_LIST_STORE (store), &iter);
      children--;
      valid = gtk_tree_model_get_iter_first (model, &iter);
    }

    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (store), GW_FLASHCARDSTORE_COLUMN_ORDER, GTK_SORT_ASCENDING);
}


void gw_flashcardstore_shuffle (GwFlashCardStore *store)
{
    //Sanity Checks
    if (store == NULL) return;

    //Declarations
    GtkTreeModel *model;
    GRand *random;
    GtkTreeIter iter;
    gint order;
    gint position;
    gint children;

    order = 0;
    model = GTK_TREE_MODEL (store);
    children = gtk_tree_model_iter_n_children (model, NULL);
    if (children > 0)
    {
      random = g_rand_new ();
      if (random != NULL)
      {
        while (children > 0)
        {
          position = g_rand_int_range (random, 0, children);
          if (gtk_tree_model_iter_nth_child (model, &iter, NULL, position))
          {
            gtk_list_store_set (GTK_LIST_STORE (store), &iter, GW_FLASHCARDSTORE_COLUMN_ORDER, &order, -1);
            gtk_list_store_move_before (GTK_LIST_STORE (store), &iter, NULL);
            order++;
          }
          children--;
        }
        g_rand_free (random); random = NULL;
      }
    }
}


static void
gw_flashcardstore_finalize_inner_paths (GwFlashCardStore *store)
{
    //Sanity checks
    if (store == NULL) return;

    //Declarations
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    gboolean valid;

    //Initializations
    model = GTK_TREE_MODEL (store);
    valid = gtk_tree_model_get_iter_first (model, &iter);

    while (valid)
    {
      gtk_tree_model_get (model, &iter, GW_FLASHCARDSTORE_COLUMN_TREE_PATH, &path, -1);

      if (path != NULL) gtk_tree_path_free (path); path = NULL;

      valid = gtk_tree_model_iter_next (model, &iter);
    }
}


/*
static void
gw_flashcardstore_increment_source_correct_guesses (GwFlashCardStore *store)
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
          gw_vocabularywordstore_update_timestamp_by_iter (GW_VOCABULARYWORDSTORE (priv->source_model), &iter);
          gw_vocabularywordstore_set_has_changes (GW_VOCABULARYWORDSTORE (priv->source_model), TRUE);
          gw_vocabularywordstore_save (GW_VOCABULARYWORDSTORE (priv->source_model), NULL);
        }

        if (source_answer != NULL) g_free (source_answer);
        if (source_question != NULL) g_free (source_question);
    }
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
          gw_vocabularywordstore_update_timestamp_by_iter (GW_VOCABULARYWORDSTORE (priv->source_model), &iter);
          gw_vocabularywordstore_set_has_changes (GW_VOCABULARYWORDSTORE (priv->source_model), TRUE);
          gw_vocabularywordstore_save (GW_VOCABULARYWORDSTORE (priv->source_model), NULL);
        }

        if (source_answer != NULL) g_free (source_answer);
        if (source_question != NULL) g_free (source_question);
    }
}
*/

