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

#include <string.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/pwr.h>
#include <libopencm3/stm32/st_usbfs.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencmsis/core_cm3.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/cm3/dwt.h>

#include "benchmark.h"
#include "aes.h"
#include "chacha20.h"
#include "md5.h"
#include "sha1.h"
#include "sha256.h"
#include "random.h"
#include "tests/test_main.h"

static int custom_control_request_in(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *dev, struct usb_setup_data *req));

volatile uint32_t systick_counter_ms = 0;

// Work variables
volatile uint32_t restart = 0;
volatile uint32_t iterations = 0;
volatile uint32_t algo_arg = 0;
volatile uint32_t startms = 0;
void (*volatile algoptr)() = 0;

/* Buffer for the USB transactions, the bigger the better really */
uint8_t usbd_control_buffer[256];
uint8_t usbd_control_send[4096];
usbd_device *usbd_dev;


const struct usb_device_descriptor dev_descr = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x02,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = VENDOR_ID,
	.idProduct = PRODUCT_ID,
	.bcdDevice = 0x0010,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

// Definition of the two interfaces, control 0 and data 1
const struct usb_interface_descriptor benchmark_iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 0,
	.bInterfaceClass = 0xFF,
	.bInterfaceSubClass = 0xFF,
	.bInterfaceProtocol = 0,
	.iInterface = 0,
};

// Iterface enumeration for this config (CDC ECM)
const struct usb_interface dev_ifaces[] = {
	{ .num_altsetting = 1, .altsetting = &benchmark_iface },
};

const struct usb_config_descriptor configs[] = {
	{
		.bLength = USB_DT_CONFIGURATION_SIZE,
		.bDescriptorType = USB_DT_CONFIGURATION,
		.wTotalLength = 0,         // This is automatically filled in
		.bNumInterfaces = 1,       // Just one interface
		.bConfigurationValue = 1,
		.iConfiguration = 0,
		.bmAttributes = 0x80,      // Bus powered
		.bMaxPower = 0x64,

		.interface = dev_ifaces,
	},
};

static char serial_no[25] = {0};

static const char *usb_strings[] = {
	"davidgf.net",
	"stm32-libs Project",
	serial_no,
};

static void get_dev_unique_id(char *s) {
	volatile uint8_t *unique_id = (volatile uint8_t *)0x1FFFF7E8;
	int i;

	/* Fetch serial number from chip's unique ID */
	for(i = 0; i < 24; i+=2) {
		s[i] = ((*unique_id >> 4) & 0xF) + '0';
		s[i+1] = (*unique_id++ & 0xF) + '0';
	}
	for(i = 0; i < 24; i++)
		if(s[i] > '9')
			s[i] += 'A' - '9' - 1;
}

// Some custom sutff int the EP0 for rebooting and poking
static void reboot_dfu_complete(usbd_device *dev, struct usb_setup_data *req) {
	(void)req;
	(void)dev;

	// Reboot into DFU!
	extern uint32_t _stack;
	uint64_t * ptr = (uint64_t*)&_stack;
	*ptr = 0xDEADBEEFCC00FFEEULL;
	scb_reset_system();
}

static void custom_set_config(usbd_device *dev, uint16_t wValue) {
	usbd_register_control_callback(
				dev,
				USB_REQ_TYPE_STANDARD | USB_REQ_TYPE_INTERFACE | USB_REQ_TYPE_IN,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT | USB_REQ_TYPE_DIRECTION,
				custom_control_request_in);
}

void usb_suspend_callback() {
	*USB_CNTR_REG |= USB_CNTR_FSUSP;
	*USB_CNTR_REG |= USB_CNTR_LP_MODE;
	SCB_SCR |= SCB_SCR_SLEEPDEEP;
	__WFI();
}

void usb_wakeup_isr() {
	exti_reset_request(EXTI18);
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	*USB_CNTR_REG &= ~USB_CNTR_FSUSP;
}

void reenumerate_usb() {
	/*
	 * Vile hack to reenumerate, physically _drag_ d+ low.
	 * (need at least 2.5us to trigger usb disconnect)
	 */
	rcc_periph_clock_enable(RCC_GPIOA);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_2_MHZ, GPIO_CNF_OUTPUT_PUSHPULL, GPIO12);
	gpio_clear(GPIOA, GPIO12);
	for (unsigned int i = 0; i < 800000; i++)
		__asm__("nop");
	gpio_set_mode(GPIOC, GPIO_MODE_INPUT, GPIO_CNF_INPUT_FLOAT, GPIO12);
}

void usb_lp_can_rx0_isr(void) {
	usbd_poll(usbd_dev);
}

void start_usb() {
	get_dev_unique_id(serial_no);

	// Force USB connection
	reenumerate_usb();

	usbd_dev = usbd_init(&st_usbfs_v1_usb_driver, &dev_descr, configs,
			     usb_strings, 3,
			     usbd_control_buffer, sizeof(usbd_control_buffer));
	usbd_register_set_config_callback(usbd_dev, custom_set_config);

	nvic_set_priority(NVIC_USB_LP_CAN_RX0_IRQ, ~0);
	nvic_enable_irq(NVIC_USB_LP_CAN_RX0_IRQ);
}

void sys_tick_handler() {
	systick_counter_ms++;
}

void init_low_power_modes() {
	// Enable clock gating the Cortex on WFI/WFE
	// Make sure we continue running after returning from ISR
	SCB_SCR &= ~SCB_SCR_SLEEPDEEP;
	SCB_SCR &= ~SCB_SCR_SLEEPONEXIT;
	pwr_voltage_regulator_on_in_stop();
	pwr_set_stop_mode();
}

void init_clock() {
	// SysTick interrupt every N clock pulses: set reload to N-1
	// Interrupt every ms assuming we have 72MHz clock
	nvic_set_priority(NVIC_SYSTICK_IRQ, 0);
	nvic_enable_irq(NVIC_SYSTICK_IRQ);
	systick_set_clocksource(STK_CSR_CLKSOURCE_AHB_DIV8);
	systick_set_reload(8999);
	systick_interrupt_enable();
	systick_counter_enable();
}

// Consumption at 72MHz in sleep mode is 5.5-14.4mA (run is 27-36mA)

// Functions to call for work
void md5_core() {
	uint32_t state[4];
	void *inpbuf = (void*)0x20000000;
	for (unsigned i = 0; i < 16; i++) {
		iterations++;
		md5_transform(state, inpbuf);
	}
}

void md5_buffer() {
	uint8_t hash[16];
	void *inpbuf = (void*)0x20000000;
	unsigned len = algo_arg * 64 - 32;
	for (unsigned i = 0; i < 16; i++) {
		iterations++;
		md5sum(inpbuf, len, hash);
	}
}

void sha1_core() {
	uint32_t state[5];
	void *inpbuf = (void*)0x20000000;
	for (unsigned i = 0; i < 16; i++) {
		iterations++;
		sha1_transform(state, inpbuf);
	}
}

void sha1_buffer() {
	uint8_t hash[20];
	void *inpbuf = (void*)0x20000000;
	unsigned len = algo_arg * 64 - 32;
	for (unsigned i = 0; i < 16; i++) {
		iterations++;
		sha1sum(inpbuf, len, hash);
	}
}

void sha256_core() {
	uint32_t state[8];
	void *inpbuf = (void*)0x20000000;
	for (unsigned i = 0; i < 16; i++) {
		iterations++;
		sha256_transform(state, inpbuf);
	}
}

void sha256_buffer() {
	uint8_t hash[20];
	void *inpbuf = (void*)0x20000000;
	unsigned len = algo_arg * 64 - 32;
	for (unsigned i = 0; i < 16; i++) {
		iterations++;
		sha256sum(inpbuf, len, hash);
	}
}

void aes_ecb_core_enc() {
	uint8_t buf[16];
	struct AES_ctx c;
	AES_init_ctx_iv(&c, buf, AESkey128, buf);
	for (unsigned i = 0; i < 16; i++) {
		iterations++;
		AES_ECB_encrypt(&c, buf);
	}
}

void aes_ecb_core_dec() {
	uint8_t buf[16];
	struct AES_ctx c;
	AES_init_ctx_iv(&c, buf, AESkey128, buf);
	for (unsigned i = 0; i < 16; i++) {
		iterations++;
		AES_ECB_decrypt(&c, buf);
	}
}

void aes_cfb_buf_enc() {
	uint8_t buf[4096], key[32];
	struct AES_ctx c;
	AES_init_ctx_iv(&c, key, AESkey128, buf);
	unsigned len = (algo_arg * AES_BLOCKLEN) & 4095;
	for (unsigned i = 0; i < 16; i++) {
		iterations++;
		AES_CFB_encrypt(&c, buf, len);
	}
}

void aes_cfb_buf_dec() {
	uint8_t buf[4096], key[32];
	struct AES_ctx c;
	AES_init_ctx_iv(&c, key, AESkey128, buf);
	unsigned len = (algo_arg * AES_BLOCKLEN) & 4095;
	for (unsigned i = 0; i < 16; i++) {
		iterations++;
		AES_CFB_decrypt(&c, buf, len);
	}
}

void chacha20_core() {
	uint32_t crypto_state[16], stream[16];
	for (unsigned i = 0; i < 16; i++) {
		chacha20_block(crypto_state, stream);
		iterations++;
	}
}

void chacha20_buffer() {
	uint8_t buffer[4096];
	uint32_t key[16];
	unsigned len = (algo_arg * CHACHA20_BLOCKSIZE) & 4095;
	for (unsigned i = 0; i < 16; i++) {
		chacha20_encrypt_decrypt_block((uint32_t*)buffer, (uint32_t*)buffer, len, key, 123);
		iterations++;
	}
}

int main() {
	// Interrupt priorities
	scb_set_priority_grouping(SCB_AIRCR_PRIGROUP_GROUP4_SUB4);

	// Start USB machinery
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	start_usb();

	dwt_enable_cycle_counter();
	init_low_power_modes();

	init_random();
	init_clock();

	while (1) {
		if (restart) {
			restart = 0;
			startms = systick_counter_ms;
			iterations = 0;
		}

		if (algoptr)
			algoptr();
	}
}

static int custom_control_request_in(usbd_device *dev, struct usb_setup_data *req, uint8_t **buf, uint16_t *len,
			void (**complete)(usbd_device *dev, struct usb_setup_data *req)) {

	// Receive data from Host
	switch (req->bRequest) {
	case CMD_RUN_SELFTEST:
		usbd_control_send[0] = runalltests();
		*buf = usbd_control_send;
		*len = 1;
		break;
	case CMD_START_BENCHMARK: {
		// Start running iterations of some hash/crypto algorithm
		uint8_t algo = req->wValue >> 8;
		uint8_t mode = req->wValue & 255;
		switch (algo) {
		case ALGO_MD5:
			if (mode)
				algoptr = md5_buffer;
			else
				algoptr = md5_core;
			break;
		case ALGO_SHA1:
			if (mode)
				algoptr = sha1_buffer;
			else
				algoptr = sha1_core;
			break;
		case ALGO_SHA256:
			if (mode)
				algoptr = sha256_buffer;
			else
				algoptr = sha256_core;
			break;
		case ALGO_SHA512:
			break;
		case ALGO_CHACHA20:
			if (mode)
				algoptr = chacha20_buffer;
			else
				algoptr = chacha20_core;
			break;
		case ALGO_AES_ENC_ECB:
			algoptr = aes_ecb_core_enc;
			break;
		case ALGO_AES_DEC_ECB:
			algoptr = aes_ecb_core_dec;
			break;
		case ALGO_AES_ENC_CFB:
			algoptr = aes_cfb_buf_enc;
			break;
		case ALGO_AES_DEC_CFB:
			algoptr = aes_cfb_buf_dec;
			break;
		default:
			break;
		};
		algo_arg = mode;
		restart = 1;
		}break;
	case CMD_QUERY_BENCHMARK: {
		// Report the iteration count and the ms elapsed
		uint32_t elapsed = systick_counter_ms - startms;
		uint32_t report[2] = { iterations, elapsed };
		memcpy(usbd_control_send, report, sizeof(report));

		*buf = usbd_control_send;
		*len = 8;
		}break;
	case CMD_QUERY_RAWDATA:
		switch (req->wValue) {
		case SRC_RAWNOISE:
			for (unsigned i = 0; i < sizeof(usbd_control_send); i += 64)
				fill_raw_samples64(&usbd_control_send[i]);
			*buf = usbd_control_send;
			*len = sizeof(usbd_control_send);
			break;
		case SRC_RAND:
			for (unsigned i = 0; i < sizeof(usbd_control_send); i += 32)
				get_random_bytes32(&usbd_control_send[i]);
			*buf = usbd_control_send;
			*len = sizeof(usbd_control_send);
			break;
		case SRC_RAND_FAST:
			get_random_bytes32(usbd_control_send);
			for (unsigned i = 32; i < sizeof(usbd_control_send); i += 32)
				sha256sum(&usbd_control_send[i-32], 32, &usbd_control_send[i]);
			*buf = usbd_control_send;
			*len = sizeof(usbd_control_send);
			break;
		};
		break;
    case CMD_GO_DFU:
        *len = 0;
        *complete = reboot_dfu_complete;
        break;
	}

	return 1;
}


