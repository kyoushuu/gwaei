#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "edictidx.h"
#include "edictidx_int.h"

edict_idx_key_fn_t
edict_idx_select_key(edict_idx_key_types_t key_type,
		     edict_idx_key_fn_t key_parser)
{
	switch (key_type) {
	case T_EDICT_IDX_KEY_USER:
		return key_parser;
	case T_EDICT_IDX_KEY_KANJI:
		return edict_idx_key_kanji_exact;
	case T_EDICT_IDX_KEY_KANA:
		return edict_idx_key_kana_exact;
	case T_EDICT_IDX_KEY_ENGLISH:
		return edict_idx_key_english_exact;
	default:
		return F_KEY_NULL;
	}
}

str_p edict_idx_key_kanji_exact(size_t* pkey_sz, str_p entry, size_t entry_sz,
				int* ppos)
{
	size_t i;

	*pkey_sz = 0;

	if (*ppos == 1)
		return 0;
	else
		*ppos = 1;

	for (i = 1; i < entry_sz; i++) {
		if (entry[i] == ' ') {
			*pkey_sz = i;
			return entry;
		}
	}

	return 0;
}

str_p edict_idx_key_kana_exact(size_t* pkey_sz, str_p entry, size_t entry_sz,
			       int* ppos)
{
	size_t li, si, ri;

	*pkey_sz = 0;

	if (*ppos == 1)
		return 0;
	else
		*ppos = 1;

	for (li = 0; li < entry_sz - 2; li++) {
		if (entry[li] == '[')
			break;
	}

	si = li + 1;

	for (ri = si + 1; ri < entry_sz; ri++) {
		if (entry[ri] == ']') {
			*pkey_sz = ri - si;
			return &entry[si];
		}
	}

	return 0;
}

static
size_t skip_parens(str_p s, size_t si, size_t sz) /*@*/
{
	size_t pi, ei;

	for (pi = si; pi < sz; pi = ei) {
		if (s[pi] != '(')
			return pi;
		for (ei = pi + 1; ei < sz; ei++) {
			if (s[ei] == ')')
				/*@innerbreak@*/
				break;
		}
		/* skip trailing whitespace */
		for (ei = ei + 1; ei < sz; ei++) {
			if (s[ei] != ' ')
				/*@innerbreak@*/
				break;
		}
	}

	return sz;
}

str_p edict_idx_key_english_exact(size_t* pkey_sz, str_p entry, size_t entry_sz,
				  int* ppos)
{
	size_t ci, li, si, ri;

	*pkey_sz = 0;

	ci = (size_t)*ppos;

	for (li = ci; li < entry_sz; li++) {
		if (entry[li] == '/')
			break;
	}

	for (ri = li + 1; ri < entry_sz; ri++) {
		if (entry[ri] == '/')
			break;
	}

	if (ri >= entry_sz)
		return 0;

	si = skip_parens(entry, li + 1, ri);

	*ppos = (int)ri;
	*pkey_sz = ri - si;
	return &entry[si];
}
