/** \file edictidx.h
 *  \brief EDICTIDX public header
 *
 *  Contains exported constant, data type and interface declarations.
 */

/* opaque type */
struct edict_idx_s;

/** \brief EDICTIDX index object
 *
 *  Represents an index linked to EDICT format dictionary file.
 *  EDICTIDX is a key-value index implemented as a hash table.
 *  The index table may be file or memory backed.
 *  The dictionary is always a file and can be shared
 *  among several indexes.
 */
typedef /*@abstract@*/ struct edict_idx_s edict_idx;

/** \name edict_idx_open flags
 *
 * Combine these values to provide edict_idx_open flags argument.
 * @{
 */

/** \brief Open existing file based index */
/*@constant unsigned F_EDICT_IDX_OPEN @*/
#define F_EDICT_IDX_OPEN      0U

/** \brief Create new file or memory backed index */
/*@constant unsigned F_EDICT_IDX_CREATE @*/
#define F_EDICT_IDX_CREATE    1U

/** \brief Create new memory backed index */
/*@constant unsigned F_EDICT_IDX_IN_MEMORY @*/
#define F_EDICT_IDX_IN_MEMORY 2U

/** @} */

/** \brief Create new edict_idx object from dictionary
 *  \param idx_fname
 *    EDICTIDX index file name. The file must be accessible for reading (OPEN)
 *    or writing (CREATE). This parameter must be null
 *    if F_EDICT_IDX_IN_MEMORY flag is specified.
 *  \param dict_fname
 *    EDICT format dictionary file name.
 *    The file must be accessible for reading.
 *  \param flags
 *    Index creation flags. See F_EDICT_IDX_* defines.
 *  \returns
 *    Pointer to the new edict_idx object or null if error occurred.
 *
 *  Allowed flags values are:
 *
 *    F_EDICT_IDX_OPEN   - open existing index file
 *
 *    F_EDICT_IDX_CREATE - create new file-backed index
 *
 *    F_EDICT_IDX_CREATE | F_EDICT_IDX_IN_MEMORY - create new memory-backed index
 */
/*@null@*/
edict_idx*
edict_idx_open(/*@null@*/ const char* idx_fname,
	       const char* dict_fname, unsigned flags)
/*@modifies nothing @*/
;

/** \brief Create new edict_idx object from existing using same dictionary
 *  \param idx_fname
 *    EDICTIDX index file name. The file must be accessible for reading (OPEN)
 *    or writing (CREATE). This parameter must be null
 *    if F_EDICT_IDX_IN_MEMORY flag is specified.
 *  \param parent
 *    An existing edictidx object linked to a valid EDICT dictionary.
 *  \param flags
 *    Index creation flags. See F_EDICT_IDX_* defines.
 *  \returns
 *    Pointer to the new edict_idx object or null if error occurred.
 *
 *  Allowed flags values are:
 *
 *    F_EDICT_IDX_OPEN   - open existing index file
 *
 *    F_EDICT_IDX_CREATE - create new file-backed index
 *
 *    F_EDICT_IDX_CREATE | F_EDICT_IDX_IN_MEMORY - create new memory-backed index
 *
 *  The dictionary memory is shared between the parent
 *  and the newly created indexes.
 *  The parent edictidx object retains sole ownership of the dictionary
 *  and must not be destroyed during the lifetime of the dependent object.
 */
/*@null@*/
edict_idx*
edict_idx_share(/*@null@*/ const char* idx_fname,
		/*@dependent@*/ const edict_idx* parent,
		unsigned flags)
/*@modifies nothing @*/
;

/** \brief Destroy edict_idx object
 *  \param s
 *    A valid pointer to an edict_idx object.
 *
 *  Destroys edict_idx object, frees associated resources and
 *  closes file handles.
 *  An object pointed by s, all associated edict_idx
 *  (via dictionary sharing) and
 *  edict_idx_query objects become invalid.
 */
void edict_idx_close(/*@only@*/ edict_idx* s)
/*@modifies s @*/
;

/** \brief Adjustable edictidx parameter fields */
typedef enum {
	/** \brief Minimum index size
	 *
	 *  Initial index size in bytes. Must be a power of 2.
	 *  The index grows by doubling its size and rebuilding
	 *  each time it is completely filled.
	 */
	P_EDICT_IDX_MIN_SIZE,
	/** \brief Maximum index size
	 *
	 *  Maximum allowed index size in bytes.
	 *  The index stops growing after this size is reached
	 *  and an error is reported.
	 */
	P_EDICT_IDX_MAX_SIZE,
	/** \brief Maximum dictionary entry size
	 *
	 *  Maximum allowed dictionary entry size in bytes.
	 *  The index building process aborts if a larger entry is found.
	 */
	P_EDICT_IDX_MAX_ENTRY_SIZE,
	/** \brief Maximum hash chain length
	 *
	 *  Indexing algorithm grows the hash table if it encounters
	 *  longer chains.
	 */
	P_EDICT_IDX_MAX_CHAIN,
	/** \brief Maximum search result list length
	 *
	 *  Maximum allowable amount of index entries
	 *  sharing the same key.
	 *  If the list length limit for some key is reached
	 *  all subsequent entries for the same key are silently dropped
	 *  in the index building stage.
	 *  This can be used to limit amount of results returned and
	 *  to reduce the index file size for prefix/suffix/substring searches.
	 */
	P_EDICT_IDX_MAX_LIST
} edict_idx_parameter_t;

/** \brief Get edict_idx parameter
 *  \param s
 *    A valid pointer to an edict_idx object.
 *  \param par
 *    A parameter field identifier.
 *  \returns
 *    The parameter value.
 */
int edict_idx_get_parameter(edict_idx* s, edict_idx_parameter_t par)
/*@modifies nothing @*/
;

/** \brief Set edict_idx parameter
 *  \param s
 *    A valid pointer to an edict_idx object.
 *  \param par
 *    A parameter field identifier.
 *  \param value
 *    The parameter value.
 *  \returns
 *    0 on success and -1 on error.
 *
 *  An error is signaled is the parameter value is out of acceptable range.
 */
int edict_idx_set_parameter(edict_idx* s, edict_idx_parameter_t par,
			    int value)
/*@modifies s @*/
;

/** \brief Index key type identifiers */
typedef enum {
	/** \brief User defined key type
	 *
	 *  Key parsing function is provided explicitly by the caller.
	 *  See edict_idx_key_fn_t prototype.
	 */
	T_EDICT_IDX_KEY_USER,
	/** \brief Kanji key
	 *
	 *  The Japanese word as written in the first field
	 *  of the EDICT entry. May actually be kanji, kana or even ASCII.
	 */
	T_EDICT_IDX_KEY_KANJI,
	/** \brief Kana key
	 *
	 *  The transcription of the Japanese word in
	 *  the optional [] field of the EDICT entry.
	 *  Usually written in hiragana.
	 */
	T_EDICT_IDX_KEY_KANA,
	/** \brief English key (case sensitive)
	 *
	 *  The translation(s) in the //-separated field(s)
	 *  of the EDICT entry.
	 *  The translated expression is matched as a whole, exactly.
	 *  All available translations are matched and added to the index.
	 */
	T_EDICT_IDX_KEY_ENGLISH
} edict_idx_key_types_t;

/** \brief User defined key parsing function prototype
 *  \param [out] pkey_sz
 *    Returned key size pointer.
 *  \param [in] entry
 *    EDICT entry string pointer.
 *  \param entry_sz
 *    EDICT entry string size.
 *  \param [in,out] ppos
 *    Parsing position variable pointer used to store
 *    the internal parsing state.
 *  \returns
 *    Key string pointer and size via *pkey_sz
 *    or null if no more keys found.
 *
 *  This function uses counted byte strings without \0
 *  termination as both input and output.
 *  The returned pointer may alias *entry to return
 *  the key substring. The string it points to must remain
 *  accessible until the next call.
 *
 *  The position variable of type int is allocated
 *  by the caller and initialized to 0 when edict_idx_key_fn
 *  is called for the first time for a given entry.
 *  It is used by the parsing function only and
 *  is not inspected by the caller.
 */
typedef /*@null@*/ /*@dependent@*/ const char*
(*edict_idx_key_fn_t)(/*@out@*/ size_t* pkey_sz,
		      const char* entry, size_t entry_sz,
		      int* ppos)
/*@modifies pkey_sz, ppos @*/;

/** \brief Build the index for a given key
 *  \param s
 *    A valid edict_idx object pointer.
 *  \param key_type
 *    Required key type identifier. Must be one of the defined
 *    edict_idx_key_types_t members.
 *  \param key_parser
 *    User defined key parser function.
 *    Used only with T_EDICT_IDX_KEY_USER key type and should be
 *    null otherwise.
 *  \returns
 *    0 on success, -1 on error.
 *
 *  (Re)builds the index for a given key.
 *  Index building takes some time and is NOT thread-safe.
 *  See edict_idx_parameter_t for parameters affecting the result.
 */
int edict_idx_build(edict_idx* s, edict_idx_key_types_t key_type,
		    /*@null@*/ /*@dependent@*/
		    edict_idx_key_fn_t key_parser)
/*@modifies s @*/
;

/** \brief Verify the index for a given key
 *  \param s
 *    A valid edict_idx object pointer.
 *  \param key_type
 *    Required key type identifier. Must be one of the defined
 *    edict_idx_key_types_t members.
 *  \param key_parser
 *    User defined key parser function.
 *    Used only with T_EDICT_IDX_KEY_USER key type and should be
 *    null otherwise.
 *  \returns
 *    0 on success, -1 on error.
 *
 *  Verifies the index for a given key.
 *  The index itself does not contain the key identifier and
 *  it must be supplied for verification.
 *  Index verification takes some time and is thread-safe.
 *
 *  The verification result is affected by the P_EDICT_IDX_MAX_LIST
 *  edictidx parameter. Verifying the index with longer MAX_LIST
 *  than it was built with will cause verification to fail.
 */
int edict_idx_verify(edict_idx* s, edict_idx_key_types_t key_type,
		     /*@null@*/ /*@dependent@*/
		     edict_idx_key_fn_t key_parser);

/* opaque type */
struct edict_idx_query_s;

/** \brief EDICTIDX query object
 *
 *  Represents index query results linked to a edictidx object.
 *  It is implemented as a simple sequential iterator with a single
 *  method.
 */
typedef /*@abstract@*/ struct edict_idx_query_s edict_idx_query;

/** \brief Find the key in the dictionary using the index
 *  \param idx
 *    A valid edict_idx object pointer.
 *  \param key
 *    Key string. A counted byte string, \0 termination is not required.
 *  \param key_sz
 *    Key string size in bytes.
 *  \returns
 *    An edict_idx_query object if at least one result is found,
 *    null otherwise.
 *
 *  This method returns an iterator linked to the edictidx object.
 *  The iterator does not store the results itself and has a small
 *  memory footprint independent of amount of results found.
 *
 *  The edict_idx_query object returned must not outlive
 *  the parent edictidx object and must be explicitly be freed
 *  via edict_idx_query_close.
 *
 *  Querying the index is thread safe, but advancing the iterator is not.
 *  Concurrent queries on the same edictidx object are supported as long
 *  they use different edict_idx_query objects.
 */
/*@null@*/
edict_idx_query*
edict_idx_find(/*@dependent@*/ edict_idx* idx,
	       const char* key, size_t key_sz)
/*@modifies nothing @*/;

/** \brief Get the next search result
 *  \param s
 *    A valid edict_idx_query object pointer.
 *  \param [out] result_entry
 *    Result entry string buffer pointer.
 *  \param max_result_sz
 *    Result entry string buffer size.
 *  \returns
 *    The size of the result entry in bytes excluding terminating \0
 *    or 0 if no more results available.
 *
 *  The result entry is stored in the provided buffer and the \0 termination
 *  is appended. The iterator is then advanced to the next result entry.
 *
 *  If the buffer supplied is too small to contain the result
 *  only first max_result_sz-1 bytes and the terminating \0 is stored
 *  in the user buffer.
 *
 *  Advancing the iterator is NOT thread-safe.
 */
size_t edict_idx_query_result(edict_idx_query* s,
			      /*@out@*/ char* result_entry,
			      size_t max_result_sz)
/*@modifies s, result_entry @*/;

/** \brief Destroy edict_idx_query object
 *  \param s
 *    A valid pointer to an edict_idx_query object.
 *
 *  Destroys edict_idx_query object.
 *  An object pointed by s becomes invalid.
 *  edict_idx_query objects can be destroyed at any time before the parent
 *  edict_idx object.
 */
void edict_idx_query_close(/*@only@*/ edict_idx_query* s)
/*@modifies s @*/;
