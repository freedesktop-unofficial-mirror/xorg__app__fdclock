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
#include <unistd.h>
#include "fdface.h"
#include "fdhand.h"
#include "findargb.h"

#define CLOCK_WIDTH	150
#define CLOCK_HEIGHT	150

static void
clear (cairo_t	*cr, double width, double height, double alpha)
{
    cairo_save (cr);
    cairo_set_rgb_color (cr, 1, 1, 1);
    cairo_set_alpha (cr, alpha);
    cairo_set_operator (cr, CAIRO_OPERATOR_SRC);
    cairo_rectangle (cr, 0, 0, width, height);
    cairo_fill (cr);
    cairo_restore (cr);
}
    
Pixmap
make_background (Display *dpy, Window root, int width, int height, int depth,
		 Visual *visual, Colormap cmap)
{
    cairo_t		*cr = cairo_create ();
    cairo_surface_t	*back_surface;
    cairo_surface_t	*temp_surface = 0;
    Pixmap		background;

    /*
     * Background pixmap 
     */
    background = XCreatePixmap (dpy, root, width, height, depth);
    back_surface = cairo_xlib_surface_create (dpy, background, visual, 0, cmap);

    if (depth == 32)
    {
	temp_surface = cairo_surface_create_similar (back_surface,
						     CAIRO_FORMAT_ARGB32,
						     width, height);
	/*
	 * draw the background to the temporary surface
	 */
    
    
	cairo_set_target_surface (cr, temp_surface);
    }
    else
	cairo_set_target_surface (cr, back_surface);

    clear (cr, width, height, temp_surface ? 0 : 1);

    if (temp_surface)
    {
	cairo_save (cr);
	{
	    cairo_set_rgb_color (cr, 1, 1, 1);
	    cairo_set_alpha (cr, 0.5);
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
	cairo_set_target_surface (cr, back_surface);

	clear (cr, width, height, 0);

	cairo_set_alpha (cr, 0.8);

	cairo_show_surface (cr, temp_surface, width, height);
	cairo_surface_destroy (temp_surface);
    }
    cairo_surface_destroy (back_surface);
    cairo_destroy (cr);
    return background;
}

void
main_x (char *dpy_name, char *geom, Bool seconds, Bool translucent)
{
    Display		    *dpy;
    int			    scr;
    int			    x = 0, y = 0;
    int			    width = CLOCK_WIDTH, height = CLOCK_HEIGHT;
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
    GC			    gc;
    XGCValues		    gcv;
    Pixmap		    background = None, buffer;
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
	XParseGeometry (geom, &x, &y, &width, &height);

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
    }
    else
    {
	visual = DefaultVisual (dpy, scr);
	depth = DefaultDepth (dpy, scr);
	wattr.colormap = cmap = DefaultColormap (dpy, scr);
	wattr.background_pixel = WhitePixel (dpy, scr);
	wattr.border_pixel = BlackPixel (dpy, scr);
    }
    wmask = CWEventMask | CWBackPixel | CWBorderPixel | CWColormap;
    w = XCreateWindow (dpy, root, x, y, width, height, 0,
		       depth, InputOutput, visual, wmask, &wattr);
    
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
    gcv.graphics_exposures = False;
    gc = XCreateGC (dpy, w, GCGraphicsExposures, &gcv);
    if (seconds)
	timeout = 100;
    else
	timeout = 1000;
    for (;;)
    {
	struct pollfd	a;

	XFlush (dpy);
	a.fd = ConnectionNumber (dpy);
	a.events = POLLIN;
	if (poll (&a, 1, timeout) > 0)
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
	    cairo_surface_t	*buffer_surface;
	    cairo_t		*cr = cairo_create ();

	    /*
	     * Create a pixmap holding the background clock image
	     * so it doesn't have to be painted every tick
	     */
	    if (!background)
		background = make_background (dpy, root, width, height, depth,
					      visual, cmap);
	    
	    buffer = XCreatePixmap (dpy, root,
				    width, height, depth);
	    buffer_surface = cairo_xlib_surface_create (dpy, buffer,
							visual, 0, cmap);
	    cairo_set_target_surface (cr, buffer_surface);
	    XCopyArea (dpy, background, buffer, gc, 
		       0, 0, width, height, 0, 0);
	    fdhand_draw_now (cr, width, height, seconds);
	    XCopyArea (dpy, buffer, w, gc, 0, 0, width, height, 0, 0);
	    cairo_surface_destroy (buffer_surface);
	    XFreePixmap (dpy, buffer);
	    paint = False;
	    cairo_destroy (cr);
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
    Bool    seconds = False, translucent = False;
    int	    c;

    while ((c = getopt (argc, argv, "std:g:")) > 0)
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
	    break;
	default:
	    fprintf (stderr, "usage: %s -st -d <dpy> -g <geom>\n", argv[0]);
	    exit (1);
	    break;
	}
    }
    
    main_x (dpy_name, geom, seconds, translucent);
    return 0;
}
