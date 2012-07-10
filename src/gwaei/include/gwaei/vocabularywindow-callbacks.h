#ifndef GW_VOCABULARYWINDOW_CALLBACKS_INCLUDED
#define GW_VOCABULARYWINDOW_CALLBACKS_INCLUDED

void gw_vocabularywindow_close_cb (GSimpleAction*, GVariant*, gpointer);
gboolean gw_vocabularywindow_delete_event_cb (GtkWidget*, GdkEvent*, gpointer);
void gw_vocabularywindow_cell_edited_cb (GtkCellRendererText*, gchar*, gchar*, gpointer);
void gw_vocabularywindow_list_cell_edited_cb (GtkCellRendererText*, gchar*, gchar*, gpointer);
void gw_vocabularywindow_list_row_deleted_cb (GtkTreeModel*, GtkTreePath*, gpointer);
gboolean gw_vocabularywindow_event_after_cb (GtkWidget*, GdkEvent*, gpointer);
void gw_vocabularywindow_liststore_changed_cb (GwVocabularyListStore*, gpointer);

void gw_vocabularywindow_sync_menubar_show_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_shuffle_flashcards_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_trim_flashcards_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_list_order_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_track_results_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_toolbar_show_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_position_column_show_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_score_column_show_cb (GSettings*, gchar*, gpointer);
void gw_vocabularywindow_sync_timestamp_column_show_cb (GSettings*, gchar*, gpointer);

void gw_vocabularywindow_toolbar_show_toggled_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_menubar_show_toggled_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_position_column_toggled_cb (GtkAction*, gpointer);
void gw_vocabularywindow_score_column_toggled_cb (GtkAction*, gpointer);
void gw_vocabularywindow_timestamp_column_toggled_cb (GtkAction*, gpointer);

void gw_vocabularywindow_new_word_cb (GSimpleAction*, GVariant*, gpointer);
void gw_vocabularywindow_new_list_cb (GSimpleAction*, GVariant*, gpointer);

void gw_vocabularywindow_select_new_word_from_dialog_cb (GtkWidget*, gpointer);

#endif
