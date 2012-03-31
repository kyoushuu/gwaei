#ifndef LW_EXAMPLEDICTINFO_INCLUDED
#define LW_EXAMPLEDICTINFO_INCLUDED

G_BEGIN_DECLS


//Boilerplate
typedef struct _LwExampledictInfo LwExampledictInfo;
typedef struct _LwExampledictInfoClass LwExampledictInfoClass;
typedef struct _LwExampledictInfoPrivate LwExampledictInfoPrivate;

#define LW_TYPE_EXAMPLEDICTINFO              (lw_exampledictinfo_get_type())
#define LW_EXAMPLEDICTINFO(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), LW_TYPE_EXAMPLEDICTINFO, LwExampledictInfo))
#define LW_EXAMPLEDICTINFO_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), LW_TYPE_EXAMPLEDICTINFO, LwExampledictInfoClass))
#define LW_IS_EXAMPLEDICTINFO(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), LW_TYPE_EXAMPLEDICTINFO))
#define LW_IS_EXAMPLEDICTINFO_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), LW_TYPE_EXAMPLEDICTINFO))
#define LW_EXAMPLEDICTINFO_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), LW_TYPE_EXAMPLEDICTINFO, LwExampledictInfoClass))

struct _LwExampledictInfo {
  LwDictInfo object;
};

struct _LwExampledictInfoClass {
  LwDictInfo parent_class;
};

//Methods
LwDictInfo* lw_exampledictinfo_new (const gchar*);
GType lw_exampledictinfo_get_type (void) G_GNUC_CONST;


G_END_DECLS

#endif

