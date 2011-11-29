#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "edictidx.h"
#include "edictidx_int.h"

/*@access edict_idx, edict_idx_query @*/

htab_index_t edict_idx_htab_index(edict_idx* s, uint32_t hash)
{
	return ((htab_index_t)hash & (s->htab_size - 1));
}

int edict_idx_htab_look(edict_idx* s, htab_index_t* pindex,
			uint32_t* poffset, uint32_t cksum)
{
	htab_index_t hi = *pindex;

	while (s->htab[hi].cksum != cksum) {
		if (s->htab[hi].cksum == 0
		    && s->htab[hi].offset == 0) {
			*pindex = hi;
			*poffset = 0;
			return -1;
		}
		hi = (hi + 1) & (s->htab_size - 1);
	}

	*poffset = s->htab[hi].offset;

	hi = (hi + 1) & (s->htab_size - 1);
	*pindex = hi;

	return 0;
}

edict_idx_query*
edict_idx_find(edict_idx* idx, const char* key, size_t key_sz)
{
	edict_idx_query* s;
	uint32_t hash, cksum;
	uint32_t offset;
	htab_index_t hi;

	hash = edict_idx_hash(key, key_sz);
	cksum = edict_idx_cksum(key, key_sz);

	hi = edict_idx_htab_index(idx, hash);
	if (edict_idx_htab_look(idx, &hi, &offset, cksum) < 0)
		return 0;

	s = (edict_idx_query*)malloc(sizeof(*s));
	if (!s)
		return 0;

	s->idx = idx;
	s->hash = hash;
	s->cksum = cksum;
	s->index = hi;
	s->offset = offset;
	s->next = 0;

	return s;
}

size_t
edict_idx_query_result(edict_idx_query* s, char* result_entry,
		       size_t max_result_sz)
{
	str_p entry;
	size_t entry_sz;
	size_t i;

	*result_entry = '\0';

	if (s->next < 0)
		return 0;

	entry = edict_idx_get_entry(s->idx, &entry_sz, &s->offset);
	if (!entry) {
		fprintf(stderr, "Bad dictionary offset %u: index corrupted\n",
			(unsigned)s->offset);
		return 0;
	}

	if (entry_sz + 1 > max_result_sz) {
		fprintf(stderr, "Query result does not fit in supplied output buffer %u > %u\n",
			(unsigned)entry_sz, (unsigned)max_result_sz);

		entry_sz = max_result_sz - 1;
	}

	s->next = edict_idx_htab_look(s->idx, &s->index, &s->offset, s->cksum);

	for (i = 0; i < entry_sz; i++) {
		result_entry[i] = entry[i];
	}
	result_entry[i] = '\0';

	return entry_sz;
}

void edict_idx_query_close(edict_idx_query* s)
{
	free(s);
}

void edict_idx_query_dump(edict_idx_query* s, FILE* f)
{
	fprintf(f, "<query>\n index: 0x%08X offset: 0x%08X "
		"hash: 0x%08X cksum: 0x%08X\n</query>\n\n",
		(unsigned)s->index, (unsigned)s->offset,
		(unsigned)s->hash, (unsigned)s->cksum);
}
