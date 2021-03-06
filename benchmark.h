
// Algo list
#define ALGO_MD5         0x01
#define ALGO_SHA1        0x02
#define ALGO_SHA256      0x03
#define ALGO_SHA512      0x04

#define ALGO_CHACHA20    0x05
#define ALGO_AES_ENC_ECB 0x06
#define ALGO_AES_DEC_ECB 0x07
#define ALGO_AES_ENC_CFB 0x08
#define ALGO_AES_DEC_CFB 0x09

// Raw sources
#define SRC_RAWNOISE     0x00
#define SRC_RAND         0x01
#define SRC_RAND_FAST    0x02

// Some magic endpoints of ours to poke the key
#define CMD_RUN_SELFTEST     0x80
#define CMD_START_BENCHMARK  0x81
#define CMD_QUERY_BENCHMARK  0x82
#define CMD_QUERY_RAWDATA    0x83
#define CMD_GO_DFU           0xff

// Device IDs
#define VENDOR_ID         0xbede
#define PRODUCT_ID        0xb1ce


