/*@-namechecks@*/
#define _POSIX_SOURCE
/* needs MAP_ANONYMOUS */
#define _BSD_SOURCE
/*@=namechecks@*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

#include "edictidx.h"
#include "edictidx_int.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

/*@-bitwisesigned@*/

void* edict_idx_mmap_create(FILE* f, size_t size)
{
	int fd;
	int flags;

	void* p;

	if (f) {
		fd = fileno(f);
		if (ftruncate(fd, (off_t)size) < 0) {
			fprintf(stderr, "Can't resize file: %s\n",
				strerror(errno));
			return 0;
		}
		flags = MAP_SHARED;
	} else {
		fd = -1;
		flags = MAP_PRIVATE | MAP_ANONYMOUS;
	}

	p = mmap(0, size, PROT_READ | PROT_WRITE,
		 flags, fd, 0);

	if (p == MAP_FAILED) {
		fprintf(stderr, "Can't mmap file: %s\n",
			strerror(errno));
		return 0;
	}

	return p;
}

/*@=bitwisesigned@*/

/*@-bitwisesigned@*/
/*@-mustmod@*/

void* edict_idx_mmap_open(FILE* f, size_t* psize)
{
	struct stat st;

	int fd;
	size_t sz;
	void* p;

	*psize = 0;

	fd = fileno(f);
	if (fstat(fd, &st) < 0) {
		fprintf(stderr, "Can't stat file: %s\n",
			strerror(errno));
		return 0;
	}
	sz = (size_t)st.st_size;

	p = mmap(0, sz, PROT_READ, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED) {
		fprintf(stderr, "Can't mmap file: %s\n",
			strerror(errno));
		return 0;
	}

	*psize = sz;

	return p;
}

/*@=mustmod@*/
/*@=bitwisesigned@*/

int edict_idx_mmap_resize(FILE* f, void** pp,
			  size_t size, size_t old_size)
{
	void* np;

	/*@-nullstate@*/

	/*@-unqualifiedtrans@*/
	if (edict_idx_mmap_unmap(f, *pp, old_size) < 0) {
		*pp = 0;
		return -1;
	}
	/*@=unqualifiedtrans@*/

	np = edict_idx_mmap_create(f, size);
	if (!np) {
		*pp = edict_idx_mmap_create(f, old_size);
		return -1;
	}

	*pp = np;
	return 0;

	/*@=nullstate@*/
}

/*@-mustmod@*/
int edict_idx_mmap_flush(FILE* f, void* p, size_t sz)
{
	f = f;
	p = p;
	sz = sz;

	return 0;
}
/*@=mustmod@*/

int edict_idx_mmap_unmap(FILE* f, void* p, size_t sz)
{
	f = f;

	return munmap(p, sz);
}
