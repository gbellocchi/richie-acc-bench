/************************************************************************
 * Filename:        axis_aes128.c
 * Description:     Axi DMA Driver
 * Date:            28/04/2022
 ************************************************************************/

#include "axi_dma_driver.c"
#include "axis_aes128.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#ifndef DEBUG
//#define DEBUG 
#endif

/*  load_axis_aes128():    load the accelerator using fpgautil
 *
 *	char *accelerator:          the accelerator file name to load
 *
 * 	return 0 if the accelerator has been loaded successfully
 */
int axis_aes128_load(char *accelerator){
	char *cmd;
	int size0 = strlen("fpgautil -b ");
	int size1 = strlen(accelerator);
	
	cmd = (char *)malloc(sizeof(char) * (size0+size1));
	
	if (cmd == NULL)
		return -1;
	
	strcpy(cmd, "fpgautil -b "); 
	strcat(cmd, accelerator);
	
	int sys_return = system(cmd);

	if(sys_return == -1){
  		// The system method failed
	}

	free(cmd);
	
	return 0;
}
 
/*  unload_axis_aes128():    unload the accelerator using fpgautil
 *
 * 	return 0 if the accelerator has been unloaded successfully
 */
int axis_aes128_unload(){
//	system("fpgautil -b dummy.bin");
	return 0;
}

/*  axis_aes128_init():    		init the accelerator
 *
 *  aes128_addr_t *addr:		the pointer to the addresses
 *
 * 	return -1 					an error occurs
 *  return  0  					accelerator initiated
 */
int axis_aes128_init(aes128_addr_t *addr){
	
	// open the ddr
	// -------------------------------------------------------------------
	if((addr->ddr_memory = open("/dev/mem", O_RDWR | O_SYNC)) < 0){
		printf("Error opening the DDR memory.\n");
		return -1;
	}
	
	// mapping the dma
	// -------------------------------------------------------------------
	addr->dma_TEXT_virtual_addr = (uint32_t *)mmap(NULL, SIZE_VIRTUAL_ADDR, PROT_READ | PROT_WRITE, MAP_SHARED, addr->ddr_memory, OFFS_DMA_TEXT_DATA);		 // MM2S
	if(addr->dma_TEXT_virtual_addr == MAP_FAILED){
		printf("Error mapping DMA TEXT virtual address.\n");
		return -1;
	}
	
	addr->dma_KEY_virtual_addr = (uint32_t *)mmap(NULL, SIZE_VIRTUAL_ADDR, PROT_READ | PROT_WRITE, MAP_SHARED, addr->ddr_memory, OFFS_DMA_KEY_DATA);			 // MM2S
	if(addr->dma_KEY_virtual_addr == MAP_FAILED){
		printf("Error mapping DMA KEY virtual address.\n");
		return -1;
	}
	
	addr->dma_RC_virtual_addr = (uint32_t *)mmap(NULL, SIZE_VIRTUAL_ADDR, PROT_READ | PROT_WRITE, MAP_SHARED, addr->ddr_memory, OFFS_DMA_RC_DATA);			 // MM2S
	if(addr->dma_RC_virtual_addr == MAP_FAILED){
		printf("Error mapping DMA RC virtual address.\n");
		return -1;
	}
	
	addr->dma_ENCRYPTED_virtual_addr = (uint32_t *)mmap(NULL, SIZE_VIRTUAL_ADDR, PROT_READ | PROT_WRITE, MAP_SHARED, addr->ddr_memory, OFFS_DMA_ENCRYPT_DATA); // S2MM
	if(addr->dma_ENCRYPTED_virtual_addr == MAP_FAILED){
		printf("Error mapping DMA ENCRYPTED TEXT virtual address.\n");
		return -1;
	}
	
	
	// mapping the source(s)
	// -------------------------------------------------------------------
	addr->virtual_src_TEXT_addr = (uint32_t *)mmap(NULL, SIZE_VIRTUAL_ADDR, PROT_READ | PROT_WRITE, MAP_SHARED, addr->ddr_memory, 0x0e000000);
	if(addr->virtual_src_TEXT_addr == MAP_FAILED){
		printf("Error mapping TEXT virtual source address.\n");
		return -1;
	}
	
	addr->virtual_src_KEY_addr = (uint32_t *)mmap(NULL, SIZE_VIRTUAL_ADDR, PROT_READ | PROT_WRITE, MAP_SHARED, addr->ddr_memory, 0x0e010000);
	if(addr->virtual_src_KEY_addr == MAP_FAILED){
		printf("Error mapping KEY virtual source address.\n");
		return -1;
	}
	
	addr->virtual_src_RC_addr = (uint32_t *)mmap(NULL, SIZE_VIRTUAL_ADDR, PROT_READ | PROT_WRITE, MAP_SHARED, addr->ddr_memory, 0x0e020000);
	if(addr->virtual_src_RC_addr == MAP_FAILED){
		printf("Error mapping RC virtual source address.\n");
		return -1;
	}
	
	// mapping the destination(s)
	// -------------------------------------------------------------------
	addr->virtual_dst_ENCRYPTED_addr = (uint32_t *)mmap(NULL, SIZE_VIRTUAL_ADDR, PROT_READ | PROT_WRITE, MAP_SHARED, addr->ddr_memory, 0x0f000000);
	if(addr->virtual_dst_ENCRYPTED_addr == MAP_FAILED){
		printf("Error mapping ENCRYPTED TEXT virtual destination address.\n");
		return -1;
	}

	// reset the dma
	// -------------------------------------------------------------------
	write_dma(addr->dma_TEXT_virtual_addr, MM2S_CONTROL_REGISTER, RESET_DMA);
	write_dma(addr->dma_KEY_virtual_addr, MM2S_CONTROL_REGISTER, RESET_DMA);
	write_dma(addr->dma_RC_virtual_addr, MM2S_CONTROL_REGISTER, RESET_DMA);
	write_dma(addr->dma_ENCRYPTED_virtual_addr, S2MM_CONTROL_REGISTER, RESET_DMA);

	#ifdef DEBUG
		// check status of dma
		dma_mm2s_status_print(addr->dma_TEXT_virtual_addr);
		dma_mm2s_status_print(addr->dma_KEY_virtual_addr);
		dma_mm2s_status_print(addr->dma_RC_virtual_addr);
		dma_s2mm_status_print(addr->dma_ENCRYPTED_virtual_addr);
	#endif

	// halt the dma
	// -------------------------------------------------------------------
	// printf("Halt the DMA.\n");
	write_dma(addr->dma_TEXT_virtual_addr, MM2S_CONTROL_REGISTER, HALT_DMA);
	write_dma(addr->dma_KEY_virtual_addr, MM2S_CONTROL_REGISTER, HALT_DMA);
	write_dma(addr->dma_RC_virtual_addr, MM2S_CONTROL_REGISTER, HALT_DMA);
	write_dma(addr->dma_ENCRYPTED_virtual_addr, S2MM_CONTROL_REGISTER, HALT_DMA);

	#ifdef DEBUG
		// check status of dma
		dma_mm2s_status_print(addr->dma_TEXT_virtual_addr);
		dma_mm2s_status_print(addr->dma_KEY_virtual_addr);
		dma_mm2s_status_print(addr->dma_RC_virtual_addr);
		dma_s2mm_status_print(addr->dma_ENCRYPTED_virtual_addr);
	#endif

	// enable all interrupts
	// -------------------------------------------------------------------
	//printf("Enable all interrupts.\n");
	write_dma(addr->dma_TEXT_virtual_addr, MM2S_CONTROL_REGISTER, ENABLE_ALL_IRQ); // ENABLE_ALL_IRQ is bit 14, 13, 12
	write_dma(addr->dma_KEY_virtual_addr, MM2S_CONTROL_REGISTER, ENABLE_ALL_IRQ);	 // ENABLE_ALL_IRQ is bit 14, 13, 12
	write_dma(addr->dma_RC_virtual_addr, MM2S_CONTROL_REGISTER, ENABLE_ALL_IRQ);	 // ENABLE_ALL_IRQ is bit 14, 13, 12
	write_dma(addr->dma_ENCRYPTED_virtual_addr, S2MM_CONTROL_REGISTER, ENABLE_ALL_IRQ);

	#ifdef DEBUG
	dma_mm2s_status_print(addr->dma_TEXT_virtual_addr);
	dma_mm2s_status_print(addr->dma_KEY_virtual_addr);
	dma_mm2s_status_print(addr->dma_RC_virtual_addr);
	dma_s2mm_status_print(addr->dma_ENCRYPTED_virtual_addr);
	#endif

	// writing the source and destination registers
	// -------------------------------------------------------------------
	//	printf("Writing source address of the data from MM2S in DDR...\n"); // source pointer (pointer to reading data)
	write_dma(addr->dma_TEXT_virtual_addr, MM2S_SRC_ADDRESS_REGISTER, 0x0e000000);
	write_dma(addr->dma_KEY_virtual_addr, MM2S_SRC_ADDRESS_REGISTER, 0x0e010000);
	write_dma(addr->dma_RC_virtual_addr, MM2S_SRC_ADDRESS_REGISTER, 0x0e020000);
	//printf("Writing the destination address for the data from S2MM in DDR...\n");
	write_dma(addr->dma_ENCRYPTED_virtual_addr, S2MM_DST_ADDRESS_REGISTER, 0x0f000000);

	#ifdef DEBUG
	dma_mm2s_status_print(addr->dma_TEXT_virtual_addr);
	dma_mm2s_status_print(addr->dma_KEY_virtual_addr);
	dma_mm2s_status_print(addr->dma_RC_virtual_addr);
	dma_s2mm_status_print(addr->dma_ENCRYPTED_virtual_addr);
	#endif 

	return 0;
}

/*  axis_aes128_init():    		stop the accelerator
 *
 *  aes128_addr_t *addr:		the pointer to the addresses
 *
 * 	return -1 					an error occurs
 *  return  0  					accelerator stopped
 */
int axis_aes128_stop(aes128_addr_t *addr){

	// unmapping
	if(munmap(addr->dma_TEXT_virtual_addr, SIZE_VIRTUAL_ADDR) != 0){
		printf("Error unmapping DMA TEXT virtual address.\n");
		return -1;
	}
		
	if(munmap(addr->dma_KEY_virtual_addr, SIZE_VIRTUAL_ADDR) != 0){
		printf("Error unmapping DMA KEY virtual address.\n");
		return -1;
	}
	
	if(munmap(addr->dma_RC_virtual_addr, SIZE_VIRTUAL_ADDR) != 0){
		printf("Error unmapping DMA RC virtual address.\n");
		return -1;
	}
	
	if(munmap(addr->dma_ENCRYPTED_virtual_addr, SIZE_VIRTUAL_ADDR) != 0){
		printf("Error unmapping DMA ENCRYPTED TEXT virtual address.\n");
		return -1;
	}
	
	if(munmap(addr->virtual_src_TEXT_addr, SIZE_VIRTUAL_ADDR) != 0){
		printf("Error unmapping TEXT virtual source address.\n");
		return -1;
	}
	if(munmap(addr->virtual_src_KEY_addr, SIZE_VIRTUAL_ADDR) != 0){
		printf("Error unmapping KEY virtual source address.\n");
		return -1;
	}
	if(munmap(addr->virtual_src_RC_addr, SIZE_VIRTUAL_ADDR) != 0){
		printf("Error unmapping RC virtual source address.\n");
		return -1;
	}
	if(munmap(addr->virtual_dst_ENCRYPTED_addr, SIZE_VIRTUAL_ADDR) != 0){
		printf("Error unmapping ENCRYPTED TEXT virtual destination address.\n");
		return -1;
	}

	// close /dev/mem
	if(close(addr->ddr_memory) != 0){
		printf("Error closing the DDR memory.\n");
		return -1;
	}
	
	return 0;
}

/*  axis_aes128_send():    		send data the accelerator
 *
 *	uint8_t *text_data          128-bit text message
 *  uint8_t *key_data:			128-bit key
 *  uint8_t *rc_data:			8-bit round_const
 *
 */
void axis_aes128_send(aes128_addr_t *addr, uint8_t *text_data, uint8_t *key_data, uint8_t *rc_data){

	//aes128_addr_t aes128_addr = (aes128_addr_t)*addr;	// saving the addresses already opened
	
	// write source data
	for(int i = 0; i<16; ++i){
		addr->virtual_src_TEXT_addr[i] = (uint32_t)text_data[i];
		addr->virtual_src_KEY_addr[i] = (uint32_t)key_data[i];
	}
	addr->virtual_src_RC_addr[0] = (uint32_t)rc_data[0];
	

	// run the MM2S channel(s)
	// -------------------------------------------------------------------
	//printf("Run the MM2S input channels.\n");
	write_dma(addr->dma_RC_virtual_addr, MM2S_CONTROL_REGISTER, RUN_DMA);
	write_dma(addr->dma_KEY_virtual_addr, MM2S_CONTROL_REGISTER, RUN_DMA);
	write_dma(addr->dma_TEXT_virtual_addr, MM2S_CONTROL_REGISTER, RUN_DMA);
	#ifdef DEBUG
		dma_mm2s_status_print(addr->dma_RC_virtual_addr);
		dma_mm2s_status_print(addr->dma_KEY_virtual_addr);
		dma_mm2s_status_print(addr->dma_TEXT_virtual_addr);
	#endif

	// run the S2MM channel(s)
	// -------------------------------------------------------------------
	//printf("Run the S2MM output channel.\n");
	write_dma(addr->dma_ENCRYPTED_virtual_addr, S2MM_CONTROL_REGISTER, RUN_DMA);
	#ifdef DEBUG
		dma_s2mm_status_print(addr->dma_ENCRYPTED_virtual_addr);
	#endif

	// writing the MM2S channel(s) transfer lengths
	// -------------------------------------------------------------------
	// printf("Writing MM2S transfer length of 16, 32 and 1 32-bit-words...\n");
	write_dma(addr->dma_TEXT_virtual_addr, MM2S_TRNSFR_LENGTH_REGISTER, 16 * 4);
	write_dma(addr->dma_KEY_virtual_addr, MM2S_TRNSFR_LENGTH_REGISTER, 16 * 4);
	write_dma(addr->dma_RC_virtual_addr, MM2S_TRNSFR_LENGTH_REGISTER, 1 * 4);
	#ifdef DEBUG
		dma_mm2s_status_print(addr->dma_TEXT_virtual_addr);
		dma_mm2s_status_print(addr->dma_KEY_virtual_addr);
		dma_mm2s_status_print(addr->dma_RC_virtual_addr);
	#endif

	// writing the S2MM channel(s) transfer lengths
	// -------------------------------------------------------------------
	// printf("Writing S2MM transfer length of 16 * 32-bit-words...\n");
	write_dma(addr->dma_ENCRYPTED_virtual_addr, S2MM_BUFF_LENGTH_REGISTER, 16 * 4);
	#ifdef DEBUG
		dma_s2mm_status_print(addr->dma_ENCRYPTED_virtual_addr);
	#endif
	
}

/*  axis_aes128_wait():    		wait the accelerator and return processed data
 *
 * 	uint8_t *encrypted_text:	the output chiped text
 */
void axis_aes128_wait(aes128_addr_t *addr, uint8_t *encrypted_text){
		
	// waiting for synchronizations
	// -------------------------------------------------------------------
	// printf("Waiting for MM2S and S2MM synchronizations...\n");
	dma_mm2s_sync(addr->dma_TEXT_virtual_addr);
	dma_mm2s_sync(addr->dma_KEY_virtual_addr);
	dma_mm2s_sync(addr->dma_RC_virtual_addr);
	dma_s2mm_sync(addr->dma_ENCRYPTED_virtual_addr);
	#ifdef DEBUG
		dma_mm2s_status_print(addr->dma_TEXT_virtual_addr);
		dma_mm2s_status_print(addr->dma_KEY_virtual_addr);
		dma_mm2s_status_print(addr->dma_RC_virtual_addr);

		dma_s2mm_status_print(addr->dma_ENCRYPTED_virtual_addr);
	#endif	
	
	// read data from destination 
	uint8_t *out = encrypted_text;
	for(int i = 0; i<16; ++i)
		out[i] = (uint8_t)addr->virtual_dst_ENCRYPTED_addr[i];
	
}

/*  axis_aes128_send_wait():    send data and wait the accelerator
 *
 *	uint8_t *text_data          128-bit text message
 *  uint8_t *key_data:			128-bit key
 *  uint8_t *rc_data:			8-bit round_const
 *
 * 	uint8_t *encrypted_text:	output chiped text
 * 
 * 	return 
 */
void axis_aes128_send_wait(aes128_addr_t *addr, uint8_t *encrypted_text, uint8_t *text_data, uint8_t *key_data, uint8_t *rc_data){
	
	axis_aes128_send(addr, text_data, key_data, rc_data);

	axis_aes128_wait(addr, encrypted_text);
}
