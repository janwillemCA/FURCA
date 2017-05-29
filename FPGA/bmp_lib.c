/**
 * @author Henri van de Munt
 */

#include <stdio.h>
#include <system.h>
#include "bmp_lib.h"
#include <Altera_UP_SD_Card_Avalon_Interface.h>

short int sd_fileh;

void
print_pixel(int x1, int y1, short pixel_color){
  int offset, row, col;
  volatile short * pixel_buffer = (short *) 0x08000000;   // VGA pixel buffer

  row = y1 + 1;
  col = x1 + 1;
  offset = (row << 9) + col;
  *(pixel_buffer + offset) = pixel_color; // compute halfword address, set pixel

}

/*
 * 'read_word()' - Read a 16-bit unsigned integer.
 */

static unsigned short     			/* O - 16-bit unsigned integer */
read_word(FILE *fp){      			/* I - File to read from */
  unsigned char b0, b1;   			/* Bytes from file */

  b0 = alt_up_sd_card_read(fp);
  b1 = alt_up_sd_card_read(fp);

  return ((b1 << 8) | b0);
}


/*
 * 'read_dword()' - Read a 32-bit unsigned integer.
 */

static unsigned int              	/* O - 32-bit unsigned integer */
read_dword(FILE *fp){            	/* I - File to read from */
  unsigned char b0, b1, b2, b3;   	/* Bytes from file */

  b0 = alt_up_sd_card_read(fp);
  b1 = alt_up_sd_card_read(fp);
  b2 = alt_up_sd_card_read(fp);
  b3 = alt_up_sd_card_read(fp);

  return ((((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}


/*
 * 'read_long()' - Read a 32-bit signed integer.
 */

static int                    		/* O - 32-bit signed integer */
read_long(FILE *fp){         	 	/* I - File to read from */
  unsigned char b0, b1, b2, b3; 	/* Bytes from file */

  b0 = alt_up_sd_card_read(fp);
  b1 = alt_up_sd_card_read(fp);
  b2 = alt_up_sd_card_read(fp);
  b3 = alt_up_sd_card_read(fp);

  return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

/*
 * 'read_pixel()' - Read a 32-bit signed integer.
 */

int                        			/* O - 32-bit signed integer */
read_pixel(FILE *fp){        		/* I - File to read from */
  unsigned char r, g, b;       		/* Bytes from file */

  r = (unsigned char) alt_up_sd_card_read(fp);
  g = (unsigned char) alt_up_sd_card_read(fp);
  b = (unsigned char) alt_up_sd_card_read(fp);

  return (int)((b>>3)<<11) | ((g>>2)<<5) | (r>>3);
}

/*
* 'print_img(char *name, int x, int y)' - Display bmp img on display
*/

void
print_img(char *name, int x, int y){	/* I - File name to read from, x offset, y offset */
	  short att;

	  BITMAPFILEHEADER header;       	/* File header */
	  BITMAPINFOHEADER info;			/* File info */

	  alt_up_sd_card_dev *sd_card_dev = alt_up_sd_card_open_dev(SD_CARD_NAME);
	  if (sd_card_dev != 0) {
	      if (alt_up_sd_card_is_Present()) {
	          if (!alt_up_sd_card_is_FAT16())
	            printf("Card is not FAT16\n");

	          sd_fileh = alt_up_sd_card_fopen(name, false);

	          if (sd_fileh < 0)
	            printf("Error opening file: %i", sd_fileh);
	          else {
	        	  /* SD Accessed Successfully, opening data... */
	              att = alt_up_sd_card_get_attributes(sd_fileh);

	              /* Read the file header and any following bitmap information... */
	              header.bfType      = read_word(sd_fileh);
	              header.bfSize      = read_dword(sd_fileh);
	              header.bfReserved1 = read_word(sd_fileh);
	              header.bfReserved2 = read_word(sd_fileh);
	              header.bfOffBits   = read_dword(sd_fileh);

	              info.biSize          = read_dword(sd_fileh);
	              info.biWidth         = read_long(sd_fileh);
	              info.biHeight        = read_long(sd_fileh);
	              info.biPlanes        = read_word(sd_fileh);
	              info.biBitCount      = read_word(sd_fileh);
	              info.biCompression   = read_dword(sd_fileh);
	              info.biSizeImage     = read_dword(sd_fileh);
	              info.biXPelsPerMeter = read_long(sd_fileh);
	              info.biYPelsPerMeter = read_long(sd_fileh);
	              info.biClrUsed       = read_dword(sd_fileh);
	              info.biClrImportant  = read_dword(sd_fileh);

	              /* Loading img...*/

	              for (int q = info.biHeight; q > 0; q--) {
	                  for (int e = 0; e < info.biWidth; ++e) {
	                	  print_pixel(e + x, q+y, (int)read_pixel(sd_fileh));
	                    }
	                }

	              /* Closing img...*/
	              alt_up_sd_card_fclose(sd_fileh);
	              /* Done! */
	            }
	        }
	    }
}
