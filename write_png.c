#include <stdio.h>
#include <png.h>
#include <stdlib.h>

#include "write_png.h"

static void
unpremultiply_data (png_structp png, png_row_infop row_info, png_bytep data)
{
    int i;

    for (i = 0; i < row_info->rowbytes; i += 4) {
        unsigned char *b = &data[i];
        unsigned char alpha = b[3];
        unsigned long pixel;
        unsigned long *p;
        if (alpha == 0)
            pixel = 0;
        else
            pixel = ((((b[0] * 255) / alpha) << 0) |
                     (((b[1] * 255) / alpha) << 8) |
                     (((b[2] * 255) / alpha) << 16) |
                     (alpha << 24));
        p = (unsigned long *) b;
        *p = pixel;
    }
}

static void
premultiply_data (png_structp png, png_row_infop row_info, png_bytep data)
{
    int i;

    for (i = 0; i < row_info->rowbytes; i += 4) {
	unsigned char  *base = &data[i];
	unsigned char  blue = base[0];
	unsigned char  green = base[1];
	unsigned char  red = base[2];
	unsigned char  alpha = base[3];
	unsigned long	p;

	red = (unsigned) red * (unsigned) alpha / 255;
	green = (unsigned) green * (unsigned) alpha / 255;
	blue = (unsigned) blue * (unsigned) alpha / 255;
	p = (alpha << 24) | (red << 16) | (green << 8) | (blue << 0);
	memcpy (base, &p, sizeof (unsigned long));
    }
}

void
write_png_argb32 (char *buffer, char* filename,
		  int width, int height, int stride)
{
    FILE* f;
    png_struct* png;
    png_info* info;
    png_byte** rows;
    int       i;
    png_color_16 white;
    
    f = fopen (filename, "w");
    rows = malloc (height*sizeof (png_byte*));

    for (i=0;i<height;i++) {
	rows[i]=buffer+i*stride;
    }

    png = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info = png_create_info_struct (png);

    png_init_io (png, f);
    png_set_IHDR (png, info,
		  width, height, 8,
		  PNG_COLOR_TYPE_RGB_ALPHA, 
		  PNG_INTERLACE_NONE,
		  PNG_COMPRESSION_TYPE_DEFAULT,
		  PNG_FILTER_TYPE_DEFAULT);

    white.red=0xff;
    white.blue=0xff;
    white.green=0xff;
    png_set_bKGD (png, info, &white);

    png_set_write_user_transform_fn (png, unpremultiply_data);
    png_set_bgr (png);

    png_write_info (png, info);
    png_write_image (png, rows);
    png_write_end (png, info);

    png_destroy_write_struct (&png, &info);

    free (rows);
    fclose (f);
}

char *
read_png_argb32 (char *filename,
		 int *widthp, int *heightp, int *stridep)
{
    FILE    *f;
    char    *buffer;
    png_structp png;
    png_infop info;
    png_bytepp rows;
    int i;
    png_uint_32 width, height;
    png_uint_32	stride;
    int depth, color, interlace;
    
    png = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png == NULL)
	return NULL;
    info = png_create_info_struct (png);
    if (info == NULL)
    {
	png_destroy_read_struct (&png, NULL, NULL);
	return NULL;
    }
    if (setjmp (png->jmpbuf))
    {
	png_destroy_read_struct (&png, &info, NULL);
	return NULL;
    }
    f = fopen (filename, "rb");
    if (f == NULL)
    {
	png_destroy_read_struct (&png, &info, NULL);
	return NULL;
    }
    png_init_io (png, f);
    png_read_info (png, info);
    png_get_IHDR (png, info, &width, &height, &depth, &color, &interlace,
		  NULL, NULL);

    if (color == PNG_COLOR_TYPE_PALETTE && depth <= 8)
	png_set_expand (png);

    if (color == PNG_COLOR_TYPE_GRAY && depth < 8)
	png_set_expand (png);

    if (png_get_valid (png, info, PNG_INFO_tRNS))
	png_set_expand (png);

    if (depth == 16)
	png_set_strip_16 (png);

    if (depth < 8)
	png_set_packing (png);

    if (color == PNG_COLOR_TYPE_GRAY || color == PNG_COLOR_TYPE_GRAY_ALPHA)
	png_set_gray_to_rgb (png);

    if (interlace != PNG_INTERLACE_NONE)
	png_set_interlace_handling (png);

    png_set_bgr (png);
    png_set_filler (png, 255, PNG_FILLER_AFTER);

    png_set_read_user_transform_fn (png, premultiply_data);

    png_read_update_info (png, info);

    stride = width * 4;
    buffer = malloc (stride * height);
    
    rows = malloc (sizeof (png_bytep) * height);

    for (i = 0; i < height; i++)
	rows[i] = (png_bytep) (buffer + i * stride);
    
    png_read_image (png, rows);
    png_read_end (png, info);

    free (rows);
    fclose (f);
    png_destroy_read_struct (&png, &info, NULL);

    *widthp = (int) width;
    *heightp = (int) height;
    *stridep = (int) stride;
    
    return buffer;
}

		    
