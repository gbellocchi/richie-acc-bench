/* =====================================================================
 * Project:      COMP4DRONES - Anomaly Detection
 * Title:        main.cpp
 * Description:  AES-128 software implementation.
 * 
 * $Date:        19.5.2022
 * ===================================================================== */

/* Libraries. */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <errno.h>

#include <aes_sw.h>
#include <LCM.h>

int main(int argc, char **argv)
{
  message_t temp;
  char decrypted[TAKS_PAYLOAD_LEN];

  if (argc < 2)
      return -1;

	initNodes();
  printf("\n\nINPUT MESSAGE - PLAIN TEXT: %s",argv[1]);

  int r = encryptMessage(&temp, argv[1]);

  if (r == -1) {
      printf("\n\n    ENCRYPT ERROR");
      printf("\n||---------------------------------------------------------------------------------------||\n\n\n\n\n\n\n\n");
      return 0;
  }

  printf("\n\n    Message Encrypted \n");
  printf("\nRESULT: %d", r);

  r = decryptMessage(decrypted, &temp);
    
  if (r == -1) {
      printf("\n\n    DECRYPT ERROR");
      printf("\n||---------------------------------------------------------------------------------------||\n\n\n\n\n\n\n\n");
      return 0;
  }

  printf("\n\n    Message Decrypted \n");
  printf("\nRESULT: %d\n\n", r);

  return 0;
}

