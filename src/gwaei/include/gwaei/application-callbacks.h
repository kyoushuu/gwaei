#ifndef GW_APPLICATION_CALLBACKS_INCLUDED
#define GW_APPLICATION_CALLBACKS_INCLUDED

void gw_application_quit_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_preferences_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_settingswindow_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_aboutdialog_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_searchwindow_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_open_vocabularywindow_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_show_glossary_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_show_help_cb (GSimpleAction*, GVariant*, gpointer);
void gw_application_show_homepage_cb (GSimpleAction*, GVariant*, gpointer);

#endif
