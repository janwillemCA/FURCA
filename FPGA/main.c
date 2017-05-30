 //* @author Steven van der Vlist

#include <stdio.h>
#include <string.h>
#include "includes.h"
#include <math.h>
#include "bmp_lib.h"

#include "os/alt_sem.h"

#include <unistd.h>
#include <fcntl.h>

#include <altera_up_avalon_ps2.h>
#include <altera_up_ps2_keyboard.h>
#include "altera_up_avalon_character_lcd.h"
#include <stdlib.h>

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
#define TASK_STACKSIZE  2048
OS_STK taskKeyboard_stk                         [TASK_STACKSIZE];
OS_STK task_Send_Receive_Data_stk               [TASK_STACKSIZE];
OS_STK controlPingOutput_stk               		[TASK_STACKSIZE];
OS_STK controlSpeedAnimation_stk               	[TASK_STACKSIZE];

/* Definition of Task Priorities */
#define         taskKeyboard_PRIORITY           1
#define         task_Send_Receive_Data_PRIORITY 2
#define         controlPingOutput_PRIORITY 3
#define 		controlSpeedAnimation_PRIORITY 4

/* Variables */
OS_EVENT * KeyboardQueue; // message queue
OS_EVENT * PingLeftQueue; // Queue for processing control data
OS_EVENT * PingRightQueue; // Queue for processing control data
OS_EVENT * PwmQueue;

void * KeyboardMessages[20]; // message pointers pool
void * PingLeftMessages[50];
void * PingRightMessages[50];
void * PwmMessages[20];

ALT_SEM(sem_RS232);
float *speedMeterArray(int x, int y, int radius);



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
              VGA_text (60, 4, buffer);
              err = OSQPost(KeyboardQueue, ascii);

              if(ascii == 'Z'){
                VGA_text (60, 3, "Driving Autonomous");
              }

              if(ascii == 'X'){
                VGA_text (60, 3, "Driving Manual    ");
              }

              if(ascii == 'L'){
                            VGA_text (10, 3, "Lights ON ");
                           }
              if(ascii == 'O'){
                            VGA_text (10, 3, "Lights OFF");
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
                  OSQPost(PwmQueue, speed);
                }
            }
          OSTimeDlyHMSM(0, 0, 0, 15);
        }
      fclose(fp);
      err = ALT_SEM_POST(sem_RS232);
    }
}

void controlSpeedAnimation(void *pdata)
{
	INT8U err;
	int speed = 0;
	float previousSpeed = 0;
	//direction: 1 is up, 2 is down
	int direction = 0;
	char buffer[MAX_BUFFER];
	int *outerCircle = speedMeterArray(241,122,43);
	//amount of values stored in the array/2
	int size = 350;
	//index: at what place in the array are we? max size is at 0pwm. index 0 is at 255 pwm
	int index = size;
	float steps = 255/(size/5);




while(1){

		int s = (int)OSQPend(PwmQueue, 1, &err);
		if(s != NULL) {
			speed=s;
		}
		printf("test:%d\n",speed);
		//printf("previousSpeed:%f\n",previousSpeed);
		sprintf(buffer, "%d  ", speed);
		VGA_text (64, 43, buffer);
		if(speed > previousSpeed) {
			//difference = speed - previousSpeed;
			direction = 1;

		}else if(speed < previousSpeed){
			//difference =  previousSpeed - speed;
			direction = 2;
		}else{
			direction = 0;
		}

		if(direction == 1) {
			index-=4;
			previousSpeed+=steps;
		}else if(direction == 2) {
			index+=4;
			previousSpeed-=steps;
		}
		draw_line((int)outerCircle[index],(int)outerCircle[index+1],242,125,0xffffff);
		if(index <= 345 && index >= 4) {
			if(direction == 1) {
				draw_line((int)outerCircle[index+4],(int)outerCircle[index+5],242,125,0x00);
			}else if(direction == 2) {
				draw_line((int)outerCircle[index-4],(int)outerCircle[index-3],242,125,0x00);
			}
		}
		OSTimeDlyHMSM(0, 0, 0, 60);
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



float *speedMeterArray(int x0, int y0, int radius)
{
	int *array = malloc(sizeof(float)*400);
    int x = radius;
    int y = 0;
    int err = 0;
    int index = 0;
    int a2[200],a3[200],a4[200],a5[200],a6[200],a7[200];

    while (x >= y)
    {
    	a2[index] = x0+x;
    	a2[index+1] = y0+y;

    	a7[index] = x0-x;
    	a7[index+1] = y0+y;

    	a6[index] = x0-x;
    	a6[index+1] = y0-y;

    	a5[index] = x0-y;
    	a5[index+1] = y0-x;

    	a4[index] = x0+y;
    	a4[index+1] = y0-x;

    	a3[index] = x0+x;
    	a3[index+1] = y0-y;

        y += 1;
        if (err <= 0)
        {
            err += 2*y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2*x + 1;
        }

        index +=2;
    }

            //the below code is used to put the coörds in the correct order in the final array so you can loop through them.
            int finalIndex = 0;
            //size is the same for all 8 arrays
            int size = 0;
            while(a2[size] != 0) {
            	size++;
            }
            size=size-2;
            printf("size:%d\n",size);
            for(int i=0; i <= size; i++) {
            	array[finalIndex] = a2[size-i];
            	array[finalIndex+size] = a3[i];
            	array[finalIndex+size*2] = a4[size-i];
            	array[finalIndex+size*3] = a5[i];
            	array[finalIndex+size*4] = a6[size-i];
            	array[finalIndex+size*5] = a7[i];
            	printf("%d\n",a2[size-i]);
            	finalIndex++;
            }
	return array;
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



void draw_line(int x, int y, int endX, int endY, short pixel_color) {
  float dx;
  float dy;
  int xDirection; //1 is right 0 is left 2 is horizontal
  int yDirection; //1 is up 0 is down 2 is vertical
  if(x < endX) {
    dx = endX - x;
    xDirection = 1;
  }else if(x > endX) {
    dx = x - endX;
    xDirection = 0;
  }else{
    dx = 0;
    yDirection = 2;
  }

  if(y < endY) {
    dy = endY - y;
    yDirection = 0;
  }else if(y > endY) {
    dy = y - endY;
    yDirection = 1;
  }else{
    dy = 0;
    yDirection = 2;
  }

  //printf("dx,dy:%f,%f\n",dx,dy);

  float up;
  float down;
  float curX = x;
  float curY = y;

  //up to the right
  if(xDirection == 1 && yDirection == 1) {
	  if(dy > dx) {
	  	up = dx/dy;
	  	for(int i = 0; i<=dy;i++) {
	  		curX = curX + up;
	  		curY = curY - 1;
	  		VGA_box (curX,curY,curX,curY, pixel_color);
	  		  }
	  	}else {
	  	up = dy/dx;
	  	for(int i = 0; i<=dx;i++) {
	  		curX = curX + 1;
	  		curY = curY - up;
	  		VGA_box (curX,curY,curX,curY, pixel_color);
	  	}
	 }
  }else if(xDirection == 1 && yDirection == 0) {
	  if(dy > dx) {
	  		  up = dx/dy;
	  		  for(int i = 0; i<=dy;i++) {
	  			  curX = curX + up;
	  			  curY = curY + 1;
	  			  VGA_box (curX,curY,curX,curY, pixel_color);
	  		  }
	  	  }else {
	  		  up = dy/dx;
	  		  for(int i = 0; i<=dx;i++) {
	  			  curX = curX + 1;
	  			  curY = curY + up;
	  			  VGA_box (curX,curY,curX,curY, pixel_color);
	  		  }
	  	  }
  }else if(xDirection == 0 && yDirection == 1) {
	  if(dy > dx) {
		  up = dx/dy;
		  for(int i = 0; i<=dy;i++) {
			  curX = curX - up;
			  curY = curY - 1;
			  VGA_box (curX,curY,curX,curY, pixel_color);
		  }
	  }else {
		  up = dy/dx;
		  for(int i = 0; i<=dx;i++) {
			  curX = curX - 1;
			  curY = curY - up;
			  VGA_box (curX,curY,curX,curY, pixel_color);
		  }
	  }

  }else if(xDirection == 0 && yDirection == 0)
  {//down to the left
	  if(dy > dx) {
	  		  up = dx/dy;
	  		  for(int i = 0; i<=dy;i++) {
	  			  curX = curX - up;
	  			  curY = curY + 1;
	  			  VGA_box (curX,curY,curX,curY, pixel_color);
	  		  }
	  	  }else {
	  		  up = dy/dx;
	  		  for(int i = 0; i<=dx;i++) {
	  			  curX = curX - 1;
	  			  curY = curY + up;
	  			  VGA_box (curX,curY,curX,curY, pixel_color);
	  		  }
	  	  }
  }


}

int main(void)
{

  KeyboardQueue = OSQCreate(&KeyboardMessages[0], 20);                // Create message queue
  PingLeftQueue = OSQCreate(&PingLeftMessages[0], 20);                // Create message queue
  PingRightQueue = OSQCreate(&PingRightMessages[0], 20);                // Create message queue
  PwmQueue = OSQCreate(&PwmMessages[0], 20);

  ALT_SEM_CREATE(&sem_RS232, 1);

  OSTaskCreateExt(taskKeyboard,NULL,(void *)&taskKeyboard_stk[TASK_STACKSIZE-1],taskKeyboard_PRIORITY,taskKeyboard_PRIORITY,taskKeyboard_stk,TASK_STACKSIZE, NULL,0);
  OSTaskCreateExt(task_Send_Receive_Data,NULL,(void *)&task_Send_Receive_Data_stk[TASK_STACKSIZE-1],task_Send_Receive_Data_PRIORITY,task_Send_Receive_Data_PRIORITY,task_Send_Receive_Data_stk,TASK_STACKSIZE,NULL,0);
  OSTaskCreateExt(controlPingOutput,NULL,(void *)&controlPingOutput_stk[TASK_STACKSIZE-1],controlPingOutput_PRIORITY,controlPingOutput_PRIORITY,controlPingOutput_stk,TASK_STACKSIZE,NULL,0);
  OSTaskCreateExt(controlSpeedAnimation,NULL,(void *)&controlSpeedAnimation_stk[TASK_STACKSIZE-1],controlSpeedAnimation_PRIORITY,controlSpeedAnimation_PRIORITY,controlSpeedAnimation_stk,TASK_STACKSIZE,NULL,0);
  /*
  * VGA Display
  */
  VGA_text (50, 40, "0");
  VGA_text (69, 40, "260");
  VGA_text (58, 15, "130");
  VGA_text (45, 28, "65");
  VGA_text (73, 28, "195");
  VGA_text (56, 43, "Power: ");
  VGA_text (64, 43, "0  ");
  VGA_text (12, 23, "Measured distance: ");
  VGA_text (60, 3, "Driving Autonomous");
  VGA_text (10, 3, "Lights ON ");

  VGA_box (0, 0, 319, 239, 0x00);           // clear the screen
  print_img("bg1.bmp",0,0);
  //OSQPost(PwmQueue,120);


  OSStart();
  
  return 0; // this line will never reached
}
