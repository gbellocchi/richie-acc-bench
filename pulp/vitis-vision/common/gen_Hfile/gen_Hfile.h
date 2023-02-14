/* =====================================================================
 * Project:      Verification dataset generator.
 * Title:        gen_Hfile.h
 * Description:  Generate output header file.
 *
 * $Date:        14.11.2021
 * ===================================================================== */
/*
 * Copyright (C) 2021 University of Modena and Reggio Emilia..
 *
 * Author: Gianluca Bellocchi, University of Modena and Reggio Emilia.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

/* Functions declaration. */

static void gen_Hfile(
    char* val_name, 
    char* synth_data, 
    int width, 
    int height,
    int bit_depth
) {
  printf(">> Export synthetic %s to header file.\n", val_name);

  char filename[100];
  char h_body[1000];

  // Create file
  strcpy(filename, val_name);
  strcat(filename, ".h");

  FILE *fp = fopen(filename, "w+");

  // Generate header file
  strcpy(h_body, "__attribute__((aligned(32))) int32_t ");
  strcat(h_body, val_name);
  strcat(h_body, "[] = {\n");
  fprintf(fp, "%s", h_body);

  if(bit_depth==8) {
    for (int ii = 0; ii < height; ii++) {
      for (int jj = 0; jj < width; jj++) {
        if(ii*width+jj==width*height-1)
          fprintf(fp, "\t0x%02hhx\n", synth_data[ii*width+jj]);
        else
          fprintf(fp, "\t0x%02hhx,\n", synth_data[ii*width+jj]);
      }
    }
  } else {
    for (int ii = 0; ii < height; ii++) {
      for (int jj = 0; jj < width; jj++) {
        if(ii*width+jj==width*height-1)
          fprintf(fp, "\t0x%lx\n", synth_data[ii*width+jj]);
        else
          fprintf(fp, "\t0x%lx,\n", synth_data[ii*width+jj]);
      }
    }
  }

  fprintf(fp, "};");

  // Close file
  fclose(fp);
}