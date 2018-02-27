
// SHA1 size optimized implementation
// Copyright 2017 David Guillen Fandos <david@davidgf,net>

#ifndef __SHA1__H__
#define __SHA1__H__

#include <string.h>
#include <stdint.h>

#define SHA1_DIGEST_SIZE 20
void sha1_transform(uint32_t *state, const void *data);
void sha1sum(const uint8_t *inbuffer, unsigned length, void *output);

#endif

