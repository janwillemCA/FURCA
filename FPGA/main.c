 * @author Steven van der Vlist

#include <stdio.h>
#include <string.h>
#include "includes.h"
#include <math.h>

#include "os/alt_sem.h"

#include <unistd.h>
#include <fcntl.h>

#include <altera_up_avalon_ps2.h>
#include <altera_up_ps2_keyboard.h>
#include "altera_up_avalon_character_lcd.h"

/* Definition of plotting a cirkle */
#define WIDTH     60
#define HEIGHT      20
#define X         WIDTH/2
#define Y         HEIGHT/2
#define XMAX      WIDTH-X-1
#define XMIN -(WIDTH-X)
#define YMAX      HEIGHT-Y
#define YMIN -(HEIGHT-Y)+1
#define MAX_BUFFER      100

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
OS_EVENT * PingLeftQueue; // Queue for processing control data
OS_EVENT * PingRightQueue; // Queue for processing control data

void * KeyboardMessages[20]; // message pointers pool
void * PingLeftMessages[50];
void * PingRightMessages[50];

ALT_SEM(sem_RS232);


const char logo[36][67]= //18 37
{
    {'c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','c','c','c','c','c','c',},
    {'c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c','c',},
    {'c','c','c','c','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','c','c','c','c',},
    {'c','c','c','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','b','b','b','c','c','c',},
    {'c','c','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','b','b','b','c','c',},
    {'c','b','b','b','c','c','c','c','c','c','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','b','b','b','c',},
    {'c','b','b','b','c','c','c','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','c','c','c','c','c','b','b','b','c',},
    {'c','b','b','b','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','c','c','c','c','c','c','c','c','c','c','b','b','c',},
    {'c','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','b','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','b','b','b',},
    {'b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','c','c','c','c','c','c','c','c','c','b','c','c','b','c','c','c','c','c','c','c','c','b','b','b',},
    {'b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','b','b','b',},
    {'c','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','b','b','b',},
    {'c','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','c','c','b','b','c',},
    {'c','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','b','b','b','c','c','c','c','c','c','b','b','b','c',},
    {'c','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','b','b','b','c',},
    {'c','c','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','b','c','c','c','c','c','c','c','c','b','c','c','c','c','c','c','c','c','c','c','c','c','b','c','c','c','c','c','c','b','b','b','c','c',},
    {'c','c','c','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','b','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','c','c','c',},
    {'c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c',},
    {'c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','c','c','c','c','c','b','b','b','b','c','c','c','c',},
    {'c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','b','b','b','b','b','b','c','c','c','c','c','c','c','c','b','b','b','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','b','b','b','b','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','b','c','c','c','c','c','c','b','b','b','b','b','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c',},
    {'c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','b','b','b','b','b','b','b','b','b','b','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c','c',},

};

/**
 * task to receive data from the keyboard
 *
 * @Mqueue puts data from keyboard in message queue
 */
void taskKeyboard(void* pdata)
{
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
              VGA_text (33, 52, buffer);
              err = OSQPost(KeyboardQueue, ascii);

              if(ascii == 'Z'){
                VGA_text (60, 52, "Driving Autonomous");
              }

              if(ascii == 'X'){
                VGA_text (60, 52, "Driving Manual    ");
              }

              if(ascii == 'L'){
                            VGA_text (10, 52, "Light ON ");
                           }
              if(ascii == 'O'){
                            VGA_text (10, 52, "Light OFF");
                           }





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

void task_Send_Receive_Data(void* pdata)
{
  INT8U err;
  char *msg;

  char incomingData[50];

  char buffer[MAX_BUFFER];

  FILE * fp;
  while (1) {
    ALT_SEM_PEND(sem_RS232, 0);
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

              if (dD[0] == 'L') {
                  int distanceLeft = atoi(d);
                  // printf("%d\n", distanceLeft);
                  err = OSQPost(PingLeftQueue, distanceLeft);
                }
              if (dD[0] == 'R') {
                  int distanceRight = atoi(d);
                  //printf("%d\n", distanceRight);
                  err = OSQPost(PingRightQueue, distanceRight);
                }
              if (dD[0] == 'V') {
                  int speed = atoi(d);
                  printf(speed);
                  sprintf(buffer, "%d  ", speed);
                  VGA_text (64, 43, buffer);
                }
            }
          OSTimeDlyHMSM(0, 0, 0, 15);
        }
      fclose(fp);
      err = ALT_SEM_POST(sem_RS232);
    }
}

void controlPingOutput(void *pdata)
{
  INT8U err;
  int left;
  int right;
  char buffer[MAX_BUFFER];
  while (1) {
      // check right
      left = OSQPend(PingLeftQueue, 0, &err);
      printf("left %d\n", left);
      sprintf(buffer, "Left: %d  cm", left);
      VGA_text (8, 30, buffer);
      if (left < 50 && left > 0)
        err = OSQPost(KeyboardQueue, 'H'); // send hold

      // check right
      right = OSQPend(PingRightQueue, 0, &err);
      printf("right %d\n", right);
      sprintf(buffer, "Right: %d  cm", right);

       VGA_text (21, 30, buffer);
      if (right < 50 && right > 0)
        err = OSQPost(KeyboardQueue, 'H'); // send hold
    }
}

/**
 * Used to display text to the lcd
 *
 * @param data to display
 */

void displayTextLCD(char * message)
{
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


/****************************************************************************************
 * Subroutine to send a string of text to the VGA monitor
****************************************************************************************/
void VGA_text(int x, int y, char * text_ptr)
{
  int offset;
    volatile char * character_buffer = (char *) 0x09000000; // VGA character buffer

  /* assume that the text string fits on one line */
  offset = (y << 7) + x;
  while ( *(text_ptr) ){
    *(character_buffer + offset) = *(text_ptr); // write to the character buffer
    ++text_ptr;
    ++offset;
  }
}

/****************************************************************************************
 * Draw a filled rectangle on the VGA monitor
****************************************************************************************/
void VGA_box(int x1, int y1, int x2, int y2, short pixel_color)
{
  int offset, row, col;
    volatile short * pixel_buffer = (short *) 0x08000000; // VGA pixel buffer

  /* assume that the box coordinates are valid */
  for (row = y1; row <= y2; row++){
    col = x1;
    while (col <= x2)
    {
      offset = (row << 9) + col;
      *(pixel_buffer + offset) = pixel_color; // compute halfword address, set pixel
      ++col;
    }
  }
}


int circle(int x, int y, int radius)
{
  //float xpos, ypos, radsqr, xsqr;
    int xsqrt, rsqrt,ysum, ypositive;
    float xpos,xleft;
    xleft = x;
    for(xpos = x; xpos <= radius+x; xpos+=0.1) {
      xleft-=0.1;
      xsqrt = pow(xpos-x,2);
      rsqrt = pow(radius,2);

      ysum = sqrt(abs(rsqrt - xsqrt));
      ypositive = radius - ysum;

      VGA_box (xpos, ysum+y,xpos + 2,ysum+y + 2, 0x333333);
      VGA_box (xleft, ysum+y,xleft + 2,ysum+y + 2, 0x333333);
      VGA_box (xpos, ypositive+y-radius,xpos + 2,ypositive+y-radius+ 2, 0x333333);
      VGA_box (xleft, ypositive+y-radius,xleft + 2,ypositive+y-radius+ 2, 0x333333);
    }
    return(1);
}

void draw_object(int xpos, int ypos, const char sprite[36][67])
{
  for (int spritex = 0; spritex < 67; spritex++)
    {
      for (int spritey = 0; spritey < 36; spritey++)
        {
          switch (sprite[spritey][spritex])
            {
            case 'c':
              //nothing
              break;
            case 'b':
              VGA_box (xpos + spritex, ypos + spritey, xpos + spritex ,ypos + spritey, 0xffffff);
              break;
            }
        }
    }
}

void draw_line(int x, int y, int endX, int endY) {
  int dx;
  int dy;
  if(x < endX) {
    dx = endX - x;
  }else if(x > endX) {
    dx = x - endX;
  }else{
    dx = 0;
  }

  if(y < endY) {
    dy = endY - y;
  }else if(y > endY) {
    dy = y - endY;
  }else{
    dy = 0;
  }

  if(x < endX) {
    for (int i = x; i <= endX; i++) {
      if(y < endY) {
        int yDef = y + dy * (i - x) / dx;
        VGA_box(i, yDef,i,yDef,0xffffff);
      }else{
        printf("test");
        int yDef = y - dy * (i - x) / dx;
        VGA_box(i, yDef,i,yDef,0xffffff);
      }
    }
  }else if(x > endX) {
    for (int i = x; i >= endX; i--) {
      int yDef = y + dy * (i + x) / dx;
      VGA_box(i, yDef,i,yDef,0xffffff);
      printf("%d\n",i);
    }
  }
}

int main(void)
{

  KeyboardQueue = OSQCreate(&KeyboardMessages[0], 20);                // Create message queue

  PingLeftQueue = OSQCreate(&PingLeftMessages[0], 20);                // Create message queue
  PingRightQueue = OSQCreate(&PingRightMessages[0], 20);                // Create message queue

  ALT_SEM_CREATE(&sem_RS232, 1);

  OSTaskCreateExt(taskKeyboard,         NULL,   (void *)&taskKeyboard_stk[TASK_STACKSIZE-1],            taskKeyboard_PRIORITY,          taskKeyboard_PRIORITY,          taskKeyboard_stk,       TASK_STACKSIZE, NULL,   0);
  OSTaskCreateExt(task_Send_Receive_Data,       NULL,   (void *)&task_Send_Receive_Data_stk[TASK_STACKSIZE-1],          task_Send_Receive_Data_PRIORITY,        task_Send_Receive_Data_PRIORITY,        task_Send_Receive_Data_stk,     TASK_STACKSIZE, NULL,   0);
  OSTaskCreateExt(controlPingOutput,       NULL,   (void *)&controlPingOutput_stk[TASK_STACKSIZE-1],          controlPingOutput_PRIORITY,        controlPingOutput_PRIORITY,        controlPingOutput_stk,     TASK_STACKSIZE, NULL,   0);

  /*
  * VGA Display
  */
  VGA_text (50, 40, "0");
  VGA_text (68, 40, "260");
  VGA_text (58, 15, "130");
  VGA_text (45, 28, "65");
  VGA_text (72, 28, "195");
  VGA_text (56, 43, "Power: ");
  VGA_text (64, 43, "0  ");
  VGA_text (12, 23, "Measured distance: ");
  VGA_text (60, 52, "Driving Autonomous");
  VGA_text (10, 52, "Light ON ");

  VGA_box (0, 0, 319, 239, 0x00);           // clear the screen
  circle(80, 120 , 70);
  circle(240,120 , 70);

  draw_object(125, 15, logo);
  //draw_line(53,53,100,100);
  //draw_line(20,10,50,50);
  draw_line(50,50,0,60);
  OSStart();
  
  return 0; // this line will never reached
}
