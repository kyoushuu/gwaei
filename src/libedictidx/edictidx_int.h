/* INTERNAL */

typedef /*@immutable@*/ const char* str_p;
typedef /*@unsignedintegraltype@*/ unsigned int htab_index_t;

struct edict_idx_file_s;
typedef /*@abstract@*/ struct edict_idx_file_s edict_idx_file;

struct edict_idx_parser_s;
typedef /*@abstract@*/ struct edict_idx_parser_s edict_idx_parser;

size_t /*@alt void @*/ edict_idx_fputs(str_p s, size_t sz, FILE* f)
/*@modifies f @*/
;

/*@constant unsigned F_FILE_MUTABLE @*/
#define F_FILE_MUTABLE 1U

struct edict_idx_file_s {
	/*@null@*/ FILE* file;
	/*@owned@*/ void* mem;
	size_t size;
	unsigned flags;
};

/*@constant size_t C_EDICT_MIN_IDX_SIZE @*/
#define C_EDICT_MIN_IDX_SIZE	(size_t) 0x10000
/*@constant size_t C_EDICT_MAX_IDX_SIZE @*/
#define C_EDICT_MAX_IDX_SIZE	(size_t) 0x1000000
/*@constant size_t C_EDICT_MAX_ENTRY_SIZE @*/
#define C_EDICT_MAX_ENTRY_SIZE	(size_t) 0x1000

/*@constant unsigned C_EDICT_MAX_CHAIN_SIZE @*/
#define C_EDICT_MAX_CHAIN_SIZE	1000U
/*@constant unsigned C_EDICT_MAX_LIST_SIZE @*/
#define C_EDICT_MAX_LIST_SIZE	100U

struct edict_idx_params {
	size_t min_idx_size, max_idx_size;
	size_t max_entry_size;

	unsigned max_list, max_chain;
};

struct edict_idx_htab_rec {
	uint32_t cksum;
	uint32_t offset;
};

struct edict_idx_s {
	/*@null@*/
	edict_idx_file* dict_file;
	/*@dependent@*/ str_p dict;
	size_t dict_size;

	edict_idx_file* htab_file;
	/*@dependent@*/
	struct edict_idx_htab_rec* htab;
	htab_index_t htab_size;

	struct edict_idx_params params;
};

struct edict_idx_query_s {
	/*@dependent@*/
	edict_idx* idx;

	uint32_t hash, cksum;

	htab_index_t index;
	uint32_t offset;

	int next;
};

struct edict_idx_parser_s {
	/*@dependent@*/
	edict_idx* idx;

	unsigned entry_index;

	uint32_t offset;
	uint32_t next_offset;

	/*@null@*/ /*@observer@*/ str_p entry;
	size_t entry_size;

	/*@null@*/ /*@observer@*/ str_p key;
	size_t key_size;

	/*@dependent@*/ edict_idx_key_fn_t key_parser;
	int key_parser_chain_var;
};

/* edictidx.c */

/*@null@*/
edict_idx*
edict_idx_new(/*@null@*/ const char* idx_fname,
	      /*@null@*/ const char* dict_fname,
	      /*@null@*/ /*@dependent@*/ const edict_idx* parent,
	      unsigned flags)
/*@*/;

void edict_idx_params_init(/*@out@*/ struct edict_idx_params* s)
/*@modifies s @*/;

/* build.c */
int edict_idx_htab_fill(edict_idx* s,
			/*@dependent@*/ edict_idx_key_fn_t key_parser,
			unsigned max_list)
/*@modifies s @*/;

int edict_idx_htab_resize2(edict_idx* s)
/*@modifies s @*/;

/* query.c */
void edict_idx_query_dump(edict_idx_query* s, FILE* f)
/*@modifies f @*/
;

/*@null@*/ /*@observer@*/
str_p edict_idx_get_entry(edict_idx*, /*@out@*/ size_t* pentry_sz,
			  uint32_t* poffset)
/*@modifies pentry_sz, poffset */;

htab_index_t edict_idx_htab_index(edict_idx*, uint32_t hash) /*@*/;

int edict_idx_htab_look(edict_idx*, htab_index_t* pindex,
			/*@out@*/ uint32_t* poffset,
			uint32_t cksum)
/*@modifies pindex, poffset @*/;

/* parser.c */

/*@null@*/
edict_idx_parser*
edict_idx_parser_new(/*@dependent@*/ edict_idx*,
		     /*@dependent@*/ edict_idx_key_fn_t key_parser) /*@*/;

/*@null@*/ /*@dependent@*/ /*@observer@*/
str_p
edict_idx_parser_fetch_key(edict_idx_parser* s, /*@out@*/ size_t* pkey_sz,
			   /*@out@*/ uint32_t* poffset)
/*@modifies s, pkey_sz, poffset @*/
;

void edict_idx_parser_close(/*@only@*/ edict_idx_parser* s)
/*@modifies s @*/
;

void edict_idx_parser_dump(edict_idx_parser* s, FILE* f)
/*@modifies f @*/
;

/* fileops.c */

/*@null@*/
edict_idx_file*
edict_idx_index_create(/*@null@*/ const char* fname, size_t size) /*@*/;

/*@null@*/
edict_idx_file*
edict_idx_index_open(const char* fname) /*@*/;

/*@null@*/
edict_idx_file*
edict_idx_dict_open(const char* fname) /*@*/;

int edict_idx_index_resize(edict_idx_file* s,
			   size_t new_size)
/*@modifies *s @*/;

int edict_idx_file_close(/*@only@*/ edict_idx_file* s)
/*@modifies s @*/;

/* mmap.c */

/*@null@*/
void* edict_idx_mmap_create(/*@null@*/ FILE* f, size_t size)
/*@*/;

/*@null@*/
void* edict_idx_mmap_open(FILE* f, /*@out@*/ size_t* psize)
/*@modifies f, psize @*/;

int edict_idx_mmap_resize(/*@null@*/ FILE* f, void** p, size_t size)
/*@modifies p @*/;

int edict_idx_mmap_flush(FILE* f, void* p, size_t sz)
/*@modifies f @*/;

int edict_idx_mmap_unmap(/*@null@*/ FILE* f, /*@only@*/ void* p,
		     size_t sz)
/*@modifies p @*/;

/* hash.c */

uint32_t edict_idx_hash(str_p p, size_t sz) /*@*/;
uint32_t edict_idx_cksum(str_p p, size_t sz) /*@*/;

/* key.c */

/*@-castfcnptr@*/
/*@constant null edict_idx_key_fn_t F_KEY_NULL@*/
#define F_KEY_NULL (edict_idx_key_fn_t)0
/*@=castfcnptr@*/

/*@dependent@*/ /*@null@*/
edict_idx_key_fn_t
edict_idx_select_key(edict_idx_key_types_t key_type,
		     /*@returned@*/ edict_idx_key_fn_t key_parser) /*@*/;

/*@null@*/ /*@observer@*/
str_p edict_idx_key_kanji_exact(/*@out@*/ size_t* pkey_sz,
				str_p entry, size_t entry_sz,
				int* chain)
/*@modifies pkey_sz, chain @*/;

/*@null@*/ /*@observer@*/
str_p edict_idx_key_kana_exact(/*@out@*/ size_t* pkey_sz,
			       str_p entry, size_t entry_sz,
			       int* chain)
/*@modifies pkey_sz, chain @*/;

