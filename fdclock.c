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
#include <cairo-xlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include <sys/poll.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "fdface.h"
#include "fdhand.h"
#include "findargb.h"

#define CLOCK_WIDTH	150
#define CLOCK_HEIGHT	150

static void
clear (cairo_t	*cr, double width, double height, double alpha)
{
    cairo_save (cr);
    cairo_set_source_rgba (cr, 1, 1, 1, alpha);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill (cr);
    cairo_restore (cr);
}
    
cairo_surface_t *
make_background (cairo_surface_t *buffer_surface,
		 cairo_content_t content,
		 int width, int height,
		 Bool opaque)
{
    cairo_surface_t	*back_surface = cairo_surface_create_similar (buffer_surface,
								      content,
								      width, height);
    cairo_surface_t	*temp_surface = 0;
    cairo_t		*cr;

    if (content == CAIRO_CONTENT_COLOR_ALPHA)
    {
	temp_surface = cairo_surface_create_similar (back_surface,
						     CAIRO_CONTENT_COLOR_ALPHA,
						     width, height);
	/*
	 * draw the background to the temporary surface
	 */
    
	cr = cairo_create (temp_surface);
    }
    else
	cr = cairo_create (back_surface);

    clear (cr, width, height, temp_surface ? 0 : 1);

    if (temp_surface)
    {
	cairo_save (cr);
	{
	    cairo_set_source_rgba (cr, 1, 1, 1, opaque ? 1 : 0.5);
	    cairo_scale (cr, width, height);
	    cairo_translate (cr, 0.5, 0.5);
	    cairo_arc (cr, 0, 0, 0.5, 0, 2 * M_PI);
	    cairo_fill (cr);
	}
	cairo_restore (cr);
    }

    fdface_draw (cr, width, height);

    if (temp_surface)
    {
	cairo_destroy (cr);
	cr = cairo_create (back_surface);

	cairo_set_source_surface (cr, temp_surface, 0, 0);
	cairo_paint_with_alpha (cr, opaque ? 1 : 0.8);
	cairo_surface_destroy (temp_surface);
    }
    cairo_destroy (cr);
    return back_surface;
}

static void
main_x (char *dpy_name, char *geom, Bool seconds, 
	Bool translucent, Bool square, Bool opaque)
{
    Display		    *dpy;
    int			    scr;
    int			    x = 0, y = 0;
    unsigned int	    u_width = CLOCK_WIDTH, u_height = CLOCK_HEIGHT;
    int			    width, height;
    Window		    root;
    Visual		    *visual;
    XWMHints		    *wmhints;
    XSizeHints		    *normalhints;
    XClassHint		    *classhint;
    char		    *name;
    XSetWindowAttributes    wattr;
    unsigned long	    wmask;
    int			    depth;
    Colormap		    cmap;
    Window		    w;
    cairo_surface_t	    *background = NULL;
    cairo_surface_t	    *window;
    cairo_content_t	    content;
    XEvent		    ev;
    Bool		    paint = False;
    int			    timeout;
    
    dpy = XOpenDisplay (dpy_name);
    if (!dpy)
    {
	fprintf (stderr, "Cannot open display\n");
	return;
    }
    
    if (geom)
	XParseGeometry (geom, &x, &y, &u_width, &u_height);

    width = (int) u_width;
    height = (int) u_height;
    scr = DefaultScreen (dpy);
    root = RootWindow (dpy, scr);
    if (translucent)
	visual = find_argb_visual (dpy, scr);
    else
	visual = 0;
    
    wattr.event_mask = (ExposureMask | StructureNotifyMask);
    if (visual)
    {
	depth = 32;
	wattr.background_pixel = 0;
	wattr.border_pixel = 0;
	wattr.colormap = cmap = XCreateColormap (dpy, root, visual, AllocNone);
	content = CAIRO_CONTENT_COLOR_ALPHA;
    }
    else
    {
	visual = DefaultVisual (dpy, scr);
	depth = DefaultDepth (dpy, scr);
	wattr.colormap = cmap = DefaultColormap (dpy, scr);
	wattr.background_pixel = WhitePixel (dpy, scr);
	wattr.border_pixel = BlackPixel (dpy, scr);
	content = CAIRO_CONTENT_COLOR;
    }
    wmask = CWEventMask | CWBackPixel | CWBorderPixel | CWColormap;
    w = XCreateWindow (dpy, root, x, y, width, height, 0,
		       depth, InputOutput, visual, wmask, &wattr);
    
    window = cairo_xlib_surface_create (dpy, w, visual, width, height);
	
    name = "fdclock";
    
    normalhints = XAllocSizeHints ();
    normalhints->flags = 0;
    normalhints->x = x;
    normalhints->y = y;
    normalhints->width = width;
    normalhints->height = height;

    classhint = XAllocClassHint ();
    classhint->res_name = "fdclock";
    classhint->res_class = "Fdclock";
    
    wmhints = XAllocWMHints ();
    wmhints->flags = InputHint;
    wmhints->input = True;
    
    Xutf8SetWMProperties (dpy, w, name, name, 0, 0, 
			  normalhints, wmhints, classhint);
    XFree (wmhints);
    XFree (classhint);
    XFree (normalhints);
    
    XMapWindow (dpy, w);
    if (seconds)
	timeout = 100;
    else
	timeout = 1000;
    for (;;)
    {
	struct pollfd	a;

	a.fd = ConnectionNumber (dpy);
	a.events = POLLIN;
	if (XEventsQueued (dpy, QueuedAfterFlush) || poll (&a, 1, timeout) > 0)
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
			    cairo_surface_destroy (background);
			    background = NULL;
			}
			cairo_xlib_surface_set_size (window, width, height);
			XClearArea (dpy, w, 0, 0, 0, 0, False);
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
	    cairo_surface_t	*buffer;
	    cairo_t		*cr;
	    int    		x_off = 0, y_off = 0;
	    int    		u_width = width, u_height = height;

	    if (square)
	    {
		if (width < height)
		    u_width = u_height = width;
		else
		    u_width = u_height = height;
		x_off = (width - u_width) / 2;
		y_off = (height - u_height) / 2;
	    }
	    /*
	     * Create a pixmap holding the background clock image
	     * so it doesn't have to be painted every tick
	     */
	    if (!background)
		background = make_background (window,
					      content, u_width, u_height, 
					      opaque);
	    
	    buffer = cairo_surface_create_similar (window, content,
						   u_width, u_height);
	    cr = cairo_create (buffer);
	    cairo_save (cr);
	    {
		cairo_set_source_surface (cr, background, 0, 0);
		cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
		cairo_paint (cr);
	    }
	    cairo_restore (cr);
	    fdhand_draw_now (cr, u_width, u_height, seconds);
	    cairo_destroy (cr);
	    
	    cr = cairo_create (window);
	    cairo_set_source_surface (cr, buffer, x_off, y_off);
	    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	    cairo_paint (cr);
	    cairo_destroy (cr);
	    
	    cairo_surface_destroy (buffer);
	    paint = False;
	}
    }
}

extern char *optarg;
extern int optind, opterr, optopt;

int
main (int argc, char **argv)
{
    char    *dpy_name = 0;
    char    *geom = 0;
    Bool    seconds = False, translucent = False, square = False, opaque = False;
    int	    c;

    while ((c = getopt (argc, argv, "staod:g:")) > 0)
    {
	switch (c) {
	case 'd':
	    dpy_name = optarg;
	    break;
	case 'g':
	    geom = optarg;
	    break;
	case 's':
	    seconds = True;
	    break;
	case 't':
	    translucent = True;
	    opaque = False;
	    break;
	case 'o':
	    translucent = True;
	    opaque = True;
	    break;
	case 'a':
	    square = True;
	    break;
	default:
	    fprintf (stderr, "usage: %s -sta -d <dpy> -g <geom>\n", argv[0]);
	    exit (1);
	    break;
	}
    }
    
    main_x (dpy_name, geom, seconds, translucent, square, opaque);
    return 0;
}
