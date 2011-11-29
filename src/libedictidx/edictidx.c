#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "edictidx.h"
#include "edictidx_int.h"

/*@access edict_idx, edict_idx_file @*/

edict_idx*
edict_idx_open(const char* idx_fname,
	       const char* dict_fname,
	       unsigned flags)
{
	if (!dict_fname)
		return 0;

	return edict_idx_new(idx_fname, dict_fname, 0, flags);
}

edict_idx*
edict_idx_share(const char* idx_fname,
		const edict_idx* parent,
		unsigned flags)
{
	if (!parent || !parent->dict)
		return 0;

        return edict_idx_new(idx_fname, 0, parent, flags);
}

edict_idx*
edict_idx_new(const char* idx_fname,
	      const char* dict_fname,
	      const edict_idx* parent,
	      unsigned flags)
{
	edict_idx* s = (edict_idx*)malloc(sizeof(*s));
	if (!s)
		return 0;

	edict_idx_params_init(&s->params);

	if (dict_fname) {
		s->dict_file = edict_idx_dict_open(dict_fname);
		if (!s->dict_file) {
			free(s);
			return 0;
		}
		s->dict = (str_p)s->dict_file->mem;
		s->dict_size = s->dict_file->size;
	} else if (parent) {
		s->dict_file = 0;

		s->dict = parent->dict;
		s->dict_size = parent->dict_size;
	} else {
		free(s);
		return 0;
	}

	if ((flags & F_EDICT_IDX_CREATE) != 0) {
		if ((flags & F_EDICT_IDX_IN_MEMORY) != 0) {
			s->htab_file = edict_idx_index_create(0, s->params.min_idx_size);
		} else if (idx_fname) {
			s->htab_file = edict_idx_index_create(idx_fname, s->params.min_idx_size);
		} else
			s->htab_file = 0;
	} else {
		if (idx_fname) {
			s->htab_file = edict_idx_index_open(idx_fname);
		} else
			s->htab_file = 0;
	}

	if (!s->htab_file) {
		if (s->dict_file)
			(void)edict_idx_file_close(s->dict_file);
		free(s);
		return 0;
	}

	s->htab = (struct edict_idx_htab_rec*)s->htab_file->mem;
	s->htab_size = (htab_index_t)(s->htab_file->size / sizeof(*s->htab));

	return s;
}

void edict_idx_close(edict_idx* s)
{
	(void)edict_idx_file_close(s->htab_file);
	if (s->dict_file)
		(void)edict_idx_file_close(s->dict_file);
	free(s);
}

void edict_idx_params_init(struct edict_idx_params* s)
{
	s->min_idx_size = C_EDICT_MIN_IDX_SIZE;
	s->max_idx_size = C_EDICT_MAX_IDX_SIZE;

	s->max_entry_size = C_EDICT_MAX_ENTRY_SIZE;

	s->max_chain = C_EDICT_MAX_CHAIN_SIZE;
	s->max_list = C_EDICT_MAX_LIST_SIZE;
}

int edict_idx_get_parameter(edict_idx* s, edict_idx_parameter_t par)
{
	switch (par) {
	case P_EDICT_IDX_MIN_SIZE:
		return (int)s->params.min_idx_size;
	case P_EDICT_IDX_MAX_SIZE:
		return (int)s->params.max_idx_size;
	case P_EDICT_IDX_MAX_ENTRY_SIZE:
		return (int)s->params.max_entry_size;
	case P_EDICT_IDX_MAX_CHAIN:
		return (int)s->params.max_chain;
	case P_EDICT_IDX_MAX_LIST:
		return (int)s->params.max_list;
	default:
		return -1;
	}
}

int edict_idx_set_parameter(edict_idx* s, edict_idx_parameter_t par, int value)
{
	switch (par) {
	case P_EDICT_IDX_MIN_SIZE:
		s->params.min_idx_size = (size_t)value;
		return 0;
	case P_EDICT_IDX_MAX_SIZE:
		s->params.max_idx_size = (size_t)value;
		return 0;
	case P_EDICT_IDX_MAX_ENTRY_SIZE:
		s->params.max_entry_size = (size_t)value;
		return 0;
	case P_EDICT_IDX_MAX_CHAIN:
		s->params.max_chain = (unsigned)value;
		return 0;
	case P_EDICT_IDX_MAX_LIST:
		s->params.max_list = (unsigned)value;
		return 0;
	default:
		return -1;
	}
}
