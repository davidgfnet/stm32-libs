
// Common hashing functions

#include <stdint.h>
#include <string.h>
#include "hash_common.h"
#include "common.h"

void hash32_sum(const t_hash_fns *desc, const uint8_t *inbuffer, unsigned length, void *output) {
	uint32_t *state = (uint32_t*)output;
	uint64_t bitlen = length << 3;

	// Init state
	memcpy(state, desc->init_state, desc->hashsize & 0x7f);

	while (length >= 64) {
		desc->transformfn(state, inbuffer);
		inbuffer += 64;
		length -= 64;
	}

	// Last bits
	union {
		uint8_t chars[64];
		uint32_t u32[16];
		uint64_t u64[8];
	} tmp = {0};
	memcpy(tmp.chars, inbuffer, length);
	tmp.chars[length] = 0x80;

	if (length >= 56) {
		// Need an extra block
		desc->transformfn(state, tmp.u32);
		memset(tmp.chars, 0, 64);
	}

	tmp.u64[7] = (desc->hashsize & 0x80) ? read64be(bitlen) : read64le(bitlen);
	desc->transformfn(state, tmp.u32);
}

