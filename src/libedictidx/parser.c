#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "edictidx.h"
#include "edictidx_int.h"

/*@access edict_idx, edict_idx_parser @*/

/*@null@*/
edict_idx_parser*
edict_idx_parser_new(/*@dependent@*/ edict_idx* idx,
		     edict_idx_key_fn_t key_parser)

{
	edict_idx_parser* s =
		(edict_idx_parser*)malloc(sizeof(*s));
	if (!s)
		return 0;

	s->idx = idx;
	s->entry_index = 0;
	s->entry = 0;
	s->entry_size = 0;
	s->offset = 0;
	s->next_offset = 0;
	s->key_parser_chain_var = 0;
	s->key = 0;
	s->entry = 0;
	s->key_size = 0;
	s->entry_size = 0;
	s->key_parser = key_parser;

	return s;


}

str_p edict_idx_get_entry(edict_idx* s, size_t* pentry_sz, uint32_t* poffset)
{
	size_t ei, si = (size_t)*poffset;
	size_t maxi = si + s->params.max_entry_size;

	if (maxi > s->dict_size)
		maxi = s->dict_size;

	*pentry_sz = 0;

	for (ei = si; ei < maxi; ei++) {
		if (s->dict[ei] == '\n') {
			*pentry_sz = ei - si;
			*poffset = ei + 1;
			return &s->dict[si];
		}
	}

	return 0;
}

/*@null@*/ /*@dependent@*/
str_p
edict_idx_parser_fetch_key(edict_idx_parser* s, /*@out@*/ size_t* pkey_sz,
			   /*@out@*/ uint32_t* poffset)
{
	do {
		if (!s->key) {
			s->offset = s->next_offset;
			s->entry = edict_idx_get_entry(s->idx, &s->entry_size,
						       &s->next_offset);
			if (!s->entry) {
				*poffset = s->offset;
				*pkey_sz = 0;
				return 0;
			}

			s->entry_index++;
			s->key_parser_chain_var = 0;
		}

		s->key = s->key_parser(&s->key_size, s->entry, s->entry_size,
				       &s->key_parser_chain_var);
	} while (!s->key);

	*poffset = s->offset;
	*pkey_sz = s->key_size;

	return s->key;
}

void edict_idx_parser_close(/*@only@*/ edict_idx_parser* s)
{
	free(s);
}

void edict_idx_parser_dump(edict_idx_parser* s, FILE* f)
{
	fprintf(f, "<parser>\n    entry_index: %u offset: 0x%08X next_offset: 0x%08X "
		"entry_size: %u key_size: %u key_parser_chain_var: %d\n",
		s->entry_index, (unsigned)s->offset, (unsigned)s->next_offset,
		(unsigned)s->entry_size, (unsigned)s->key_size,
		s->key_parser_chain_var);
	if (s->key) {
		fprintf(f, "    key: '");
		edict_idx_fputs(s->key, s->key_size, f);
	}
	if (s->entry) {
		fprintf(f, "'\n    entry: '");
		edict_idx_fputs(s->entry, s->entry_size, f);
	}
	fprintf(f, "'\n</parser>\n\n");
}
