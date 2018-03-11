/*
 * cli-tool.cc
 *
 * Author: David Guillen Fandos (2018) <david@davidgf.net>
 *
 * CLI tool to measure benchmark speed
 *
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unordered_map>
#include <string>
#include <fstream>
#include <unistd.h>
#include <libusb-1.0/libusb.h>

#include "benchmark.h"

#define IFACE_NUMBER         0x0
#define TIMEOUT_MS          5000

const std::unordered_map<std::string, unsigned> algomap = {
	{"md5", ALGO_MD5},
	{"sha1", ALGO_SHA1},
	{"sha256", ALGO_SHA256},
	{"sha512", ALGO_SHA512},
	{"chacha20", ALGO_CHACHA20},
	{"aes-ebc-enc", ALGO_AES_ENC_ECB},
	{"aes-ebc-dec", ALGO_AES_DEC_ECB},
	{"aes-cfb-enc", ALGO_AES_ENC_CFB},
	{"aes-cfb-dec", ALGO_AES_DEC_CFB},
};

const std::unordered_map<std::string, unsigned> algorand = {
	{"rawnoise", SRC_RAWNOISE},
	{"rand", SRC_RAND},
	{"randfast", SRC_RAND_FAST},
};

static const int CTRL_REQ_TYPE_IN  = LIBUSB_ENDPOINT_IN  | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_INTERFACE;
static const int CTRL_REQ_TYPE_OUT = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_STANDARD | LIBUSB_RECIPIENT_INTERFACE;

void fatal_error(const char * errmsg, int code) {
	fprintf(stderr, "ERROR! %d %s\n", code, errmsg);
	exit(1);
}

int main(int argc, char ** argv) {
	// Parse input
	// Usage is like ./xion set port-num value
	if (argc < 2) {
		fprintf(stderr, "Usage: %s algorithm block-len\n", argv[0]);
		exit(1);
	}

	std::string cmd = argv[1];
	std::string arg1 = argv[2];

	int result = libusb_init(NULL);
	if (result < 0)
		fatal_error("libusb_init failed!", result);

	struct libusb_device_handle *devh = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);

	if (!devh)
		fatal_error("libusb_open_device_with_vid_pid failed to find a matching device!", 0);

	result = libusb_detach_kernel_driver(devh, IFACE_NUMBER);
	result = libusb_claim_interface(devh, IFACE_NUMBER);
	if (result < 0)
		fatal_error("libusb_claim_interface failed!", result);

	if (cmd == "rebootdfu") {
		// Do not check result since the reboot happens inmediately and it's very likely to fail
		uint8_t dummy;
		if (libusb_control_transfer(devh, CTRL_REQ_TYPE_IN, CMD_GO_DFU, 0, IFACE_NUMBER, &dummy, 1, TIMEOUT_MS) < 0)
			fatal_error("Reboot into DFU command failed!", 0);
	}
	else if (algorand.count(cmd)) {
		uint16_t mode = (algorand.at(cmd));
		for (unsigned i = 0; i < atoi(arg1.c_str()); i++) {
			uint8_t raw[4096];
			auto r = libusb_control_transfer(devh, CTRL_REQ_TYPE_IN, CMD_QUERY_RAWDATA, mode, IFACE_NUMBER, raw, sizeof(raw), TIMEOUT_MS);
			if (r < 0)
				fatal_error("Failed to retrieve info from the device!", 0);
			else {
				write(1, raw, r);
			}
		}
	}
	else if (algomap.count(cmd)) {
		printf("Starting benchmark\n");
		uint16_t mode = (algomap.at(cmd) << 8) | atoi(arg1.c_str());
		auto r = libusb_control_transfer(devh, CTRL_REQ_TYPE_IN, CMD_START_BENCHMARK, mode, IFACE_NUMBER, 0, 0, TIMEOUT_MS);
		if (r < 0)
			fatal_error("Failed to retrieve info from the device!", 0);
		sleep(5);
		uint8_t report[8];
		r = libusb_control_transfer(devh, CTRL_REQ_TYPE_IN, CMD_QUERY_BENCHMARK, mode, IFACE_NUMBER, report, sizeof(report), TIMEOUT_MS);
		if (r != sizeof(report))
			fatal_error("Failed to retrieve info from the device!", 0);

		uint32_t niters = (report[3] << 24) | (report[2] << 16) | (report[1] << 8) | (report[0]);
		uint32_t elapsed = (report[7] << 24) | (report[6] << 16) | (report[5] << 8) | (report[4]);

		printf("Iterations: %u\nTook %ums\n", niters, elapsed);
		printf("Rate %f iterations/sec\n", niters / (elapsed / 1000.0f) );
	}

	libusb_release_interface(devh, 0);
	libusb_close(devh);
	libusb_exit(NULL);
	return 0;
}

