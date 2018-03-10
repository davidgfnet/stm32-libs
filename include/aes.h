
#ifndef __AES__H___
#define __AES__H___

#define AES_BLOCKLEN    16

typedef enum {
	AESkey128 = 0,
	AESkey192 = 1,
	AESkey256 = 2
} AESKeyType;

struct AES_ctx {
	uint32_t RoundKey[15*4];
	uint8_t iv[AES_BLOCKLEN];
	uint8_t kt;
};

// Typical AES modes
void AES_ECB_encrypt(struct AES_ctx *ctx, uint8_t* buf);
void AES_ECB_decrypt(struct AES_ctx* ctx, uint8_t* buf);
void AES_CFB_encrypt(struct AES_ctx *ctx, uint8_t* buf, int len);
void AES_CFB_decrypt(struct AES_ctx *ctx, uint8_t* buf, int len);

// AES init
#define AES_init_ctx(ctx, key, kt) AES_init_ctx_iv(ctx, key, kt, NULL)
void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, AESKeyType kt, const uint8_t* iv);

#endif

