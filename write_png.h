#ifndef WRITE_PNG_H
#define WRITE_PNG_H

void
write_png_argb32 (char *buffer, char* filename,
		  int width, int height, int stride);

char *
read_png_argb32 (char *filename,
		 int *width, int *height, int *stride);

#endif
