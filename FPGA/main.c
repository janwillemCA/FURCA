/**
 * @author Steven van der Vlist
 * @author Henri van de Munt
 * @author Jan Willem Castelein
 */

#include <stdio.h>
#include <string.h>
#include "includes.h"

#include <unistd.h>
#include <fcntl.h>

#include <altera_up_avalon_ps2.h>
#include <altera_up_ps2_keyboard.h>
#include "altera_up_avalon_character_lcd.h"


#define         MAX_BUFFER                      100

/* Definition of Task Stacks */
#define         TASK_STACKSIZE                  2048
OS_STK taskKeyboard_stk                         [TASK_STACKSIZE];
OS_STK task_Send_Receive_Data_stk               [TASK_STACKSIZE];
OS_STK controlPingOutput_stk               [TASK_STACKSIZE];


/* Definition of Task Priorities */
#define         taskKeyboard_PRIORITY           1
#define         task_Send_Receive_Data_PRIORITY 2
#define         controlPingOutput_PRIORITY 3

/* Variables */
OS_EVENT * KeyboardQueue; // message queue
OS_EVENT * PingFrontQueue; // Queue for processing control data


void * KeyboardMessages[20]; // message pointers pool
void * PingFrontMessages[50];

OS_EVENT *sem_RS232;

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

  ps2 = alt_up_ps2_open_dev(PS2_PORT_NAME);
  alt_up_ps2_init(ps2);
  while (1) {
      decode_scancode(ps2,decode_mode,&buf,&ascii);
      if (*decode_mode != 6) {
          if (*decode_mode == 1) {
              //alt_printf("KEY1: %c\n",ascii);
              sprintf(buffer, "Pressed key: %c", ascii);
              displayTextLCD(buffer);
              err = OSQPost(KeyboardQueue, ascii);
            }
        } else {
          OSTimeDlyHMSM(0, 0, 0, 100);
        }
    }
}

/**
 * this task takes all data from the message queue and writes it to the RS232 port
 * this task reads all incoming data from the RS232 port
 *
 */

void task_Send_Receive_Data(void* pdata){
  INT8U err;
  char *msg;

  char incomingData[50];

  int distanceFront;

  FILE * fp;
  while (1) {
      OSSemPend(sem_RS232, 0, &err);
      fp = fopen(SERIAL_PORT_NAME, "r+");
      if (fp == NULL) {
          alt_printf("\nFile /RS232 not open for writing....");
        } else {
          msg = OSQPend(KeyboardQueue, 1, &err);
          if (err == OS_NO_ERR) {
              fprintf(fp, "%c", msg);
              printf("%c", msg);
            }
          if (fscanf(fp, "%s", incomingData) == 1) {
              int pos = 0;
              int startPos;
              int temp;

              startPos = pos;
              while (incomingData[pos] != '\0' && incomingData[pos] >= 'A' && incomingData[pos] <= 'Z') {
                  pos++;
                }
              char dD[10];             // dataDescription
              temp = 0;
              while (startPos < pos) {
                  dD[temp] = incomingData[startPos];
                  startPos++;
                  temp++;
                }
              dD[temp] = '\0';
              startPos = pos;
              while ((incomingData[pos] != '\0') && incomingData[pos] >= '0' && incomingData[pos] <= '9') {
                  pos++;
                }
              char d[10];             // data
              temp = 0;
              while (startPos < pos) {
                  d[temp] = incomingData[startPos];
                  startPos++;
                  temp++;
                }
              d[temp] = '\0';
              if (dD[0] == 'M') {
                  distanceFront = atoi(d);
                  //printf("%d\n", distanceFront);
                  err = OSQPost(PingFrontQueue, distanceFront);
                }
            }
          OSTimeDlyHMSM(0, 0, 0, 50);
        }
      fclose(fp);
      err = OSSemPost(sem_RS232);
    }
}

void controlPingOutput(void *pdata){

  INT8U err;
  int left;
  int right;
  int front;
  while (1) {
      // check right
      OSTimeDlyHMSM(0, 0, 0, 50);
      front = OSQPend(PingFrontQueue, 0, &err);
      printf("pend front %d\n", front);
      if (front < 100 && front > 0)
        err = OSQPost(KeyboardQueue, 'H'); // send hold


    }

}

/**
 * Used to display text to the lcd
 *
 * @param data to display
 */

void displayTextLCD(char * message) {
  // open the Character LCD port

  alt_up_character_lcd_dev * char_lcd_dev;
  char_lcd_dev = alt_up_character_lcd_open_dev(CHAR_LCD_16X2_NAME);
  if (char_lcd_dev == NULL)
    alt_printf("Error: could not open character LCD device\n");


  /* Initialise the character display */
  alt_up_character_lcd_init(char_lcd_dev);

  alt_up_character_lcd_string(char_lcd_dev, message);

//  /* Write in the second row */
  alt_up_character_lcd_set_cursor_pos(char_lcd_dev, 0, 1);

//  int i;
//  for (i = 0; i < 16; i++) {
//    alt_up_character_lcd_erase_pos(char_lcd_dev, i, 1); //erase the previous message
//  }
  alt_up_character_lcd_string(char_lcd_dev, message);
}

/** The main function creates the tasks and starts multi-tasking
 *
 *
 */

int main(void){
  KeyboardQueue = OSQCreate(&KeyboardMessages[0], 20);                // Create message queue
  PingFrontQueue = OSQCreate(&PingFrontMessages[0], 20);                // Create message queue
  //PingRightQueue = OSQCreate(&PingRightMessages[0], 20);                // Create message queue

  sem_RS232 = OSSemCreate(1);                        // Sem for keyboard

  OSTaskCreateExt(taskKeyboard,         NULL,   (void *)&taskKeyboard_stk[TASK_STACKSIZE-1],            taskKeyboard_PRIORITY,          taskKeyboard_PRIORITY,          taskKeyboard_stk,       TASK_STACKSIZE, NULL,   0);
  OSTaskCreateExt(task_Send_Receive_Data,       NULL,   (void *)&task_Send_Receive_Data_stk[TASK_STACKSIZE-1],          task_Send_Receive_Data_PRIORITY,        task_Send_Receive_Data_PRIORITY,        task_Send_Receive_Data_stk,     TASK_STACKSIZE, NULL,   0);
  OSTaskCreateExt(controlPingOutput,       NULL,   (void *)&controlPingOutput_stk[TASK_STACKSIZE-1],          controlPingOutput_PRIORITY,        controlPingOutput_PRIORITY,        controlPingOutput_stk,     TASK_STACKSIZE, NULL,   0);

  OSStart();
  return 0; // this line will never reached
}
