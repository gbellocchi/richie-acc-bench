/************************************************************************
 * Filename:        axis_aes128.h
 * Description:     AES 128 Driver Header
 * Date:            28/04/2022
 ************************************************************************/
 
#ifndef AXIS_AES128_H
#define AXIS_AES128_H
 
/************************* Constant Definitions *************************/
#define OFFS_DMA_TEXT_DATA 0x00A0000000
#define OFFS_DMA_KEY_DATA 0x00A0010000
#define OFFS_DMA_RC_DATA 0x00A0020000
 
#define OFFS_DMA_ENCRYPT_DATA 0x00A0030000

#define SIZE_VIRTUAL_ADDR 65535 //64K

#include <stdint.h>
 

/************************* Struct Definitions **************************/

typedef struct aes128_addr{
	int ddr_memory;								// DDR memory
	
	uint32_t *dma_TEXT_virtual_addr;		// DMA MM2S: text data
	uint32_t *dma_KEY_virtual_addr;			// DMA MM2S: key data
	uint32_t *dma_RC_virtual_addr;			// DMA MM2S: rc data
	uint32_t *dma_ENCRYPTED_virtual_addr;	// DMA S2MM: encrypted text data
	
	uint32_t *virtual_src_TEXT_addr;		// text addr
	uint32_t *virtual_src_KEY_addr;			// key addr
	uint32_t *virtual_src_RC_addr;			// rc addr
	uint32_t *virtual_dst_ENCRYPTED_addr;	// encrypted text addr
} aes128_addr_t;
		

/************************ Function Definitions *************************/

int axis_aes128_load(char *accelerator);
 
int axis_aes128_unload();

int axis_aes128_init(aes128_addr_t *addr);

int axis_aes128_stop(aes128_addr_t *addr);

void axis_aes128_send(aes128_addr_t *addr, uint8_t *text_data, uint8_t *key_data, uint8_t *rc_data);

void axis_aes128_wait(aes128_addr_t *addr, uint8_t *encrypted_text);

void axis_aes128_send_wait(aes128_addr_t *addr, uint8_t *encrypted_text, uint8_t *text_data, uint8_t *key_data, uint8_t *rc_data);
 
#endif
