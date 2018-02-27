
// Minimal SHA256 implementation by david@davidgf.net [2018]

#include <stdint.h>
#include <string.h>
#include "sha256.h"
#include "hash_common.h"
#include "common.h"

#define rotr(x, a) (((x) >> (a)) | ((x) << (32-(a))))

const uint32_t sha256k[] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
};

void sha256_transform(uint32_t *state, const void *data) {
	uint32_t w[16];
	const uint32_t * dui = (uint32_t*)data;
	for (unsigned i = 0; i < 16; i++)
		w[i] = read32be(dui[i]);

	uint32_t ls[8];
	memcpy(ls, state, sizeof(ls));

	for (unsigned i = 0; i < 64; i++) {
		unsigned widx = i & 15;

		uint32_t s1 = rotr(ls[4], 6) ^ rotr(ls[4], 11) ^ rotr(ls[4], 25);
		uint32_t ch = (ls[4] & ls[5]) ^ ((~ls[4]) & ls[6]);
		uint32_t t1 = ls[7] + s1 + ch + sha256k[i] + w[widx];
		uint32_t s0 = rotr(ls[0], 2) ^ rotr(ls[0], 13) ^ rotr(ls[0], 22);
		uint32_t mj = (ls[0] & ls[1]) ^ (ls[0] & ls[2]) ^ (ls[1] & ls[2]);
		uint32_t t2 = s0 + mj;

		uint32_t w1  = w[(i+ 1)&15];
		uint32_t w9  = w[(i+ 9)&15];
		uint32_t w14 = w[(i+14)&15];

		w[widx] += (w9 +
	        (rotr(w1,   7) ^ rotr(w1,  18) ^ (w1  >>  3)) + 
    	    (rotr(w14, 17) ^ rotr(w14, 19) ^ (w14 >> 10)));

		ls[7] = ls[6];
		ls[6] = ls[5];
		ls[5] = ls[4];
		ls[4] = ls[3] + t1;
		ls[3] = ls[2];
		ls[2] = ls[1];
		ls[1] = ls[0];
		ls[0] = t1 + t2;
	}

	for (unsigned i = 0; i < 8; i++)
		state[i] += ls[i];
}

static const uint32_t sha256_kinit[] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
};

static const t_hash_fns sha256desc = {
	.transformfn = sha256_transform,
	.init_state = sha256_kinit,
	.hashsize = 0x80 | 32,
};

// Get the sha1sum for a buffer
void sha256sum(const uint8_t *inbuffer, unsigned length, void *output) {
	uint32_t *state = (uint32_t*)output;
	hash32_sum(&sha256desc, inbuffer, length, output);

	// Output conversion
	for (unsigned i = 0; i < 8; i++)
		state[i] = read32be(state[i]);
}


