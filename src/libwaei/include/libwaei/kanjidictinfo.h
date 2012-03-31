#ifndef LW_KANJIDICTINFO_INCLUDED
#define LW_KANJIDICTINFO_INCLUDED

G_BEGIN_DECLS


//Boilerplate
typedef struct _LwKanjidictInfo LwKanjidictInfo;
typedef struct _LwKanjidictInfoClass LwKanjidictInfoClass;
typedef struct _LwKanjidictInfoPrivate LwKanjidictInfoPrivate;

#define LW_TYPE_KANJIDICTINFO              (lw_kanjidictinfo_get_type())
#define LW_KANJIDICTINFO(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), LW_TYPE_KANJIDICTINFO, LwKanjidictInfo))
#define LW_KANJIDICTINFO_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), LW_TYPE_KANJIDICTINFO, LwKanjidictInfoClass))
#define LW_IS_KANJIDICTINFO(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), LW_TYPE_KANJIDICTINFO))
#define LW_IS_KANJIDICTINFO_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), LW_TYPE_KANJIDICTINFO))
#define LW_KANJIDICTINFO_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), LW_TYPE_KANJIDICTINFO, LwKanjidictInfoClass))

struct _LwKanjidictInfo {
  LwDictInfo object;
};

struct _LwKanjidictInfoClass {
  LwDictInfo parent_class;
};

//Methods
LwDictInfo* lw_kanjidictinfo_new (const gchar*);
GType lw_kanjidictinfo_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif

