
// AES from RFC

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "aes.h"

typedef struct testVector {
    AESKeyType  ktype;
    const char  input[16];
    const char  key[32];
    const char  iv[16];
    const char  output[16];
} testVector;

int main() {
    testVector t[] = {
        {
            .ktype  = AESkey128,
            .input  = "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a",
            .key    = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
            .output = "\x3a\xd7\x7b\xb4\x0d\x7a\x36\x60\xa8\x9e\xca\xf3\x24\x66\xef\x97",
        },
        {
            .ktype  = AESkey128,
            .input  = "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51",
            .key    = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
            .output = "\xf5\xd3\xd5\x85\x03\xb9\x69\x9d\xe7\x85\x89\x5a\x96\xfd\xba\xaf",
        },
        {
            .ktype  = AESkey128,
            .input  = "\x30\xc8\x1c\x46\xa3\x5c\xe4\x11\xe5\xfb\xc1\x19\x1a\x0a\x52\xef",
            .key    = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
            .output = "\x43\xb1\xcd\x7f\x59\x8e\xce\x23\x88\x1b\x00\xe3\xed\x03\x06\x88",
        },
        {
            .ktype  = AESkey128,
            .input  = "\xf6\x9f\x24\x45\xdf\x4f\x9b\x17\xad\x2b\x41\x7b\xe6\x6c\x37\x10",
            .key    = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
            .output = "\x7b\x0c\x78\x5e\x27\xe8\xad\x3f\x82\x23\x20\x71\x04\x72\x5d\xd4",
        },
        {
            .ktype  = AESkey192,
            .input  = "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a",
            .key    = "\x8e\x73\xb0\xf7\xda\x0e\x64\x52\xc8\x10\xf3\x2b\x80\x90\x79\xe5\x62\xf8\xea\xd2\x52\x2c\x6b\x7b",
            .output = "\xbd\x33\x4f\x1d\x6e\x45\xf2\x5f\xf7\x12\xa2\x14\x57\x1f\xa5\xcc",
        },
        {
            .ktype  = AESkey192,
            .input  = "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51",
            .key    = "\x8e\x73\xb0\xf7\xda\x0e\x64\x52\xc8\x10\xf3\x2b\x80\x90\x79\xe5\x62\xf8\xea\xd2\x52\x2c\x6b\x7b",
            .output = "\x97\x41\x04\x84\x6d\x0a\xd3\xad\x77\x34\xec\xb3\xec\xee\x4e\xef",
        },
        {
            .ktype  = AESkey256,
            .input  = "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a",
            .key    = "\x60\x3d\xeb\x10\x15\xca\x71\xbe\x2b\x73\xae\xf0\x85\x7d\x77\x81"
                      "\x1f\x35\x2c\x07\x3b\x61\x08\xd7\x2d\x98\x10\xa3\x09\x14\xdf\xf4",
            .output = "\xf3\xee\xd1\xbd\xb5\xd2\xa0\x3c\x06\x4b\x5a\x7e\x3d\xb1\x81\xf8",
        },
        {
            .ktype  = AESkey256,
            .input  = "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51",
            .key    = "\x60\x3d\xeb\x10\x15\xca\x71\xbe\x2b\x73\xae\xf0\x85\x7d\x77\x81"
                      "\x1f\x35\x2c\x07\x3b\x61\x08\xd7\x2d\x98\x10\xa3\x09\x14\xdf\xf4",
            .output = "\x59\x1c\xcb\x10\xd4\x10\xed\x26\xdc\x5b\xa7\x4a\x31\x36\x28\x70",
        },
    };

    int times = sizeof(t) / sizeof(struct testVector), i;

    for (i = 0; i < times; ++i) {
        struct AES_ctx c;
        uint8_t b[16];
        memcpy(b, t[i].input, sizeof(b));

        AES_init_ctx(&c, t[i].key, t[i].ktype);
        AES_ECB_encrypt(&c, b);

        if (memcmp(b, t[i].output, 16) != 0)
            return 1; // LCOV_EXCL_LINE

        AES_ECB_decrypt(&c, b);

        if (memcmp(b, t[i].input, 16) != 0)
            return 1; // LCOV_EXCL_LINE
    }

    testVector t2[] = {
        {
            .ktype  = AESkey128,
            .input  = "\x6b\xc1\xbe\xe2\x2e\x40\x9f\x96\xe9\x3d\x7e\x11\x73\x93\x17\x2a",
            .key    = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
            .iv     = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F",
            .output = "\x3b\x3f\xd9\x2e\xb7\x2d\xad\x20\x33\x34\x49\xf8\xe8\x3c\xfb\x4a",
        },
        {
            .ktype  = AESkey128,
            .input  = "\xae\x2d\x8a\x57\x1e\x03\xac\x9c\x9e\xb7\x6f\xac\x45\xaf\x8e\x51",
            .key    = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
            .iv     = "\x3b\x3f\xd9\x2e\xb7\x2d\xad\x20\x33\x34\x49\xf8\xe8\x3c\xfb\x4a",
            .output = "\xc8\xa6\x45\x37\xa0\xb3\xa9\x3f\xcd\xe3\xcd\xad\x9f\x1c\xe5\x8b",
        },
        {
            .ktype  = AESkey128,
            .input  = "\x30\xc8\x1c\x46\xa3\x5c\xe4\x11\xe5\xfb\xc1\x19\x1a\x0a\x52\xef",
            .key    = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
            .iv     = "\xc8\xa6\x45\x37\xa0\xb3\xa9\x3f\xcd\xe3\xcd\xad\x9f\x1c\xe5\x8b",
            .output = "\x26\x75\x1f\x67\xa3\xcb\xb1\x40\xb1\x80\x8c\xf1\x87\xa4\xf4\xdf",
        },
        {
            .ktype  = AESkey128,
            .input  = "\xf6\x9f\x24\x45\xdf\x4f\x9b\x17\xad\x2b\x41\x7b\xe6\x6c\x37\x10",
            .key    = "\x2b\x7e\x15\x16\x28\xae\xd2\xa6\xab\xf7\x15\x88\x09\xcf\x4f\x3c",
            .iv     = "\x26\x75\x1f\x67\xa3\xcb\xb1\x40\xb1\x80\x8c\xf1\x87\xa4\xf4\xdf",
            .output = "\xc0\x4b\x05\x35\x7c\x5d\x1c\x0e\xea\xc4\xc6\x6f\x9f\xf7\xf2\xe6",
        },
    };

    times = sizeof(t2) / sizeof(struct testVector);

    for (i = 0; i < times; ++i) {
        struct AES_ctx c;
        uint8_t b[16];
        memcpy(b, t2[i].input, sizeof(b));

        AES_init_ctx_iv(&c, t2[i].key, t2[i].ktype, t2[i].iv);
        AES_CFB_encrypt(&c, b, 16);

        if (memcmp(b, t2[i].output, 16) != 0)
            return 1; // LCOV_EXCL_LINE

        // Check undo works
        AES_init_ctx_iv(&c, t2[i].key, AESkey128, t2[i].iv);
        AES_CFB_decrypt(&c, b, 16);
        if (memcmp(b, t2[i].input, 16) != 0)
            return 1; // LCOV_EXCL_LINE
    }

    // Check larger vectors
    uint8_t input[16*16], key[16], iv[16], output[16*16];
    for (i = 0; i < sizeof(input); i++)
        input[i] = output[i] = rand();
    for (i = 0; i < sizeof(key); i++)
        key[i] = rand();
    for (i = 0; i < sizeof(iv); i++)
        iv[i] = rand();

    struct AES_ctx c;
    AES_init_ctx_iv(&c, key, AESkey128, iv);
    AES_CFB_encrypt(&c, output, sizeof(output));

    AES_init_ctx_iv(&c, key, AESkey128, iv);
    AES_CFB_decrypt(&c, output, sizeof(output));

    if (memcmp(input, output, sizeof(input)) != 0)
        return 1; // LCOV_EXCL_LINE

    return 0;
}

