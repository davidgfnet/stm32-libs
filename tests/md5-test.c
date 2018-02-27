
// MD5 tests

#include <stdio.h>
#include <string.h>

#include "md5.h"

struct t_test {
	uint8_t result[16];
	const char *input;
};

const struct t_test tests[] = {
	{
		"\x90\x01\x50\x98\x3c\xd2\x4f\xb0\xd6\x96\x3f\x7d\x28\xe1\x7f\x72",
		"abc",
	},{
		"\xd4\x1d\x8c\xd9\x8f\x00\xb2\x04\xe9\x80\x09\x98\xec\xf8\x42\x7e",
		"",
	},{
		"\x82\x15\xef\x07\x96\xa2\x0b\xca\xaa\xe1\x16\xd3\x87\x6c\x66\x4a",
		"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
	},{
		"\x03\xdd\x88\x07\xa9\x31\x75\xfb\x06\x2d\xfb\x55\xdc\x7d\x35\x9c",
		"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
	},
};

int main(int argc, char **argv) {
	int err = 0;
	for (unsigned i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		uint8_t out[16];
		md5sum((uint8_t*)tests[i].input, strlen(tests[i].input), out);
		err = memcmp(out, tests[i].result, sizeof(out));
		// LCOV_EXCL_START
		if (err) {
			printf("Test %d mismatch\n", i);
			printf("Ref: ");
			for (unsigned j = 0; j < sizeof(out); j++)
				printf("%02x", tests[i].result[j]);
			printf("\n");
			printf("Out: ");
			for (unsigned j = 0; j < sizeof(out); j++)
				printf("%02x", out[j]);
			printf("\n");
		}
		// LCOV_EXCL_STOP
	}
	if (!err)
		printf("MD5: Test OK, no errors!\n");
	return err;
}


