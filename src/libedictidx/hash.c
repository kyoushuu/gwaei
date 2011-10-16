#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "edictidx.h"
#include "edictidx_int.h"

/*@constant uint32_t C_HASH_M1 @*/
#define C_HASH_M1  (uint32_t) 0xfd8b6c07
/*@constant uint32_t C_HASH_M2 @*/
#define C_HASH_M2  (uint32_t) 0x86eb73a1

/*@constant uint32_t C_CKSUM_M1 @*/
#define C_CKSUM_M1 (uint32_t) 0x4bc6fdd9
/*@constant uint32_t C_CKSUM_M2 @*/
#define C_CKSUM_M2 (uint32_t) 0x6098bf21

/*@constant uint32_t C_HASH_0 @*/
#define C_HASH_0 (uint32_t) 1

static inline
uint32_t mmhash(str_p p, size_t sz, uint32_t m1, uint32_t m2) /*@*/
{
	uint32_t t, h = C_HASH_0;
	size_t i;

	for (i = 0; i < sz; i++) {
		t = (uint8_t)p[i];
		t *= m1;
		h *= m2;
		t ^= t >> 16;
		h ^= h >> 16;

		t *= m2;
		h *= m1;
		t ^= t >> 16;
		h ^= h >> 16;

		h += t;
	}

	h *= m2;
	h ^= h >> 16;
	h *= m1;
	h ^= h >> 16;

	return h;
}

uint32_t edict_idx_hash(str_p p, size_t sz)
{
	return mmhash(p, sz, C_HASH_M1, C_HASH_M2);
}

uint32_t edict_idx_cksum(str_p p, size_t sz)
{
	return mmhash(p, sz, C_CKSUM_M1, C_CKSUM_M2);
}
