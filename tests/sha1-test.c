
// SHA1 tests from

#include <stdio.h>
#include <string.h>

#include "sha1.h"

struct t_test {
	uint8_t result[20];
	const char *input;
};

const struct t_test tests[] = {
	{
		"\xa9\x99\x3e\x36\x47\x06\x81\x6a\xba\x3e\x25\x71\x78\x50\xc2\x6c\x9c\xd0\xd8\x9d",
		"abc",
	},{
		"\xda\x39\xa3\xee\x5e\x6b\x4b\x0d\x32\x55\xbf\xef\x95\x60\x18\x90\xaf\xd8\x07\x09",
		"",
	},{
		"\x84\x98\x3e\x44\x1c\x3b\xd2\x6e\xba\xae\x4a\xa1\xf9\x51\x29\xe5\xe5\x46\x70\xf1",
		"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
	},{
		"\xa4\x9b\x24\x46\xa0\x2c\x64\x5b\xf4\x19\xf9\x95\xb6\x70\x91\x25\x3a\x04\xa2\x59",
		"abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
	},
};

int main(int argc, char **argv) {
	int err = 0;
	for (unsigned i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		uint8_t out[20];
		sha1sum((uint8_t*)tests[i].input, strlen(tests[i].input), out);
		err = memcmp(out, tests[i].result, 20);
		// LCOV_EXCL_START
		if (err) {
			printf("Test %d mismatch\n", i);
			printf("Ref: ");
			for (unsigned j = 0; j < 20; j++)
				printf("%02x", tests[i].result[j]);
			printf("\n");
			printf("Out: ");
			for (unsigned j = 0; j < 20; j++)
				printf("%02x", out[j]);
			printf("\n");
		}
		// LCOV_EXCL_STOP
	}
	if (!err)
		printf("SHA1: Test OK, no errors!\n");
	return err;
}


