#ifndef LW_DICTIONARY_INSTALLER_INCLUDED
#define LW_DICTIONARY_INSTALLER_INCLUDED

G_BEGIN_DECLS

typedef struct _LwDictionaryInstall LwDictionaryInstall;

typedef enum {
  LW_DICTIONARY_INSTALLER_STATUS_UNINSTALLED,
  LW_DICTIONARY_INSTALLER_STATUS_DOWNLOADING,
  LW_DICTIONARY_INSTALLER_STATUS_DECOMPRESSING,
  LW_DICTIONARY_INSTALLER_STATUS_ENCODING,
  LW_DICTIONARY_INSTALLER_STATUS_POSTPROCESSING,
  LW_DICTIONARY_INSTALLER_STATUS_FINISHING,
  LW_DICTIONARY_INSTALLER_STATUS_INSTALLED,
  TOTAL_LW_DICTIONARY_INSTALLER_STATUSES
} LwDictionaryInstallerStatus;

//Methods

LwDictionaryInstall* lw_dictionaryinstall_new (void);
void lw_dictionaryinstall_free (LwDictionaryInstall*);

LwDictionaryInstallerStatus lw_dictionary_installer_get_status (LwDictionary*);
void lw_dictionary_installer_set_status (LwDictionary*, LwDictionaryInstallerStatus);
gint lw_dictionary_installer_get_file_index (LwDictionary*);
gboolean lw_dictionary_installer_is_valid (LwDictionary*);

gboolean lw_dictionary_installer_download (LwDictionary*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_dictionary_installer_decompress (LwDictionary*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_dictionary_installer_convert_encoding (LwDictionary*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_dictionary_installer_postprocess (LwDictionary*, LwIoProgressCallback, gpointer, GError**);
gboolean lw_dictionary_installer_install (LwDictionary*, LwIoProgressCallback, gpointer, GError **);
void lw_dictionary_installer_clean (LwDictionary*, LwIoProgressCallback, gpointer);

gdouble lw_dictionary_installer_get_stage_progress (LwDictionary*, gdouble);
gdouble lw_dictionary_installer_get_total_progress (LwDictionary*, gdouble);
gchar* lw_dictionary_installer_get_status_string (LwDictionary*, gboolean);

G_END_DECLS

#endif
