#ifndef LW_UNKNOWNDICTINFO_INCLUDED
#define LW_UNKNOWNDICTINFO_INCLUDED

G_BEGIN_DECLS


//Boilerplate
typedef struct _LwUnknowndictInfo LwUnknowndictInfo;
typedef struct _LwUnknowndictInfoClass LwUnknowndictInfoClass;
typedef struct _LwUnknowndictInfoPrivate LwUnknowndictInfoPrivate;

#define LW_TYPE_UNKNOWNDICTINFO              (lw_unknowndictinfo_get_type())
#define LW_UNKNOWNDICTINFO(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), LW_TYPE_UNKNOWNDICTINFO, LwUnknowndictInfo))
#define LW_UNKNOWNDICTINFO_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), LW_TYPE_UNKNOWNDICTINFO, LwUnknowndictInfoClass))
#define LW_IS_UNKNOWNDICTINFO(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), LW_TYPE_UNKNOWNDICTINFO))
#define LW_IS_UNKNOWNDICTINFO_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), LW_TYPE_UNKNOWNDICTINFO))
#define LW_UNKNOWNDICTINFO_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), LW_TYPE_UNKNOWNDICTINFO, LwUnknowndictInfoClass))

struct _LwUnknowndictInfo {
  LwDictInfo object;
};

struct _LwUnknowndictInfoClass {
  LwDictInfo parent_class;
};

//Methods
LwDictInfo* lw_unknowndictinfo_new (const gchar*);
GType lw_unknowndictinfo_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif

