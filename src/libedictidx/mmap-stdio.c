#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "edictidx.h"
#include "edictidx_int.h"

void*
edict_idx_mmap_create(FILE* f, size_t size)
{
	f = f;

	/*@-compdef@*/
	return malloc(size);
	/*@=compdef@*/
}

void* edict_idx_mmap_open(FILE* f, size_t* psize)
{
	size_t sz;
	void* p;

	*psize = 0;

	if (fseek(f, 0, SEEK_END) == EOF)
		return 0;
	sz = (size_t)ftell(f);
	(void)fseek(f, 0, SEEK_SET);

	p = malloc(sz);
	if (!p)
		return 0;

	if (fread(p, (size_t)1, sz, f) != sz) {
		fprintf(stderr, "Can't load file: %s\n",
			strerror(errno));
		free(p);
		return 0;
	}

	*psize = (size_t)sz;

	return p;
}

int edict_idx_mmap_resize(FILE* f, void** p, size_t size)
{
	void* np;

	f = f;

/*@-unqualifiedtrans@*/
	np = realloc(*p, size);
/*@=unqualifiedtrans@*/

	if (np) {
		*p = np;
		return 0;
	} else
		return -1;
}

int edict_idx_mmap_flush(FILE* f, void* p, size_t sz)
{
	(void)fseek(f, 0, SEEK_SET);

	if (fwrite(p, (size_t)1, sz, f) != sz
	    || fflush(f) != 0) {
		fprintf(stderr, "Error writing file: %s\n",
			strerror(errno));
		return -1;
	}

	return 0;
}

int edict_idx_mmap_unmap(FILE* f, void* p, size_t sz)
{
	f = f;
	sz = sz;

	free(p);
	return 0;
}
