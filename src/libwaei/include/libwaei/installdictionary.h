#ifndef LW_INSTALLDICTIONARY_INCLUDED
#define LW_INSTALLDICTIONARY_INCLUDED

G_BEGIN_DECLS

#define LW_INSTALLDICTIONARY_ERROR "gWaei Dictionary Installer Error"

typedef enum {
  LW_INSTALLDICTIONARY_ERROR_SOURCE_PATH,
  LW_INSTALLDICTIONARY_ERROR_TARGET_PATH,
  LW_INSTALLDICTIONARY_ERROR_FILE_MOVE
} LwInstallDictionaryError;

typedef enum {
  LW_INSTALLDICTIONARY_NEEDS_DOWNLOADING,
  LW_INSTALLDICTIONARY_NEEDS_DECOMPRESSION,
  LW_INSTALLDICTIONARY_NEEDS_TEXT_ENCODING,
  LW_INSTALLDICTIONARY_NEEDS_POSTPROCESSING,
  LW_INSTALLDICTIONARY_NEEDS_FINALIZATION,
  LW_INSTALLDICTIONARY_NEEDS_NOTHING,
  LW_INSTALLDICTIONARY_TOTAL_URIS
} LwInstallDictionaryUri;

//Boilerplate
typedef struct _LwInstallDictionary LwInstallDictionary;
typedef struct _LwInstallDictionaryClass LwInstallDictionaryClass;
typedef struct _LwInstallDictionaryPrivate LwInstallDictionaryPrivate;

#define LW_TYPE_INSTALLDICTIONARY              (lw_installdictionary_get_type())
#define LW_INSTALLDICTIONARY(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), LW_TYPE_INSTALLDICTIONARY, LwInstallDictionary))
#define LW_INSTALLDICTIONARY_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), LW_TYPE_INSTALLDICTIONARY, LwInstallDictionaryClass))
#define LW_IS_INSTALLDICTIONARY(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), LW_TYPE_INSTALLDICTIONARY))
#define LW_IS_INSTALLDICTIONARY_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), LW_TYPE_INSTALLDICTIONARY))
#define LW_INSTALLDICTIONARY_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), LW_TYPE_INSTALLDICTIONARY, LwInstallDictionaryClass))

struct _LwInstallDictionary {
  LwDictionary object;
	LwInstallDictionaryPrivate *priv;
};

struct _LwInstallDictionaryClass {
  LwDictionaryClass parent_class;
};

//Methods
GType lw_installdictionary_get_type (void) G_GNUC_CONST;

LwInstallDictionary* lw_installdictionary_new_using_pref_uri (const gchar*, 
                                            const gchar*,
                                            const gchar*,
                                            const gchar*,
                                            LwPreferences*,
                                            const gchar*,
                                            const gchar*,
																						GType,
                                            const LwCompression,
                                            const LwEncoding,
                                            gboolean, gboolean, gboolean);

LwInstallDictionary* lw_installdictionary_new (const gchar*,
                             const gchar*,
                             const gchar*,
                             const gchar*,
                             const gchar*,
														 GType,
                             const LwCompression,
                             const LwEncoding,
                             gboolean, gboolean, gboolean);
void lw_installdictionary_free (LwInstallDictionary*);
void lw_installdictionary_init (LwInstallDictionary*,
                       const gchar*,
                       const gchar*,
                       const gchar*,
                       const gchar*,
                       const gchar*,
											 GType,
                       const LwCompression,
                       const LwEncoding,
                       gboolean, gboolean, gboolean);
void lw_installdictionary_deinit (LwInstallDictionary*);


void lw_installdictionary_set_filename (LwInstallDictionary*, const gchar*);
void lw_installdictionary_set_type (LwInstallDictionary*, GType);
void lw_installdictionary_set_encoding (LwInstallDictionary*, const LwEncoding);
void lw_installdictionary_set_compression (LwInstallDictionary*, const LwCompression);
void lw_installdictionary_set_download_source (LwInstallDictionary*, const gchar*);
void lw_installdictionary_set_split (LwInstallDictionary *di, const gboolean);
void lw_installdictionary_set_merge (LwInstallDictionary *di, const gboolean);
void lw_installdictionary_set_status (LwInstallDictionary *di, const LwInstallDictionaryUri);
gchar* lw_installdictionary_get_status_string (LwInstallDictionary*, gboolean);

void lw_installdictionary_regenerate_save_target_uris (LwInstallDictionary*);
gboolean lw_installdictionary_data_is_valid (LwInstallDictionary*);

gboolean lw_installdictionary_install (LwInstallDictionary*, LwIoProgressCallback, gpointer, GError**);
gchar* lw_installdictionary_get_target_uri (LwInstallDictionary*, const LwInstallDictionaryUri, const gint);
gchar* lw_installdictionary_get_source_uri (LwInstallDictionary*, const LwInstallDictionaryUri, const gint);

double lw_installdictionary_get_total_progress (LwInstallDictionary*, double);
double lw_installdictionary_get_process_progress (LwInstallDictionary*, double);

void lw_installdictionary_set_cancel_operations (LwInstallDictionary*, gboolean);

void gw_installdictionary_update_source_uri_cb (GSettings*, gchar*, gpointer);

G_END_DECLS

#endif
