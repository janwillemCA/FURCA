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
#include "altera_up_avalon_character_lcd.h"


#define         MAX_BUFFER                      100

/* Definition of Task Stacks */
#define         TASK_STACKSIZE                  2048
OS_STK taskKeyboard_stk                         [TASK_STACKSIZE];
OS_STK task_Send_Receive_Data_stk               [TASK_STACKSIZE];

/* Definition of Task Priorities */
#define         taskKeyboard_PRIORITY           1
#define         task_Send_Receive_Data_PRIORITY 2

/* Variables */
OS_EVENT * Mqueue; // message queue
void * Qmessages[20]; // message pointers pool

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
              err = OSQPost(Mqueue, ascii);
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

  char test[10];
  int distance;

  FILE * fp;
  while (1) {

      OSSemPend(sem_RS232, 0, &err);
      fp = fopen(SERIAL_PORT_NAME, "w+r");
      if (fp == NULL) {
          alt_printf("\nFile /RS232 not open for writing....");
        } else {
          msg = OSQPend(Mqueue, 1, &err);
          if (err == OS_NO_ERR) {
              fprintf(fp, "%c", msg);
            }
          if (fscanf(fp, "%s", test) == 1) {
              //  alt_printf("%s\n",test);
              int i;
              for (i = 0; i < 10; ++i) {
                  printf("%c", test[i]);
                }
              printf("\n");

            }
        }
      fclose(fp);
      err = OSSemPost(sem_RS232);
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
  Mqueue = OSQCreate(&Qmessages[0], 20);                // Create message queue
  sem_RS232 = OSSemCreate(1);                        // Sem for keyboard

  OSTaskCreateExt(taskKeyboard,         NULL,   (void *)&taskKeyboard_stk[TASK_STACKSIZE-1],            taskKeyboard_PRIORITY,          taskKeyboard_PRIORITY,          taskKeyboard_stk,       TASK_STACKSIZE, NULL,   0);
  OSTaskCreateExt(task_Send_Receive_Data,       NULL,   (void *)&task_Send_Receive_Data_stk[TASK_STACKSIZE-1],          task_Send_Receive_Data_PRIORITY,        task_Send_Receive_Data_PRIORITY,        task_Send_Receive_Data_stk,     TASK_STACKSIZE, NULL,   0);

  OSStart();
  return 0; // this line will never reached
}
