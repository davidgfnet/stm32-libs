
// Minimal SHA256 implementation by david@davidgf.net [2017]

#include <stdint.h>
#include <string.h>
#include "sha1.h"
#include "hash_common.h"
#include "common.h"

#define rotl(x, a) (((x) << (a)) | ((x) >> (32-(a))))

void sha1_transform(uint32_t *state, const void *data) {
	uint32_t m[16];
	uint32_t *dui = (uint32_t*)data;

	for (unsigned i = 0; i < 16; i++)
		m[i] = read32be(dui[i]);

	uint32_t a = state[0];
	uint32_t b = state[1];
	uint32_t c = state[2];
	uint32_t d = state[3];
	uint32_t e = state[4];

	for (unsigned i = 0; i < 80; i++) {
		unsigned idx = i & 15;

		uint32_t t;
		if      (i < 20) t = 0x5a827999 + rotl(a, 5) + ((b & c) ^ (~b & d)) + e + m[idx];
		else if (i < 40) t = 0x6ed9eba1 + rotl(a, 5) + (b ^ c ^ d) + e + m[idx];
		else if (i < 60) t = 0x8f1bbcdc + rotl(a, 5) + ((b & c) ^ (b & d) ^ (c & d)) + e + m[idx];
		else             t = 0xca62c1d6 + rotl(a, 5) + (b ^ c ^ d) + e + m[idx];

		e = d;
		d = c;
		c = rotl(b, 30);
		b = a;
		a = t;

		// Process w[], we only need a 16 entry buffer really
		m[idx] = (m[(i - 3) & 15] ^ m[(i - 8) & 15] ^ m[(i - 14) & 15] ^ m[(i - 16) & 15]);
		m[idx] = rotl(m[idx], 1);
	}

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
}

static const uint32_t sha1_kinit[5] = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xc3d2e1f0};

static const t_hash_fns sha1desc = {
	.transformfn = sha1_transform,
	.init_state = sha1_kinit,
	.hashsize = 0x80 | 20,
};

// Get the sha1sum for a buffer
void sha1sum(const uint8_t *inbuffer, unsigned length, void *output) {
	uint32_t state[5];
	hash32_sum(&sha1desc, inbuffer, length, state);

	// Output conversion
	uint32_t *out32 = (uint32_t*)output;
	for (unsigned i = 0; i < 5; i++)
		out32[i] = read32be(state[i]);
}

