
#ifndef __RANDOM__H__
#define __RANDOM__H__

#include <stdint.h>

// Call this before using the module
void init_random();
// Fills the buffer with 64 bytes of raw data from the STM32 ADC
void fill_raw_samples64(uint8_t *buffer);
// Generate 32 bytes of true random data
void get_random_bytes32(uint8_t *buffer);

#endif

