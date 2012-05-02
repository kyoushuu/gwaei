//!
//! @brief A callback that updates the LwInstallDictionary source uri when the pref changes
//! @param setting A GSetting object
//! @param KEY The key of the pref
//! @param data User data passed to the preference listener
//!
void 
gw_dictionary_update_source_uri_cb (GSettings *settings, gchar* key, gpointer data)
{
    //Declarations
    LwDictionaryInstall *install;
    gchar source_uri[200];

    //Initialiations
    install = LW_INSTALLDICTIONARY (data);
    lw_preferences_get_string (source_uri, settings, key, 200);

    if (install->uri[LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING] != NULL)
      g_free (install->uri[LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING]);
    install->uri[LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING] = g_strdup (source_uri);
}
