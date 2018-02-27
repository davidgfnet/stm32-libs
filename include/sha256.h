
// SHA256 size optimized implementation
// Copyright 2018 David Guillen Fandos <david@davidgf,net>

#ifndef __SHA256__H__
#define __SHA256__H__

#include <string.h>
#include <stdint.h>

#define SHA256_DIGEST_SIZE 32
void sha256sum(const uint8_t *inbuffer, unsigned length, void *output);
void sha256_transform(uint32_t *state, const void *data);

#endif

