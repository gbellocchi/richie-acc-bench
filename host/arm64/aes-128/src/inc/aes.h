#ifndef WT_CRYPTO_AES_H
#define WT_CRYPTO_AES_H

#include "axis_aes128_drivers/axis_aes128.c"
#include <time.h>

#define KEYLEN 128
#define NROUNDS 10
#define TAKS_MAC_LEN 4

//#define HARDWARE
#define TIMING

struct AES_Context {
	uint8_t key[KEYLEN];
	uint8_t rconst;
	uint8_t schedule[KEYLEN*44];
};

static uint8_t Table_SubByte[256] = {
	0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
	0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
	0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
	0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
	0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
	0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
	0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
	0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
	0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
	0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
	0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
	0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
	0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
	0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
	0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
	0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16
};

static uint8_t Table_InvSubByte[256] = {
	0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
	0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
	0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
	0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
	0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
	0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
	0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
	0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
	0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
	0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
	0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
	0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
	0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
	0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
	0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
	0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d
};

static uint8_t Table_MixColumn[16] = {
	0x02, 0x03, 0x01, 0x01,
	0x01, 0x02, 0x03, 0x01,
	0x01, 0x01, 0x02, 0x03,
	0x03, 0x01, 0x01, 0x02
};

static uint8_t Table_InvMixColumn[16] = {
	0x0E, 0x0B, 0x0D, 0x09,
	0x09, 0x0E, 0x0B, 0x0D,
	0x0D, 0x09, 0x0E, 0x0B,
	0x0B, 0x0D, 0x09, 0x0E
};

void AES_CBC_MAC(uint8_t *output, const uint8_t *key, const uint8_t *text, uint16_t plen);
void AES_Decrypt_CTR(uint8_t *output, const uint8_t *key/*, const uint8_t *iv*/, const uint8_t *cipher, uint16_t clen);
void AES_Encrypt_CTR(uint8_t *output, const uint8_t *key/*, const uint8_t *iv*/, const uint8_t *plain, uint16_t plen);;
void AES_Util_Increment(uint8_t *number, size_t size);
void AES_Decrypt_Block(uint8_t *output, const uint8_t *key, const uint8_t *cblock);
void AES_Encrypt_Block(uint8_t *output, const uint8_t *key, const uint8_t *block);
size_t PKCS7_Padding(uint8_t *src, size_t s);
static void GenerateRoundKey(uint8_t *out_key, uint8_t *r);
static uint8_t *GetRoundKey(struct AES_Context *ctx, int nround);
static void InitKey(struct AES_Context *ctx, const uint8_t *key);
static void AddRoundKey(uint8_t *state, uint8_t *roundkey);
static void InvShiftRow(uint8_t *state);
static void MixColumn(uint8_t *state, uint8_t *matrix);
static void ShiftRow(uint8_t *state);
static uint8_t RijndaelMul(uint8_t a, uint8_t b);
static void SubByte(uint8_t *state, uint8_t *table);


static uint8_t RijndaelMul(uint8_t a, uint8_t b)
{
	uint8_t p = 0;
	while (a && b) {
		if (b & 1) p ^= a;
		if (a & 0x80) a = (a << 1) ^ 0x11B;
		else a <<= 1;
		b >>= 1;
	}
	return p;
}

static void SubByte(uint8_t *state, uint8_t *table)
{
	uint8_t i;
	for (i = 0; i < 16; i++)
		state[i] = table[state[i]];
}

static void ShiftRow(uint8_t *state)
{
	uint8_t t1,t2;
	t1 = state[1]; state[1] = state[5]; state[5] = state[9]; state[9] = state[13]; state[13] = t1;
	t1 = state[2]; t2 = state[6]; state[2] = state[10]; state[6] = state[14]; state[10] = t1; state[14] = t2;
	t1 = state[15]; state[15] = state[11]; state[11] = state[7]; state[7] = state[3]; state[3] = t1;
}

static void MixColumn(uint8_t *state, uint8_t *matrix)
{
	uint8_t i, j;
	uint8_t workstate[16];
	for (i = 0; i < 16; ++i)
		workstate[i] = state[i];
	for (i = 0; i < 4; ++i) {
		for (j = 0; j < 4; ++j) {
			state[j*4 + i] =  RijndaelMul(matrix[i*4+0], workstate[j*4+0]);
			state[j*4 + i] ^= RijndaelMul(matrix[i*4+1], workstate[j*4+1]);
			state[j*4 + i] ^= RijndaelMul(matrix[i*4+2], workstate[j*4+2]);
			state[j*4 + i] ^= RijndaelMul(matrix[i*4+3], workstate[j*4+3]);
		}
	}
}

static void InvShiftRow(uint8_t *state)
{
	uint8_t t1,t2;
	t1 = state[13]; state[13] = state[9]; state[9] = state[5]; state[5] = state[1]; state[1] = t1;
	t1 = state[10]; t2 = state[14]; state[10] = state[2]; state[14] = state[6]; state[2] = t1; state[6] = t2;
	t1 = state[3]; state[3] = state[7]; state[7] = state[11]; state[11] = state[15]; state[15] = t1;
}

static void AddRoundKey(uint8_t *state, uint8_t *roundkey)
{
	int i;
	for (i = 0; i < 16; i++)
		state[i] ^= roundkey[i];
}

static void InitKey(struct AES_Context *ctx, const uint8_t *key)
{
	int i, j;
	for (i = 0; i < 16; ++i)
		ctx->schedule[i] = key[i];
	ctx->rconst = 0x1; //0b00000001
	for (i = 1; i <= NROUNDS; ++i) {
		for (j = 0; j < 16; ++j)
			ctx->schedule[i*16+j] = ctx->schedule[(i-1)*16+j];
		GenerateRoundKey(&ctx->schedule[i*16], &ctx->rconst);
	}
}

static uint8_t *GetRoundKey(struct AES_Context *ctx, int nround)
{
	int i;
	for (i = 0; i < 16; ++i)
		ctx->key[i] = ctx->schedule[nround*16+i];
	return ctx->key;
}

static void GenerateRoundKey(uint8_t *out_key, uint8_t *r)
{
	int col;
	uint8_t t[4];
	uint8_t rconst = *r;
	for (col = 0; col < 4; ++col) {
		if (col == 0) {
			t[0] = out_key[0] ^ Table_SubByte[out_key[13]] ^ rconst;
			t[1] = out_key[1] ^ Table_SubByte[out_key[14]];
			t[2] = out_key[2] ^ Table_SubByte[out_key[15]];
			t[3] = out_key[3] ^ Table_SubByte[out_key[12]];
			*r = RijndaelMul(rconst, 0x1);//0b00000010
		}
		else {
			t[0] = out_key[col*4 - 4 + 0] ^ out_key[col*4 + 0];
			t[1] = out_key[col*4 - 4 + 1] ^ out_key[col*4 + 1];
			t[2] = out_key[col*4 - 4 + 2] ^ out_key[col*4 + 2];
			t[3] = out_key[col*4 - 4 + 3] ^ out_key[col*4 + 3];
		}
		out_key[col*4+0] = t[0];
		out_key[col*4+1] = t[1];
		out_key[col*4+2] = t[2];
		out_key[col*4+3] = t[3];
	}
}

size_t PKCS7_Padding(uint8_t *src, size_t s)
{
	int i;
	size_t pad_size;
	size_t result = s;
	pad_size = 16 - (s & 0xF);
	if (pad_size == 0)
		pad_size = 16;
	result += pad_size;
	for (i = s; i < result; ++i)
		src[i] = (uint8_t)pad_size;
	return result;
}


void AES_Encrypt_Block(uint8_t *output, const uint8_t *key, const uint8_t *block)
{
	struct AES_Context context;
	uint8_t *state = output;
	int i, R;

	for (i = 0; i < 16; ++i)
		state[i] = block[i];
	InitKey(&context, key);
	GetRoundKey(&context, 0);
	AddRoundKey(state, context.key);
	for (R = 0; R < NROUNDS; ++R) {
		SubByte(state, Table_SubByte);
		ShiftRow(state);
		if (R != NROUNDS - 1)
			MixColumn(state, Table_MixColumn);
		GetRoundKey(&context, R+1);
		AddRoundKey(state, context.key);
	}
}

void AES_Decrypt_Block(uint8_t *output, const uint8_t *key, const uint8_t *cblock)
{
	struct AES_Context context;
	uint8_t *state = output;
	int i, R;

	for (i = 0; i < 16; ++i)
		state[i] = cblock[i];
	InitKey(&context, key);
	for (R = 9; R >= 0; --R) {
		GetRoundKey(&context, R+1);
		AddRoundKey(state, context.key);
		if (R != 9)
			MixColumn(state, Table_InvMixColumn);
		InvShiftRow(state);
		SubByte(state, Table_InvSubByte);
	}
	GetRoundKey(&context, 0);
	AddRoundKey(state, context.key);
}

void AES_Util_Increment(uint8_t *number, size_t size)
{
	int i = size - 1;
	while (number[i] == 0xFF && i >= 0) {
		number[i] = 0;
		i--;
	}
	if (i < 0)
		i = size - 1;
	number[i]++;
}

void printOwn(uint8_t *input, int size, const char *message){
    int i;
    printf("\n\n%s",message);
    for(i = 0; i<size; i++)
        printf("%02x", input[i]);
}
void AES_Encrypt_CTR(uint8_t *output, const uint8_t *key, const uint8_t *plain, uint16_t plen)
{
	#ifdef HARDWARE // use the hardware accelerator
		axis_aes128_load("axis_aes128_drivers/axis_aes128_design_1_wrapper.bin");
        aes128_addr_t axis_aes128_addr;
        uint8_t rc[] = {0x01};

        // 1. init the accelerator
        axis_aes128_init(&axis_aes128_addr);
	#endif
	
	#ifdef TIMING
	clock_t start, end;
	double elapsed_time;
	start = clock();
	#endif

	uint8_t nblocks = (plen >> 4); // length should be a multiple of the blocksize (16)
	uint8_t block[16];
	uint8_t feedback[16];
	int i,b;

	for (i = 0; i < 16; ++i)
		feedback[i] = 0;
	//	feedback[i] = iv[i];

	
	for (b = 0; b < nblocks; ++b) {
		#ifdef HARDWARE // use the hardware accelerator
        // 2. send data to the accelerator
		uint8_t *_key = (uint8_t *) key;

        axis_aes128_send(&axis_aes128_addr, feedback, _key, rc);

		// 3. receive data back 
		axis_aes128_wait(&axis_aes128_addr, block);
 	   	#else	
		AES_Encrypt_Block(block, key, feedback);
		#endif
		for (i = 0; i < 16; ++i)
			output[b*16+i] = plain[b*16 + i] ^ block[i];
		AES_Util_Increment(feedback, 16);
    	printOwn(block,16,"        ENCRYPT - FEEDBACK: ");
    	printf("\n\n\t\t NBLOCKS:\t%d\n\n", nblocks);
	}
	#ifdef TIMING
	end = clock();
    elapsed_time = ((double) (end - start)) / CLOCKS_PER_SEC;
		#ifdef HARDWARE
		printf("******************* HARDWARE ELAPSED **********************\n\n");
		printf("HARDWARE ELAPSED TIME: %f", elapsed_time);
		#else
		printf("******************* SOFTWARE ELAPSED **********************\n\n");
		printf("SOFTWARE ELAPSED TIME: %f", elapsed_time);
		#endif
	printf("\n\n***********************************************************\n");
	#endif
	
	#ifdef HARDWARE
	axis_aes128_stop(&axis_aes128_addr);
	#endif
}

void AES_Decrypt_CTR(uint8_t *output, const uint8_t *key, const uint8_t *cipher, uint16_t clen)
{
	#ifdef HARDWARE // use the hardware accelerator
		axis_aes128_load("axis_aes128_drivers/axis_aes128_design_1_wrapper.bin");
        aes128_addr_t axis_aes128_addr;
        uint8_t rc[] = {0x01};

        // 1. init the accelerator
        axis_aes128_init(&axis_aes128_addr);
	#endif

	#ifdef TIMING
	clock_t start, end;
	double elapsed_time;
	start = clock();
	#endif

	uint8_t nblocks = (clen >> 4); // length should be a multiple of the blocksize (16)
	uint8_t block[16];
	uint8_t feedback[16];
	int i, b;

	for (i = 0; i < 16; ++i)
		feedback[i] = 0; // TODO
		//feedback[i] = iv[i];

	for (b = 0; b < nblocks; ++b) {
		#ifdef HARDWARE // use the hardware accelerator
        // 2. send data to the accelerator
		uint8_t *_key = (uint8_t *) key;

        axis_aes128_send(&axis_aes128_addr, feedback, _key, rc);

		// 3. receive data back 
		axis_aes128_wait(&axis_aes128_addr, block);
 	   	#else
		AES_Encrypt_Block(block, key, feedback);
		#endif
		for (i = 0; i < 16; ++i)
			output[b*16+i] = cipher[b*16 + i] ^ block[i];
		AES_Util_Increment(feedback, 16);
	}

	#ifdef TIMING
	end = clock();
    elapsed_time = ((double) (end - start)) / CLOCKS_PER_SEC;
		#ifdef HARDWARE
		printf("\n******************* HARDWARE ELAPSED **********************\n\n");
		printf("HARDWARE ELAPSED TIME: %f", elapsed_time);
		#else
		printf("\n******************* SOFTWARE ELAPSED **********************\n\n");
		printf("SOFTWARE ELAPSED TIME: %f", elapsed_time);
		#endif
	printf("\n\n***********************************************************\n");
	#endif
	
	#ifdef HARDWARE
	axis_aes128_stop(&axis_aes128_addr);
	#endif
}

void AES_CBC_MAC(uint8_t *output, const uint8_t *key, const uint8_t *text, uint16_t plen)
{
	uint8_t nblocks = (plen >> 4); // length should be a multiple of the blocksize (16)
	uint8_t block[16];
	uint8_t feedback[16];
	int i, b;

	// NONCE
	for (i = 0; i < 16; ++i)
		feedback[i] = 0;
	AES_Encrypt_Block(feedback, key, feedback);
	for (b = 0; b < nblocks; ++b) {
		for (i = 0; i < 16; ++i)
			block[i] = text[b*16 + i] ^ feedback[i];
		AES_Encrypt_Block(feedback, key, block);
	}

	for (i = 0; i < TAKS_MAC_LEN; ++i)
		output[i] = feedback[i];
}

#endif /* end of WT_CRYPTO_AES_H */
