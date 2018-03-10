
// Implementation of ChaCha20, bits borrowed here and there from Linux, the RFC
// and other random places

#ifndef __CHACHA20__H_
#define __CHACHA20__H_

#include <stdint.h>

#define CHACHA20_KEYSIZE   32
#define CHACHA20_BLOCKSIZE 64

void chacha20_encrypt_decrypt_block(const uint32_t *inbuffer, uint32_t *outbuffer, unsigned len, const void* key, uint64_t nonce64);
void chacha20_block(uint32_t *state, void *stream);

#endif

