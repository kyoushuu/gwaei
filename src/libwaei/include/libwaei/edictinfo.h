#ifndef LW_EDICTINFO_INCLUDED
#define LW_EDICTINFO_INCLUDED

G_BEGIN_DECLS


//Boilerplate
typedef struct _LwEdictInfo LwEdictInfo;
typedef struct _LwEdictInfoClass LwEdictInfoClass;
typedef struct _LwEdictInfoPrivate LwEdictInfoPrivate;

#define LW_TYPE_EDICTINFO              (lw_edictinfo_get_type())
#define LW_EDICTINFO(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), LW_TYPE_EDICTINFO, LwEdictInfo))
#define LW_EDICTINFO_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), LW_TYPE_EDICTINFO, LwEdictInfoClass))
#define LW_IS_EDICTINFO(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), LW_TYPE_EDICTINFO))
#define LW_IS_EDICTINFO_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), LW_TYPE_EDICTINFO))
#define LW_EDICTINFO_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), LW_TYPE_EDICTINFO, LwEdictInfoClass))

struct _LwEdictInfo {
  LwDictInfo object;
};

struct _LwEdictInfoClass {
  LwDictInfo parent_class;
};

//Methods
LwDictInfo* lw_edictinfo_new (const gchar*);
GType lw_edictinfo_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif

