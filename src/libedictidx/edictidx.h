/* PUBLIC */

struct edict_idx_s;
typedef /*@abstract@*/ struct edict_idx_s edict_idx;


/*@constant unsigned F_EDICT_IDX_OPEN @*/
#define F_EDICT_IDX_OPEN      0U
/*@constant unsigned F_EDICT_IDX_CREATE @*/
#define F_EDICT_IDX_CREATE    1U
/*@constant unsigned F_EDICT_IDX_IN_MEMORY @*/
#define F_EDICT_IDX_IN_MEMORY 2U

/*@null@*/
edict_idx*
edict_idx_open(/*@null@*/ const char* idx_fname,
	       const char* dict_fname, unsigned flags)
/*@modifies nothing @*/
;

/*@null@*/
edict_idx*
edict_idx_share(/*@null@*/ const char* idx_fname,
		/*@dependent@*/ const edict_idx* parent,
		unsigned flags)
/*@modifies nothing @*/
;

void edict_idx_close(/*@only@*/ edict_idx* s)
/*@modifies s @*/
;

/* PARAMETERS */

typedef enum {
	P_EDICT_IDX_MIN_SIZE,
	P_EDICT_IDX_MAX_SIZE,
	P_EDICT_IDX_MAX_ENTRY_SIZE,
	P_EDICT_IDX_MAX_CHAIN,
	P_EDICT_IDX_MAX_LIST
} edict_idx_parameter_t;

int edict_idx_get_parameter(edict_idx* s, edict_idx_parameter_t par)
/*@modifies nothing @*/
;

int edict_idx_set_parameter(edict_idx* s, edict_idx_parameter_t par,
			    int value)
/*@modifies s @*/
;

/* INDEXING */

typedef enum {
	T_EDICT_IDX_KEY_USER,
	T_EDICT_IDX_KEY_KANJI,
	T_EDICT_IDX_KEY_KANA,
	T_EDICT_IDX_KEY_ENGLISH
} edict_idx_key_types_t;

typedef /*@null@*/ /*@dependent@*/ const char*
(*edict_idx_key_fn_t)(size_t* pkey_sz,
		      const char* entry, size_t entry_sz,
		      int* chain)
/*@modifies pkey_sz, chain @*/;

int edict_idx_build(edict_idx* s, edict_idx_key_types_t key_type,
		    /*@null@*/ /*@dependent@*/
		    edict_idx_key_fn_t key_parser)
/*@modifies s @*/
;

int edict_idx_verify(edict_idx*, edict_idx_key_types_t key_type,
		     /*@null@*/ /*@dependent@*/
		     edict_idx_key_fn_t key_parser);

/* QUERYING */

struct edict_idx_query_s;
typedef /*@abstract@*/ struct edict_idx_query_s edict_idx_query;

/*@null@*/
edict_idx_query*
edict_idx_find(/*@dependent@*/ edict_idx*,
	       const char* key, size_t key_sz)
/*@modifies nothing @*/;

size_t edict_idx_query_result(edict_idx_query* s,
			      /*@out@*/ char* result_entry,
			      size_t max_result_sz)
/*@modifies s, result_entry @*/;

void edict_idx_query_close(/*@only@*/ edict_idx_query* s)
/*@modifies s @*/;
