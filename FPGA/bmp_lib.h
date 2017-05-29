/**
 * @author Henri van de Munt
 */

#ifndef BMP_LIB_H
# define BMP_LIB_H

static unsigned short read_word(FILE *fp);
static unsigned int read_dword(FILE *fp);
static int read_long(FILE *fp);
int read_pixel(FILE *fp);
void print_img(char *name, int x, int y);

#endif

typedef struct                       /**** BMP file header structure ****/
{
  unsigned short bfType;             /* Magic number for file */
  unsigned int bfSize;               /* Size of file */
  unsigned short bfReserved1;        /* Reserved */
  unsigned short bfReserved2;        /* ... */
  unsigned int bfOffBits;            /* Offset to bitmap data */
} BITMAPFILEHEADER;

#  define BF_TYPE 0x4D42             /* "MB" */

typedef struct                       /**** BMP file info structure ****/
{
  unsigned int biSize;               /* Size of info header */
  int biWidth;                       /* Width of image */
  int biHeight;                      /* Height of image */
  unsigned short biPlanes;           /* Number of color planes */
  unsigned short biBitCount;         /* Number of bits per pixel */
  unsigned int biCompression;        /* Type of compression to use */
  unsigned int biSizeImage;          /* Size of image data */
  int biXPelsPerMeter;               /* X pixels per meter */
  int biYPelsPerMeter;               /* Y pixels per meter */
  unsigned int biClrUsed;            /* Number of colors used */
  unsigned int biClrImportant;       /* Number of important colors */
} BITMAPINFOHEADER;
