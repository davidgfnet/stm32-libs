/*
 * Copyright (C) 2017 David Guillen Fandos <david@davidgf.net>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

// Implementation of ChaCha20, bits borrowed here and there from Linux, the RFC
// and other random places

#include <stdint.h>
#include <string.h>

#include "chacha20.h"
#include "common.h"

#define SMALLEST_IMPL
// This is aproximate ~500 bytes in ARM thumb

static inline uint32_t rotl32(uint32_t v, uint8_t n) {
	return (v << n) | (v >> (32 - n));
}
static inline unsigned umin(unsigned a, unsigned b) {
	return a < b ? a : b;
}

// Very minimal implmentation of the core function, table based.
// This shouldn't be that slow for a micro
#ifdef SMALLEST_IMPL
const uint8_t map4[] = {16, 12, 8, 7};
const uint16_t fnmap[16] = {
	( 0 << 8) | ( 4 << 4) | (12 << 0),
	( 1 << 8) | ( 5 << 4) | (13 << 0),
	( 2 << 8) | ( 6 << 4) | (14 << 0),
	( 3 << 8) | ( 7 << 4) | (15 << 0),

	( 8 << 8) | (12 << 4) | ( 4 << 0),
	( 9 << 8) | (13 << 4) | ( 5 << 0),
	(10 << 8) | (14 << 4) | ( 6 << 0),
	(11 << 8) | (15 << 4) | ( 7 << 0),

	( 0 << 8) | ( 5 << 4) | (15 << 0),
	( 1 << 8) | ( 6 << 4) | (12 << 0),
	( 2 << 8) | ( 7 << 4) | (13 << 0),
	( 3 << 8) | ( 4 << 4) | (14 << 0),

	(10 << 8) | (15 << 4) | ( 5 << 0),
	(11 << 8) | (12 << 4) | ( 6 << 0),
	( 8 << 8) | (13 << 4) | ( 7 << 0),
	( 9 << 8) | (14 << 4) | ( 4 << 0),
};
#define get1(j) ( fnmap[j] >> 8)
#define get2(j) ((fnmap[j] >> 4) & 15)
#define get3(j) ( fnmap[j] & 15)

void chacha20_block(uint32_t *state, void *stream) {
	uint32_t x[16];
	memcpy(x, state, sizeof(uint32_t) * 16);

	for (unsigned i = 0; i < 20; i += 2) {
		for (unsigned j = 0; j < 32; j++) {
			unsigned idx = (j & 7) | ((j & 0x10) >> 1);
			unsigned idm = (j >> 2) & 3;

			unsigned i1 =  get1(idx);
			unsigned i2 =  get2(idx);
			unsigned i3 =  get3(idx);
			unsigned i4 =  map4[idm];
			x[i1] += x[i2];
			x[i3] = rotl32(x[i3] ^ x[i1], i4);
		}
	}

	uint32_t * out = (uint32_t*)stream;
	for (unsigned i = 0; i < 16; i++)
		// This only works for little endian systems :)
		out[i] = read32le(x[i] + state[i]);

	// Only works for 32 bit (max 4GB can be generated!)
	state[12]++;
}

#else

#define chstep(i1, i2, i3, i4) \
	x[i1] += x[i2]; \
	x[i3] = rotl32(x[i3] ^ x[i1], i4);

void chacha20_block(uint32_t *state, void *stream) {
	uint32_t x[16];
	for (unsigned i = 0; i < 16; i++)
		x[i] = read32le(((uint32_t*)stream)[i]);

	for (unsigned i = 0; i < 20; i += 2) {
		chstep( 0,  4, 12, 16);
		chstep( 1,  5, 13, 16);
		chstep( 2,  6, 14, 16);
		chstep( 3,  7, 15, 16);

		chstep( 8, 12,  4, 12);
		chstep( 9, 13,  5, 12);
		chstep(10, 14,  6, 12);
		chstep(11, 15,  7, 12);

		chstep( 0,  4, 12,  8);
		chstep( 1,  5, 13,  8);
		chstep( 2,  6, 14,  8);
		chstep( 3,  7, 15,  8);

		chstep( 8, 12,  4,  7);
		chstep( 9, 13,  5,  7);
		chstep(10, 14,  6,  7);
		chstep(11, 15,  7,  7);

		chstep( 0,  5, 15, 16);
		chstep( 1,  6, 12, 16);
		chstep( 2,  7, 13, 16);
		chstep( 3,  4, 14, 16);

		chstep(10, 15,  5, 12);
		chstep(11, 12,  6, 12);
		chstep( 8, 13,  7, 12);
		chstep( 9, 14,  4, 12);

		chstep( 0,  5, 15,  8);
		chstep( 1,  6, 12,  8);
		chstep( 2,  7, 13,  8);
		chstep( 3,  4, 14,  8);

		chstep(10, 15,  5,  7);
		chstep(11, 12,  6,  7);
		chstep( 8, 13,  7,  7);
		chstep( 9, 14,  4,  7);
	}

	uint32_t * out = (uint32_t*)stream;
	for (unsigned i = 0; i < 16; i++)
		// This only works for little endian systems :)
		out[i] = read32le(x[i] + state[i]);

	// Only works for 32 bit (max 4GB can be generated!)
	state[12]++;
}

#endif

void crypto_chacha20_init(uint32_t *state, uint32_t *key, uint64_t nonce64) {
	const uint32_t k[4] = {0x61707865, 0x3320646e, 0x79622d32, 0x6b206574};

	memcpy(&state[ 0],     k, sizeof(k));
	for (unsigned i = 0; i < CHACHA20_KEYSIZE/sizeof(uint32_t); i++)
		state[i+4] = read32le(key[i]);
	state[12] = 0;              // This gets incremented on every 16 byte block
	state[13] = 0;
	state[14] = nonce64;
	state[15] = nonce64 >> 32;  // This should be different for every block
}

// Inplace crypt/decrypt of a certain block
// Key is 32 bytes long and nonce is just 32 bit
// Input buffer and output can be the same pointer for in-place
void chacha20_encrypt_decrypt_block(const uint32_t *inbuffer, uint32_t *outbuffer, unsigned len, const void* key, uint64_t nonce64) {
	uint32_t crypto_state[16];
	crypto_chacha20_init(&crypto_state[0], (uint32_t*)key, nonce64);

	union {
		uint32_t u32[16];
		uint8_t   u8[64];
	} tmp;
	unsigned j;
	for (unsigned i = 0; i < len; i += 64) {
		chacha20_block(crypto_state, tmp.u32);
		// Just xor the data with the stream generated
		unsigned rlen4 = umin((len-i) >> 2, 16);
		for (j = 0; j < rlen4; j++)
			*outbuffer++ = *inbuffer++ ^ tmp.u32[j];
	}
	if (j < 16) {
		uint8_t *o8buffer = (uint8_t*)outbuffer;
		uint8_t *i8buffer = (uint8_t*)inbuffer;
		for (unsigned k = j*4 ; k < (len & 63); k++)
			*o8buffer++ = *i8buffer++ ^ tmp.u8[k];
	}
}

