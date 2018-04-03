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

#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/crc.h>
#include <libopencm3/cm3/dwt.h>
#include <stdint.h>
#include <string.h>

#include "random.h"
#include "sha256.h"

// The random number generator is inspired in the ideas implemented by
// NeuG, which consists on using ADC noise to generate some entropy.
// The idea is that we can sample from different sources (temperature
// sensor, GPIOs, ...) and combine that entropy with other sources
// (we use the cycle counter too) of entropy and then feed some filters
// like CRC32 (as a quick and dirty low-entropy sample accumulation)
// and SHA256 (as a proper filter that outputs true random bits).
// One recommendation that NeuG does is to stop conversions to capture
// more noise but not allowing the ADC to stabilize.

void init_random() {
	// Turn off to configure
	rcc_periph_clock_enable(RCC_ADC1);
	adc_power_off(ADC1);
	rcc_periph_reset_pulse(RST_ADC1);

	rcc_set_adcpre(RCC_CFGR_ADCPRE_PCLK2_DIV2);
	adc_set_dual_mode(ADC_CR1_DUALMOD_IND);

	// We configure everything for one single conversion.
	adc_disable_scan_mode(ADC1);
	adc_set_single_conversion_mode(ADC1);
	adc_enable_temperature_sensor();
	adc_disable_external_trigger_regular(ADC1);
	adc_set_right_aligned(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_1DOT5CYC);

	adc_power_on(ADC1);

	adc_reset_calibration(ADC1);

	// Enable CRC machinery
	rcc_periph_clock_enable(RCC_AHBENR_CRCEN);
	rcc_periph_clock_enable(RCC_CRC);
	crc_reset();
}

// Fills the buffer with 64 bytes of raw data from the STM32 ADC
void fill_raw_samples64(uint8_t *buffer) {
	uint16_t *obuf = (uint16_t*)buffer;

	// 1 channel single conversion, for temp sensor
	ADC_SQR1(ADC1) = 0;
	ADC_SQR2(ADC1) = 0;

	for (unsigned j = 0; j < 32*8; j++) {
		// Switch temperature and Vint each iteration
		ADC_SQR3(ADC1) = 16 | (j & 1);

		adc_start_conversion_direct(ADC1);
		while (!adc_eoc(ADC1));
		uint16_t v = adc_read_regular(ADC1);
		obuf[j >> 3] = (obuf[j >> 3] << 1) ^ v;
	}
}

// Seems there's quite a low entropy in the returned buffer
// Good idea to use plenty of bits
void get_random_bytes32(uint8_t *buffer) {
	uint32_t noises[32 + 1 + 3];
	for (unsigned j = 0; j < 32; j++) {
		const unsigned nwords = 64 / sizeof(uint32_t);
		uint32_t tmpb[nwords];
		fill_raw_samples64((uint8_t*)tmpb);

		// Run the buffer through a CRC filter to get one word
		noises[j] = crc_calculate_block(tmpb, nwords);
	}
	noises[32] = DWT_CYCCNT;
	noises[33] = *(uint32_t*)(0x1ffff7e8);
	noises[34] = *(uint32_t*)(0x1ffff7ec);
	noises[35] = *(uint32_t*)(0x1ffff7f0);
	sha256sum((uint8_t*)noises, sizeof(noises), buffer);
}


