#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "edictidx.h"
#include "edictidx_int.h"

size_t edict_idx_fputs(str_p s, size_t sz, FILE* f)
{
	return fwrite(s, (size_t)1, sz, f);
}

/*@access edict_idx_file @*/

edict_idx_file*
edict_idx_index_create(const char* fname, size_t size)
{
	edict_idx_file* s =
		(edict_idx_file*)malloc(sizeof(*s));

	if (!s)
		return 0;

	if (fname) {
		s->file = fopen(fname, "w+b");
		if (!s->file) {
			fprintf(stderr, "Can't create index file %s: %s\n",
				fname, strerror(errno));
			free(s);
			return 0;
		}
	} else
		s->file = (FILE*)0;

	s->size = size;
	s->mem = edict_idx_mmap_create(s->file, s->size);
	if (!s->mem) {
		if (s->file)
			(void)fclose(s->file);
		free(s);
		return 0;
	}

	s->flags = F_FILE_MUTABLE;

	return s;
}

edict_idx_file*
edict_idx_index_open(const char* fname)
{
	edict_idx_file* s =
		(edict_idx_file*)malloc(sizeof(*s));

	if (!s)
		return 0;

	s->file = fopen(fname, "rb");
	if (!s->file) {
		fprintf(stderr, "Can't open index file %s: %s\n",
			fname, strerror(errno));
		free(s);
		return 0;
	}

 	s->mem = edict_idx_mmap_open(s->file, &s->size);
	if (!s->mem) {
		(void)fclose(s->file);
		free(s);
		return 0;
	}

	if (s->size == 0 || (s->size & (s->size - 1)) != 0) {
		fprintf(stderr, "Bad index file '%s': invalid hash table size %lu\n",
			fname, (unsigned long)s->size);
		(void)edict_idx_mmap_unmap(s->file, s->mem, s->size);
		(void)fclose(s->file);
		free(s);
		return 0;
	}

	s->flags = 0;

	return s;
}

edict_idx_file*
edict_idx_dict_open(const char* fname)
{
	edict_idx_file* s =
		(edict_idx_file*)malloc(sizeof(*s));

	if (!s)
		return 0;

	s->file = fopen(fname, "rb");
	if (!s->file) {
		fprintf(stderr, "Can't open dict file '%s': %s\n",
			fname, strerror(errno));
		free(s);
		return 0;
	}

 	s->mem = edict_idx_mmap_open(s->file, &s->size);
	if (!s->mem) {
		(void)fclose(s->file);
		free(s);
		return 0;
	}

	s->flags = 0;

	return s;
}

int edict_idx_index_resize(edict_idx_file* s, size_t new_size)
{
	int r;

	r = edict_idx_mmap_resize(s->file, &s->mem,
				  new_size, s->size);

	if (r < 0)
		return -1;

	s->size = new_size;

	return 0;
}

int edict_idx_file_close(edict_idx_file* s)
{
	if (s->file)
		if ((s->flags & F_FILE_MUTABLE) != 0)
			(void)edict_idx_mmap_flush(s->file, s->mem, s->size);

	(void)edict_idx_mmap_unmap(s->file, s->mem, s->size);

	if (s->file)
		(void)fclose(s->file);

	free(s);
	return 0;
}
