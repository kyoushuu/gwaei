#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../edictidx.h"
#include "../edictidx_int.h"

#pragma GCC diagnostic ignored "-Wunused-variable"

int main(int argc, char* argv[])
{
	edict_idx* id;
	edict_idx_parser* p;
	edict_idx_query* q;
	str_p key;
	size_t key_sz;
	uint32_t offset;
	char keybuf[1024];
	char entry[4096];
	edict_idx_key_types_t key_t;
	edict_idx_key_fn_t key_parser = edict_idx_key_kanji_exact;
	int i;
	uint32_t hash, cksum;

#ifdef HASH_TEST
	if (argc < 2)
		return 1;

	key = argv[1];
	key_sz = strlen(key);

	for (i = 0; i < 1000000; i++) {
		hash = edict_idx_hash(key, key_sz);
		cksum = edict_idx_cksum(key, key_sz);
	}

	printf("%08X ", (unsigned)hash);
	printf("%08X\n", (unsigned)cksum);
#endif

#ifdef PARSER_TEST
	if (argc < 2)
		return -1;

	id = edict_idx_open(0, argv[1], F_EDICT_IDX_CREATE | F_EDICT_IDX_IN_MEMORY);
	if (!id)
		return 2;

	if (argc > 2) {
		key_parser = edict_idx_select_key((edict_idx_key_types_t)atoi(argv[2]),
						  key_parser);
		if (!key_parser)
			return 3;
	}

	if (argc > 3)
		edict_idx_set_parameter(id, P_EDICT_IDX_MAX_ENTRY_SIZE,
					atoi(argv[3]));

	p = edict_idx_parser_new(id, key_parser);
	if (!p)
		return 4;

	while ((key = edict_idx_parser_fetch_key(p, &key_sz, &offset))) {
		edict_idx_parser_dump(p, stdout);
	}

	edict_idx_parser_close(p);
	edict_idx_close(id);
#endif

#ifdef BUILD_TEST
	if (argc < 2)
		return 1;

	if (argc < 3)
		id = edict_idx_open(0, argv[1], F_EDICT_IDX_CREATE | F_EDICT_IDX_IN_MEMORY);
	else
		id = edict_idx_open(argv[2], argv[1], F_EDICT_IDX_CREATE);

	if (!id)
		return 2;

	if (argc >= 4)
		key_t = (edict_idx_key_types_t)atoi(argv[3]);
	else
		key_t = T_EDICT_IDX_KEY_KANJI;

	if (argc >= 5)
		edict_idx_set_parameter(id, P_EDICT_IDX_MAX_LIST, atoi(argv[4]));

	if (argc >= 6)
		edict_idx_set_parameter(id, P_EDICT_IDX_MAX_CHAIN, atoi(argv[5]));

	if (argc >= 7)
		edict_idx_set_parameter(id, P_EDICT_IDX_MAX_ENTRY_SIZE,
					atoi(argv[6]));

	if (edict_idx_build(id, key_t, 0) < 0)
		return 3;

	if (edict_idx_verify(id, key_t, 0) < 0)
		return 4;

	edict_idx_close(id);
#endif

#ifdef VERIFY_TEST
	if (argc < 3)
		return 1;

	id = edict_idx_open(argv[2], argv[1], F_EDICT_IDX_OPEN);
	if (!id)
		return 2;

	if (argc >= 4)
		key_t = (edict_idx_key_types_t)atoi(argv[3]);
	else
		key_t = T_EDICT_IDX_KEY_KANJI;

	if (argc >= 5)
		edict_idx_set_parameter(id, P_EDICT_IDX_MAX_LIST, atoi(argv[4]));

	if (argc >= 6)
		edict_idx_set_parameter(id, P_EDICT_IDX_MAX_CHAIN, atoi(argv[5]));

	if (argc >= 7)
		edict_idx_set_parameter(id, P_EDICT_IDX_MAX_ENTRY_SIZE,
					atoi(argv[6]));

	fprintf(stderr, "Verifying index...\n");
	edict_idx_verify(id, key_t, 0);

	edict_idx_close(id);
	return 0;
#endif

#ifdef QUERY_TEST
	if (argc < 4)
		return 1;

	id = edict_idx_open(argv[2], argv[1], F_EDICT_IDX_OPEN);
	if (!id)
		return 2;

	q = edict_idx_find(id, argv[3], strlen(argv[3]));
	if (q) {
		while (edict_idx_query_result(q, entry, sizeof(entry))) {
			puts(entry);
		}
		edict_idx_query_close(q);
	}

	edict_idx_close(id);
#endif
	return 0;
}
