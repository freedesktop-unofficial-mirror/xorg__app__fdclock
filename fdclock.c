/*
 * $Id$
 *
 * Copyright © 2003 Keith Packard
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <cairo.h>
#include <math.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <sys/time.h>
#include <time.h>
#include <sys/poll.h>

void
draw_boundary (cairo_t *cr)
{
    cairo_move_to      (cr,  63.000,  36.000);
    
    cairo_curve_to     (cr,  63.000,  43.000,
			     58.000,  47.000,
			     51.000,  47.000);

    cairo_line_to      (cr,   13.000,  47.000);
    
    cairo_curve_to     (cr,    6.000,  47.000,
			       1.000,  43.000,
			       1.000,  36.000);
			   
    cairo_line_to      (cr,    1.000,  12.000);
    
    cairo_curve_to     (cr,    1.000,  5.000,
			       6.000,  1.000,
			      13.000,  1.000);
    
    cairo_line_to      (cr,   51.000,  1.000);
    
    cairo_curve_to     (cr,   58.000,  1.000,
			      63.000,  5.000,
			      63.000,  12.000);
    cairo_close_path (cr);
}

void
draw_outline (cairo_t *cr)
{
    cairo_set_rgb_color (cr, 0.73, 0.73, 0.73);
    cairo_set_line_width (cr, 2);
    draw_boundary (cr);
    cairo_stroke (cr);
}

void
draw_background (cairo_t *cr)
{
    cairo_save (cr);
    cairo_set_rgb_color (cr, 0.231, 0.502, 0.682);
    cairo_translate (cr, 3.5, 3.5);
    cairo_scale (cr, 0.887, 0.848);
    draw_boundary (cr);
    cairo_fill (cr);
    cairo_restore (cr);
}

void
draw_window (cairo_t *cr)
{
    cairo_move_to (cr,  -6.00, -7.125);
    
    cairo_line_to (cr,   6.00, -7.125);
    
    cairo_curve_to (cr,  8.00, -7.125,
			 9.00, -6.125,
			 9.00, -4.125);
    
    cairo_line_to (cr,   9.00,  4.125);

    cairo_curve_to (cr,  9.00,  6.125,
			 8.00,  7.125,
			 6.00,  7.125);

    cairo_line_to (cr,  -6.00,  7.125);

    cairo_curve_to (cr, -8.00,  7.125,
			-9.00,  6.125,
			-9.00,  4.125);

    cairo_line_to (cr,	-9.00, -4.125);

    cairo_curve_to (cr,	-9.00, -6.125,
			-8.00, -7.125,
			-6.00, -7.125);
    cairo_close_path (cr);
}

void
draw_window_at (cairo_t *cr, double x, double y, double scale)
{
    cairo_save (cr);
    {
	cairo_translate (cr, x, y);
	cairo_scale (cr, scale, scale);
	draw_window (cr);
	cairo_save (cr);
	{
	    cairo_set_rgb_color (cr, 1, 1, 1);
	    cairo_fill (cr);
	}
	cairo_restore (cr);
	cairo_set_rgb_color (cr, 0.231, 0.502, 0.682);
	cairo_scale (cr, 1/scale, 1/scale);
	cairo_stroke (cr);
    }
    cairo_restore (cr);
}

void
draw_windows (cairo_t *cr)
{
    cairo_save (cr);
    {
	cairo_move_to (cr, 18.00, 16.125);
	cairo_line_to (cr, 48.25, 20.375);
	cairo_line_to (cr, 30.25, 35.825);
	cairo_close_path (cr);
	cairo_set_rgb_color (cr, 1, 1, 1);
	cairo_set_alpha (cr, 0.5);
	cairo_stroke (cr);
    }
    cairo_restore (cr);
    draw_window_at (cr, 18.00, 16.125, 1);
    draw_window_at (cr, 48.25, 20.375, 0.8);
    draw_window_at (cr, 30.25, 35.825, 0.5);
}

#define SCALE	10
#define ROT_X_FACTOR	1.086
#define ROT_Y_FACTOR	1.213
#define NWIDTH	(64 * ROT_X_FACTOR)
#define NHEIGHT	(48 * ROT_Y_FACTOR)
#define WIDTH	(NWIDTH * SCALE)
#define HEIGHT	(NHEIGHT * SCALE)
#define CLOCK_WIDTH	WIDTH
#define CLOCK_HEIGHT	WIDTH

void
draw_logo (cairo_t *cr, double width, double height)
{
    double  x_scale, y_scale, scale, x_off, y_off;
    cairo_save (cr);
    x_scale = width / NWIDTH;
    y_scale = height / NHEIGHT;
    scale = x_scale < y_scale ? x_scale : y_scale;
    x_off = (width - (scale * NWIDTH)) / 2;
    y_off = (height - (scale * NHEIGHT)) / 2;
    cairo_translate (cr, x_off, y_off);
    cairo_scale (cr, scale, scale);
    
    cairo_translate (cr, -2.5, 14.75);
    cairo_rotate (cr, -0.274990703529840);
    
    draw_outline (cr);
    draw_background (cr);
    draw_windows (cr);
    cairo_restore (cr);
}

void
draw_fancy_tick (cairo_t *cr, double radius)
{
    cairo_save (cr);
    cairo_arc (cr, 0, 0, radius, 0, 2 * M_PI);
    cairo_set_rgb_color (cr, 0.231, 0.502, 0.682);
    cairo_fill (cr);
    cairo_set_rgb_color (cr, 0.73, 0.73, 0.73);
    cairo_set_line_width (cr, radius * 2 / 3);
    cairo_arc (cr, 0, 0, radius * 2, 0, 2 * M_PI);
    cairo_stroke (cr);
    cairo_restore (cr);
}

void
draw_plain_tick (cairo_t *cr, double radius)
{
    cairo_save (cr);
    cairo_arc (cr, 0, 0, radius, 0, 2 * M_PI);
    cairo_set_rgb_color (cr, 0.231, 0.502, 0.682);
    cairo_fill (cr);
    cairo_restore (cr);
}

#define PI_6	(M_PI/6.0)

void
draw_hand_helper (cairo_t *cr, double width, double length)
{
    double  r = width / 2;
    cairo_move_to (cr,	length,		    -r);
    cairo_arc (cr,	length,		    0,		r, -M_PI_2, M_PI_2);
    cairo_line_to (cr,	width * M_SQRT2,    r);
    cairo_arc (cr,	0,		    0,		r*2, PI_6, M_PI - PI_6);
    cairo_line_to (cr,	-length / 10,	    r);
    cairo_arc (cr,	-length / 10,	    0,		r, M_PI_2, -M_PI_2);
    cairo_line_to (cr,	-width * M_SQRT2,   -r);
    cairo_arc (cr,	0,		    0,		r*2, M_PI + PI_6, -PI_6);
    cairo_close_path (cr);
}

void
draw_hand (cairo_t *cr, double angle, double width, double length, double alt)
{
    cairo_save (cr);
    {
	cairo_translate (cr, alt, alt);
	cairo_rotate (cr, angle);
	draw_hand_helper (cr, width, length);
	cairo_set_rgb_color (cr, 0, 0, 0);
	cairo_set_alpha (cr, 0.5);
	cairo_fill (cr);
    }
    cairo_restore (cr);
    cairo_save (cr);
    {
	cairo_rotate (cr, angle);
	cairo_set_rgb_color (cr, 0, 0, 0);
	draw_hand_helper (cr, width, length);
	cairo_fill (cr);
    }
    cairo_restore (cr);
}

void
draw_time (cairo_t *cr, double width, double height, struct timeval *tv)
{
    double  hour_angle, minute_angle, second_angle;
    struct tm	tm_ret;
    struct tm	*tm;

    tm = localtime_r (&tv->tv_sec, &tm_ret);
    
    second_angle = (tm->tm_sec + tv->tv_usec / 1000000.0) * 6.0;
    minute_angle = tm->tm_min * 6.0 + second_angle / 60.0;
    hour_angle = tm->tm_hour * 30.0 + minute_angle / 60.0;

    cairo_save (cr);
    {
	cairo_scale (cr, width, height);
	cairo_translate (cr, 0.5, 0.5);
	draw_hand (cr, hour_angle * M_PI / 180.0 - M_PI_2, 0.02, 0.25, 0.005);
	draw_hand (cr, minute_angle * M_PI / 180.0 - M_PI_2, 0.01, 0.4, 0.010);
	draw_hand (cr, second_angle * M_PI / 180.0 - M_PI_2, 0.005, 0.3, 0.015);
    }
    cairo_restore (cr);
}

void
draw_now (cairo_t *cr, double width, double height)
{
    struct timeval  tv;
    struct timezone tz;
    gettimeofday (&tv, &tz);

    draw_time (cr, width, height, &tv);
}

void
draw_clock (cairo_t *cr, double width, double height)
{
    double  x_off, y_off;
    double  size;
    int	    minute;

    cairo_save (cr);
    {
	cairo_scale (cr, width, height);
	cairo_save (cr);
	{
	    cairo_translate (cr, .15, .15);
	    cairo_scale (cr, .7, .7);
	    draw_logo (cr, 1, 1);
	}
	cairo_restore (cr);
	cairo_translate (cr, 0.5, 0.5);
	cairo_scale (cr, 0.93, 0.93);
	for (minute = 0; minute < 60; minute++)
	{
	    double  degrees, radians;
	    cairo_save (cr);
	    degrees = minute * 6.0;
	    radians = degrees * M_PI / 180;
	    cairo_rotate (cr, radians);
	    cairo_translate (cr, 0, 0.5);
	    if (minute % 15 == 0)
		draw_fancy_tick (cr, 0.015);
	    else if (minute % 5 == 0)
		draw_fancy_tick (cr, 0.01);
	    else
		draw_plain_tick (cr, 0.01);
	    cairo_restore (cr);
	}
    }
    cairo_restore (cr);
}

void
main_x (int width, int height)
{
    cairo_t *cr = cairo_create ();
    Display *dpy = XOpenDisplay (0);
    int scr = DefaultScreen (dpy);
    Window  w = XCreateSimpleWindow (dpy, RootWindow (dpy, scr),
				     0, 0, WIDTH, HEIGHT, 0, 0,
				     WhitePixel (dpy, scr));
    GC		gc;
    XGCValues	gcv;
    Pixmap  background = None, buffer;
    XEvent  ev;
    Bool paint = False;
    XSelectInput (dpy, w, ExposureMask|StructureNotifyMask);
    XMapWindow (dpy, w);
    
    gcv.graphics_exposures = False;
    gc = XCreateGC (dpy, w, GCGraphicsExposures, &gcv);
    for (;;)
    {
	struct pollfd	a;

	XFlush (dpy);
	a.fd = ConnectionNumber (dpy);
	a.events = POLLIN;
	if (poll (&a, 1, 100) > 0)
	{
	    do {
		XNextEvent (dpy, &ev);
		switch (ev.type) {
		case Expose:
		    paint = True;
		    break;
		case ConfigureNotify:
		    if (width != ev.xconfigure.width ||
			height != ev.xconfigure.height)
		    {
			width = ev.xconfigure.width;
			height = ev.xconfigure.height;
			if (background)
			{
			    XFreePixmap (dpy, background);
			    background = None;
			}
			paint = True;
		    }
		    break;
		}
	    } while (XPending (dpy));
	}
	else
	    paint = True;
	if (paint)
	{
	    if (!background)
	    {
		background = XCreatePixmap (dpy, RootWindow (dpy, 0),
					    width, height,
					    DefaultDepth (dpy, scr));
		cairo_set_target_drawable (cr, dpy, background);
		cairo_set_rgb_color (cr, 1, 1, 1);
		cairo_rectangle (cr, 0, 0, width, height);
		cairo_fill (cr);
		draw_clock (cr, width, height);
	    }
	    buffer = XCreatePixmap (dpy, RootWindow (dpy, 0),
				    width, height,
				    DefaultDepth (dpy, scr));
	    cairo_set_target_drawable (cr, dpy, buffer);
	    XCopyArea (dpy, background, buffer, gc, 
		       0, 0, width, height, 0, 0);
	    draw_now (cr, width, height);
	    XCopyArea (dpy, buffer, w, gc, 0, 0, width, height, 0, 0);
	    XFreePixmap (dpy, buffer);
	    paint = False;
	}
    }
}


int
main_png (int width, int height)
{
    int	    stride = width * 4;
    cairo_t *cr = cairo_create ();;
    char    *image = calloc (stride * height, 1);
    char    out[1024];
    
    cairo_set_target_image (cr, image, CAIRO_FORMAT_ARGB32,
			    width, height, stride);
    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, width, 0);
    cairo_line_to (cr, width, height);
    cairo_line_to (cr, 0, height);
    cairo_close_path (cr);
    cairo_set_rgb_color (cr, 1, 1, 1);
    cairo_fill (cr);
    draw_clock (cr, width, height);
    sprintf (out, "freedesktop-clock-%d.png", width);
    write_png_argb32 (image, out, width, height, stride);

    cairo_destroy (cr);
    return 0;
}

int
main (int argc, char **argv)
{
    main_x (CLOCK_WIDTH, CLOCK_HEIGHT);
/*    main_png (2400, 2400); */
    return 0;
}
