#ifndef LW_DICTIONARY_INSTALLER_INCLUDED
#define LW_DICTIONARY_INSTALLER_INCLUDED

G_BEGIN_DECLS

typedef struct _LwDictionaryInstall LwDictionaryInstall;

typedef enum {
  LW_DICTIONARY_INSTALLER_STATE_IDLE,
  LW_DICTIONARY_INSTALLER_STATE_DOWNLOADING,
  LW_DICTIONARY_INSTALLER_STATE_DECOMPRESSING,
  LW_DICTIONARY_INSTALLER_STATE_ENCODING,
  LW_DICTIONARY_INSTALLER_STATE_POSTPROCESSING,
  LW_DICTIONARY_INSTALLER_STATE_FINISHING,
  TOTAL_LW_DICTIONARY_INSTALLER_STATES
} LwDictionaryInstallerState;

//Methods

LwDictionaryInstall* lw_dictionaryinstall_new (void);
void lw_dictionaryinstall_free (LwDictionaryInstall*);

LwDictionaryInstallerState lw_dictionary_installer_get_state (LwDictionary*);
void lw_dictionary_installer_set_state (LwDictionary*, LwDictionaryInstallerState);
gboolean lw_dictionary_installer_is_valid (LwDictionary*);

gboolean lw_dictionary_installer_download (LwDictionary*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_dictionary_installer_decompress (LwDictionary*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_dictionary_installer_convert_encoding (LwDictionary*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_dictionary_installer_postprocess (LwDictionary*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_dictionary_installer_install (LwDictionary*, LwIoProgressCallback, gpointer, GError **);
void lw_dictionary_installer_clean (LwDictionary*, LwIoProgressCallback, gpointer);

gdouble lw_dictionary_installer_get_step_progress (LwDictionary*);
gdouble lw_dictionary_installer_get_progress (LwDictionary*);
gchar* lw_dictionary_installer_get_status_string (LwDictionary*, gboolean);

G_END_DECLS

#endif
