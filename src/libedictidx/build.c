#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "edictidx.h"
#include "edictidx_int.h"

/*@access edict_idx, edict_idx_file @*/

int edict_idx_build(edict_idx* s,
		    edict_idx_key_types_t key_type,
		    edict_idx_key_fn_t key_parser
)
{
	int r;

	key_parser = edict_idx_select_key(key_type, key_parser);
	if (!key_parser)
		return -1;

	while ((r = edict_idx_htab_fill(s, key_parser, 0)) != 0) {
		if (r < 0)
			return -1;
		if (edict_idx_htab_resize2(s) < 0)
			return -1;
	}

	return 0;
}

int edict_idx_htab_resize2(edict_idx* s)
{
	fprintf(stderr, "Resizing hash table %u k -> %u k bins\n",
	       (unsigned)s->htab_size/1024, (unsigned)s->htab_size*2/1024);
	s->htab_size *= 2;

	if (s->htab_size*sizeof(*s->htab) > s->params.max_idx_size) {
		fprintf(stderr, "Maximum index size %u kb reached\n",
			(unsigned)s->params.max_idx_size/1024);
		return -1;
	}

	if (edict_idx_index_resize(s->htab_file, s->htab_size*sizeof(*s->htab)) < 0) {
		fprintf(stderr, "Can't resize hash table to %u kb\n",
			(unsigned)(s->htab_size*sizeof(*s->htab))/1024);
		return -1;
	}

	s->htab = s->htab_file->mem;

	return 0;
}

int edict_idx_htab_fill(edict_idx* s, edict_idx_key_fn_t key_parser,
			unsigned max_list)
{
	edict_idx_parser* p;

	unsigned max_chain = 0;
	unsigned entries = 0;

	str_p key;
	size_t key_sz;

	uint32_t offset;

	/*@-temptrans@*/
	p = edict_idx_parser_new(s, key_parser);
	/*@=temptrans@*/

	if (!p)
		return -1;

	memset(s->htab, 0, sizeof(*s->htab)*s->htab_size);

	while ((key = edict_idx_parser_fetch_key(p, &key_sz, &offset))) {
		uint32_t hash, cksum;
		htab_index_t hi, hi0;
		uint32_t dup_offset;
		unsigned chain = 0, list = 0;

#ifdef DUMP
		edict_idx_parser_dump(p, stderr);
#endif

		hash = edict_idx_hash(key, key_sz);
		hi = hi0 = edict_idx_htab_index(s, hash);
		cksum = edict_idx_cksum(key, key_sz);

		while (edict_idx_htab_look(s, &hi, &dup_offset, cksum) == 0) {
			list++;
		}

		if (list >= s->params.max_list)
			continue;

		s->htab[hi].cksum = cksum;
		s->htab[hi].offset = offset;
		list++;

		chain = (unsigned)edict_idx_htab_index(s, (uint32_t)(hi - hi0));

		if (chain > max_chain)
			max_chain = chain;
		if (list > max_list)
			max_list = list;

#ifdef DUMP
		fprintf(stderr, "<htentry>\n    hash: 0x%08X cksum: 0x%08X "
			"hi: 0x%08X hi0: 0x%08X chain: %u list: %u\n</htentry>\n\n",
			hash, cksum, hi, hi0, chain, list);
#endif

		if (++entries >= (s->htab_size*15/16)
		    || max_chain > s->params.max_chain) {
			edict_idx_parser_close(p);

			fprintf(stderr, "Scrapping hash table index: "
				"fill %u/%u, max chain %u, max list %u\n",
				entries, (unsigned)s->htab_size,
				max_chain, max_list);

			return 1;
		}
	}

	edict_idx_parser_close(p);

	if (offset != s->dict_size) {
		fprintf(stderr, "Indexing failed : "
			"only %u of %u dictionary bytes processed\n",
			(unsigned)offset, (unsigned)s->dict_size);
		return -1;
	}

	fprintf(stderr, "Created hash table index: "
		"fill %u/%u, max chain %u, max list %u\n",
		entries, (unsigned)s->htab_size, max_chain, max_list);

	return 0;
}
