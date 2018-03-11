
#include "sha256.h"

typedef struct testVector {
    const char*  input;
    const char*  output;
    size_t inLen;
    size_t outLen;
} testVector;

int sha256_test() {
    uint8_t   hash[SHA256_DIGEST_SIZE];

    testVector t[4];
    int times = sizeof(t) / sizeof(struct testVector), i;

    t[0].input  = "";
    t[0].output = "\xe3\xb0\xc4\x42\x98\xfc\x1c\x14\x9a\xfb\xf4\xc8\x99\x6f\xb9\x24"
                  "\x27\xae\x41\xe4\x64\x9b\x93\x4c\xa4\x95\x99\x1b\x78\x52\xb8\x55";
    t[0].inLen  = strlen(t[0].input);
    t[0].outLen = SHA256_DIGEST_SIZE;

    t[1].input  = "abc";
    t[1].output = "\xBA\x78\x16\xBF\x8F\x01\xCF\xEA\x41\x41\x40\xDE\x5D\xAE\x22\x23"
                  "\xB0\x03\x61\xA3\x96\x17\x7A\x9C\xB4\x10\xFF\x61\xF2\x00\x15\xAD";
    t[1].inLen  = strlen(t[1].input);
    t[1].outLen = SHA256_DIGEST_SIZE;

    t[2].input  = "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
    t[2].output = "\x24\x8D\x6A\x61\xD2\x06\x38\xB8\xE5\xC0\x26\x93\x0C\x3E\x60\x39"
                  "\xA3\x3C\xE4\x59\x64\xFF\x21\x67\xF6\xEC\xED\xD4\x19\xDB\x06\xC1";
    t[2].inLen  = strlen(t[2].input);
    t[2].outLen = SHA256_DIGEST_SIZE;

    t[3].input  = "\x17\xc0\x38\x10\x7a\x65\x88\x0f\x83\x4a\xc0\xdf\x73\xf3\xc0\xcd"
                  "\xa3\xe7\x1c\xdb\xfc\x95\xfc\xa8\xd3\x15\xc2\x03\x34\xfe\x9f\x7e"
                  "\x09\xa6\x70\x04\xbe\x2e\x55\xe9\xc6\x5f\x5e\x12\x4d\xa5\x10\x73"
                  "\x5f\xbb\x94\x35\x62\xce\x73\x39\x45\x12\x08\xc8\xa0\xff\xca\xff";
    t[3].output = "\x3d\x73\x25\x41\xd3\xc9\xe0\x88\x23\xb4\x2e\xa2\x1b\xc9\x2c\x73"
                  "\x1d\x69\xec\x27\x8f\x7b\x0e\x9f\x24\x6f\x9c\x30\xe7\x25\x34\x5b";
    t[3].inLen  = 64;
    t[3].outLen = SHA256_DIGEST_SIZE;

    for (i = 0; i < times; ++i) {
        sha256sum((uint8_t*)t[i].input, t[i].inLen, hash);

        if (memcmp(hash, t[i].output, SHA256_DIGEST_SIZE) != 0)
            return 1;  // LCOV_EXCL_LINE
    }

    uint8_t input1[256] = {0};
    const char* digest1 =
        "\x53\x41\xe6\xb2\x64\x69\x79\xa7\x0e\x57\x65\x30\x07\xa1\xf3\x10"
        "\x16\x94\x21\xec\x9b\xdd\x9f\x1a\x56\x48\xf7\x5a\xde\x00\x5a\xf1";

    sha256sum(input1, sizeof(input1), hash);

    if (memcmp(hash, digest1, SHA256_DIGEST_SIZE) != 0)
        return 1; // LCOV_EXCL_LINE

    return 0;
}


