
//!
//! @brief Starts the install when the add button on the dictionary chooser is selected
//!
G_MODULE_EXPORT void gw_installprogress_start_cb (GtkWidget *widget, gpointer data)
{
    gw_common_load_ui_xml ("installprogress.ui");

    //Declarations
    GtkBuilder *builder;
    GtkWidget *dialog_installprogress;
    GtkWidget *dialog_dictionaryinstall;
    GtkWidget *window_settings;
    GError *error;

    //Initializations
    builder = gw_common_get_builder ();
    dialog_installprogress = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_dialog"));
    dialog_dictionaryinstall = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));
    window_settings = GTK_WIDGET (gtk_builder_get_object (builder, "settings_window"));
    error = NULL;

    //Remove the prevous window
    gtk_widget_hide (GTK_WIDGET (dialog_dictionaryinstall));

    //Set the new window
    gtk_window_set_transient_for (GTK_WINDOW (dialog_installprogress), GTK_WINDOW (window_settings));
    gtk_window_set_position (GTK_WINDOW (window_settings), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_show (GTK_WIDGET (dialog_installprogress));

    g_thread_create (_installprogress_install_thread, NULL, FALSE, &error);

    gw_common_handle_error (&error, GTK_WINDOW (window_settings), TRUE);
}


static gpointer _installprogress_install_thread (gpointer data)
{
    //Declarations
    GList *list;
    GList *iter;
    LwDictInst *di;
    GError *error;
    GtkBuilder *builder;
    GtkWidget *window_settings;
    gint timeoutid;

    //Initializations
    builder = gw_common_get_builder ();
    list = lw_dictinstlist_get_list ();
    window_settings = GTK_WIDGET (gtk_builder_get_object (builder, "settings_window"));
    error = NULL;

    //Do the installation
    for (iter = list; iter != NULL && error == NULL; iter = iter->next)
    {
      di = (LwDictInst*) iter->data;
      if (di->selected)
      {
        timeoutid = g_timeout_add (100, _installprogress_update_ui_timeout, di);
        lw_dictinst_install (di, _installprogress_update_dictinst_cb, &error);
        g_source_remove (timeoutid);
      }
    }
    g_timeout_add (100, _installprogress_update_ui_timeout, NULL);

    //Cleanup
gdk_threads_enter ();
    gw_common_handle_error (&error, GTK_WINDOW (window_settings), FALSE);
    lw_dictinfolist_initialize ();
    gw_dictionarymanager_update_items ();
    lw_dictinstlist_set_cancel_operations (FALSE);
gdk_threads_leave ();

    return NULL;
}


static int _installprogress_update_dictinst_cb (double fraction, gpointer data)
{
    //Declarations
    LwDictInst *di;

    //Initializations
    di = data;

    g_mutex_lock (di->mutex); 
    _install_fraction = lw_dictinst_get_total_progress (di, fraction);
    g_mutex_unlock (di->mutex);

    return 0;
}


//!
//! @brief Callback to update the install dialog progress.  The data passed to it should be
//!        in the form of a LwDictInst.  If it is NULL, the progress window will be closed.
//!
static gboolean _installprogress_update_ui_timeout (gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *progressbar;
    GtkWidget *label;
    GtkWidget *sublabel;
    GtkWidget *dialog;
    GList *list;
    GList *iter;
    int current_to_install;
    int total_to_install;
    LwDictInst *di;
    char *text_installing;
    char *text_installing_markup;
    char *text_left;
    char *text_left_markup;
    char *text_progressbar;

    //Sanity check
    if (data == NULL) {
      builder = gw_common_get_builder ();
      dialog = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_dialog"));
      gtk_widget_hide (dialog);
      return FALSE;
    }

    //Initializations
    di = data;
    g_mutex_lock (di->mutex);

    builder = gw_common_get_builder ();
    label = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_label"));
    sublabel = GTK_WIDGET (gtk_builder_get_object (builder, "sub_install_progress_label"));
    progressbar = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_progressbar"));
    list = lw_dictinstlist_get_list ();
    current_to_install = 0;
    total_to_install = 0;

    //Calculate the number of dictionaries left to install
    for (iter = list; iter != NULL; iter = iter->next)
    {
      di = iter->data;
      if (di->selected)
      {
        current_to_install++;
      }
      if (iter->data == data) break;
    }

    //Calculate the number of dictionaries left to install
    for (iter = list; iter != NULL; iter = iter->next)
    {
      di = iter->data;
      if (di->selected)
      {
        total_to_install++;
      }
    }
    
    di = data;

    text_progressbar =  g_markup_printf_escaped (gettext("Installing %s..."), di->filename);
    text_left = g_strdup_printf (gettext("Installing dictionary %d of %d..."), current_to_install, total_to_install);
    text_left_markup = g_markup_printf_escaped ("<big><b>%s</b></big>", text_left);
    text_installing = lw_dictinst_get_status_string (di, TRUE);
    text_installing_markup = g_markup_printf_escaped ("<small>%s</small>", text_installing);

    gtk_label_set_markup (GTK_LABEL (label), text_left_markup);
    gtk_label_set_markup (GTK_LABEL (sublabel), text_installing_markup);
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progressbar), _install_fraction);
    gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progressbar), text_progressbar);

    g_mutex_unlock (di->mutex);

    //Cleanup
    g_free (text_progressbar);
    g_free (text_left);
    g_free (text_left_markup);
    g_free (text_installing);
    g_free (text_installing_markup);

    return TRUE;
}


G_MODULE_EXPORT void gw_installprogress_cancel_cb (GtkWidget *widget, gpointer data)
{
    lw_dictinstlist_set_cancel_operations (TRUE);
}
