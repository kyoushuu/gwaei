#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "edictidx.h"
#include "edictidx_int.h"

/*@access edict_idx, edict_idx_query @*/

int edict_idx_verify(edict_idx* s, edict_idx_key_types_t key_type,
		     edict_idx_key_fn_t key_parser)
{
	edict_idx_parser* p;

	str_p key;
	size_t key_sz;

	char* entry;
	size_t entry_sz = s->params.max_entry_size;

	unsigned keys = 0, results = 0, max_list = 0;
	int failed = 0;
	unsigned list = 0;
	uint32_t offset, result_offset;

	key_parser = edict_idx_select_key(key_type, key_parser);
	if (!key_parser)
		return -1;

	/*@-temptrans@*/
	p = edict_idx_parser_new(s, key_parser);
	/*@=temptrans@*/
	if (!p)
		return -1;

	entry = (char*)malloc(entry_sz);
	if (!entry) {
		edict_idx_parser_close(p);
		return -1;
	}

	while ((key = edict_idx_parser_fetch_key(p, &key_sz, &offset))) {
		edict_idx_query* q;
		int found = 0;

#ifdef DUMP
		edict_idx_parser_dump(p, stderr);
#endif
		list = 0;
		keys++;

		/*@-temptrans@*/
		q = edict_idx_find(s, key, key_sz);
		/*@=temptrans@*/
		if (!q) {
			failed = 1;
			break;
		}

#ifdef DUMP
		edict_idx_query_dump(q, stderr);
#endif
		result_offset = q->offset;
		while (edict_idx_query_result(q, entry, entry_sz) > 0) {
#ifdef DUMP
			edict_idx_query_dump(q, stderr);
#endif
			results++;
			list++;
			if (offset == result_offset) {
				found = 1;
				/*@innerbreak@*/ break;

			}

			result_offset = q->offset;
		}

		edict_idx_query_close(q);

		if (list > max_list)
			max_list = list;

		if (list >= s->params.max_list)
			continue;

		if (found == 0) {
			failed = 1;
			break;
		}
	}

	free(entry);
	edict_idx_parser_close(p);

	if (failed == 0) {
		fprintf(stderr, "Index verified: %u keys -> %u results, max list %u\n",
		       keys, results, max_list);
		return 0;
	} else {
		if (key) {
			fprintf(stderr, "No entry for key '");
			edict_idx_fputs(key, key_sz, stderr);
			fprintf(stderr, "' [%u]\n", list);
		}
		fprintf(stderr, "Index verification failed\n");
		return -1;
	}
}
