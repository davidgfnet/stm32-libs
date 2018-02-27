
// Common hashing functions
// Copyright 2018 David Guillen Fandos <david@davidgf.net>

#ifndef __HASH_COMMON__H__
#define __HASH_COMMON__H__

#include <stdint.h>

typedef struct {
	void (*transformfn)(uint32_t *state, const void *data);
	const uint32_t *init_state;
	uint8_t hashsize;
} t_hash_fns;

void hash32_sum(const t_hash_fns *desc, const uint8_t *inbuffer, unsigned length, void *output);

#endif

