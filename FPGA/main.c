/*************************************************************************
 * Copyright (c) 2004 Altera Corporation, San Jose, California, USA.      *
 * All rights reserved. All use of this software and documentation is     *
 * subject to the License Agreement located at the end of this file below.*
 **************************************************************************
 * Description:                                                           *
 * The following is a simple hello world program running MicroC/OS-II.The *
 * purpose of the design is to be a very simple application that just     *
 * demonstrates MicroC/OS-II running on NIOS II.The design doesn't account*
 * for issues such as checking system call return codes. etc.             *
 *                                                                        *
 * Requirements:                                                          *
 *   -Supported Example Hardware Platforms                                *
 *     Standard                                                           *
 *     Full Featured                                                      *
 *     Low Cost                                                           *
 *   -Supported Development Boards                                        *
 *     Nios II Development Board, Stratix II Edition                      *
 *     Nios Development Board, Stratix Professional Edition               *
 *     Nios Development Board, Stratix Edition                            *
 *     Nios Development Board, Cyclone Edition                            *
 *   -System Library Settings                                             *
 *     RTOS Type - MicroC/OS-II                                           *
 *     Periodic System Timer                                              *
 *   -Know Issues                                                         *
 *     If this design is run on the ISS, terminal output will take several*
 *     minutes per iteration.                                             *
 **************************************************************************/


#include <stdio.h>
#include "includes.h"
#include <unistd.h>
#include <fcntl.h>

/* Definition of Task Stacks */
#define TASK_STACKSIZE 2048
OS_STK task1_stk[TASK_STACKSIZE];
OS_STK task2_stk[TASK_STACKSIZE];

/* Definition of Task Priorities */

#define TASK1_PRIORITY 1
#define TASK2_PRIORITY 2

/* Prints "Hello World" and sleeps for three seconds */
void task1(void * pdata) {
  while (1) {

      OSTimeDlyHMSM(0, 0, 3, 0);
    }
}
/* Prints "Hello World" and sleeps for three seconds */
void task2(void * pdata) {
  FILE * fp;
  char test[10];
  fp = fopen(SERIAL_PORT_NAME, "w+r");
  while (1) {
      if (fp == NULL) {
          printf("\nFile /RS232 not open for writing....");
        } else {
          fprintf(fp, "%s", "Hallo Allemaal. ");
          if (fscanf(fp, "%s", test) == 1) {
              printf("%s\n", test);
            }
        }

      OSTimeDlyHMSM(0, 0, 1, 0);
    }
  fclose(fp);
}
/* The main function creates two task and starts multi-tasking */
int main(void) {
  OSTaskCreateExt(task1,
                  NULL,
                  (void * ) &task1_stk[TASK_STACKSIZE - 1],
                  TASK1_PRIORITY,
                  TASK1_PRIORITY,
                  task1_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
  OSTaskCreateExt(task2,
                  NULL,
                  (void * ) &task2_stk[TASK_STACKSIZE - 1],
                  TASK2_PRIORITY,
                  TASK2_PRIORITY,
                  task2_stk,
                  TASK_STACKSIZE,
                  NULL,
                  0);
  OSStart();

  return 0;
}
