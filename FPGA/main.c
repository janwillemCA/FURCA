/**
 * @author Steven van der Vlist
 * @author Henri van de Munt
 * @author Jan Willem Castelein
 */

#include <stdio.h>
#include "includes.h"

#include <unistd.h>
#include <fcntl.h>

#include <altera_up_avalon_ps2.h>
#include <altera_up_ps2_keyboard.h>

#define         MAX_BUFFER                                      100

/* Definition of Task Stacks */
#define         TASK_STACKSIZE                          2048
OS_STK taskKeyboard_stk                                [TASK_STACKSIZE];
OS_STK task_Send_Data_stk                              [TASK_STACKSIZE];
OS_STK task_Receive_Data_stk                   [TASK_STACKSIZE];

/* Definition of Task Priorities */
#define         taskKeyboard_PRIORITY                   1
#define         task_Receive_Data_PRIORITY      2
#define         task_Send_Data_PRIORITY         3

/* Variables */
OS_EVENT * Mqueue; // message queue
void * Qmessages[20]; // message pointers pool

OS_EVENT *sem_Keyboard;

/**
 * task to receive data from the keyboard
 *
 * @Mqueue puts data from keyboard in message queue
 */

void taskKeyboard(void* pdata){
  INT8U err;
  alt_up_ps2_dev *ps2;
  KB_CODE_TYPE *decode_mode;
  alt_u8 buf;
  char ascii;
  char buffer[MAX_BUFFER];

  ps2 = alt_up_ps2_open_dev("/dev/PS2_Port");
  alt_up_ps2_init(ps2);
  while (1) {
      decode_scancode(ps2,decode_mode,&buf,&ascii);
      if (*decode_mode != 6) {
          if (*decode_mode == 1) {
              //printf("KEY1: %c\n",ascii);
              err = OSQPost(Mqueue, ascii);
            }
        } else {
          OSTimeDlyHMSM(0, 0, 0, 100);
        }
    }
}

/**
 * this task takes all data from the message queue and writes it to the RS232 port
 *
 *
 */

void task_Send_Data(void* pdata){
  INT8U err;
  char *msg;

  FILE * fp;
  while (1) {
      msg = OSQPend(Mqueue, 0, &err);
      fp = fopen(SERIAL_PORT_NAME, "w+r");
      if (fp == NULL) {
          printf("\nFile /RS232 not open for writing....");
        } else {
          fprintf(fp, "%c", msg);
        }
      fclose(fp);
    }
}

/**
 * this task reads all incoming data from the RS232 port
 *
 *
 */

void  task_Receive_Data(void* pdata){
  while (1) {
      OSTimeDlyHMSM(0, 0, 5, 0);
      //@TODO
    }
}

/** The main function creates two task and starts multi-tasking
 *
 *
 */

int main(void){

  Mqueue = OSQCreate(&Qmessages[0], 20);                // Create message queue
  sem_Keyboard = OSSemCreate(1);                        // Sem for keyboard

  OSTaskCreateExt(taskKeyboard,         NULL,   (void *)&taskKeyboard_stk[TASK_STACKSIZE-1],            taskKeyboard_PRIORITY,          taskKeyboard_PRIORITY,          taskKeyboard_stk,       TASK_STACKSIZE, NULL,   0);
  OSTaskCreateExt(task_Send_Data,       NULL,   (void *)&task_Send_Data_stk[TASK_STACKSIZE-1],          task_Send_Data_PRIORITY,        task_Send_Data_PRIORITY,        task_Send_Data_stk,     TASK_STACKSIZE, NULL,   0);
  OSTaskCreateExt(task_Receive_Data,    NULL,   (void *)&task_Receive_Data_stk[TASK_STACKSIZE-1],       task_Receive_Data_PRIORITY,     task_Receive_Data_PRIORITY,     task_Receive_Data_stk,  TASK_STACKSIZE, NULL,   0);


  OSStart();
  return 0; // this line will never reached
}
