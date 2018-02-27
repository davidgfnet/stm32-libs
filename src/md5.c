
// Minimal MD5 implementation by david@davidgf.net [2018]

#include <stdint.h>
#include <string.h>
#include "hash_common.h"
#include "md5.h"
#include "common.h"

// Small implementation is slower by ~11% but saves 100+ bytes
#define SMALLEST_IMPL

static uint32_t md5k[] = {
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
  0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
  0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
  0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
  0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
  0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
  0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
  0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
  0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
  0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
  0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
  0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
  0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
  0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
  0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
  0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

#ifdef SMALLEST_IMPL
static uint8_t md5sa_[4][4] = {
  {7, 12, 17, 22},
  {5,  9, 14, 20},
  {4, 11, 16, 23},
  {6, 10, 15, 21},
};
#define md5sa(i) (md5sa_[(i) >> 4][(i) & 3])
#define md5idx(i) (i < 16) ? i          : \
                  (i < 32) ? (5*i+1)&15 : \
                  (i < 48) ? (3*i+5)&15 : \
                             (7*i)&15;
#else
static uint8_t md5sa_[64] = {
  7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
  5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
  4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
  6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
};
#define md5sa(i) (md5sa_[(i)])
static uint8_t md5idx_[64] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
  1, 6, 11, 0, 5, 10, 15, 4, 9, 14, 3, 8, 13, 2, 7, 12,
  5, 8, 11, 14, 1, 4, 7, 10, 13, 0, 3, 6, 9, 12, 15, 2,
  0, 7, 14, 5, 12, 3, 10, 1, 8, 15, 6, 13, 4, 11, 2, 9,
};
#define md5idx(i) (md5idx_[i])
#endif

#define rotl(x, a) (((x) << (a)) | ((x) >> (32-(a))))

static const uint32_t md5_kinit[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};

void md5_transform(uint32_t *state, const void *data) {
	uint32_t *dui = (uint32_t*)data;

	uint32_t a = state[0];
	uint32_t b = state[1];
	uint32_t c = state[2];
	uint32_t d = state[3];

	for (unsigned i = 0; i < 64; i++) {
		uint32_t t = (i < 16) ? (b & c) | (~b & d) :
		             (i < 32) ? (d & b) | (~d & c) :
		             (i < 48) ? b ^ c ^ d          :
		                        c ^ (b | ~d);

		uint32_t g = md5idx(i);

		t += a + md5k[i] + read32le(dui[g]);
		a = d;
		d = c;
		c = b;
		b += rotl(t, md5sa(i));
	}

	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

static const t_hash_fns md5desc = {
	.transformfn = md5_transform,
	.init_state = md5_kinit,
	.hashsize = 16,
};

// Get the sha1sum for a buffer
void md5sum(const uint8_t *inbuffer, unsigned length, void *output) {
	uint32_t *state = (uint32_t*)output;
	hash32_sum(&md5desc, inbuffer, length, output);

	// Output conversion
	for (unsigned i = 0; i < 4; i++)
		state[i] = read32le(state[i]);
}

