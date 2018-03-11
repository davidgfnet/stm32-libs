
// List of tests that will run back to back
int aes_test();
int chacha20_test();
int md5_test();
int sha1_test();
int sha256_test();

typedef int (*test_fn_ptr)(void);

static test_fn_ptr alltests[] = {
	aes_test,
	chacha20_test,
	md5_test,
	sha1_test,
	sha256_test,
};

int runalltests() {
	// Run all tests back to back
	int ret = 0;
	for (int i = 0; i < sizeof(alltests)/sizeof(alltests[0]); i++)
		ret |= alltests[i]();
	return ret;
}

