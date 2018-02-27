
// MD5 size optimized implementation
// Copyright 2018 David Guillen Fandos <david@davidgf.net>

#ifndef __MD5__H__
#define __MD5__H__

#include <string.h>
#include <stdint.h>

#define MD5_DIGEST_SIZE 16
void md5_transform(uint32_t *state, const void *data);
void md5sum(const uint8_t *inbuffer, unsigned length, void *output);

#endif

