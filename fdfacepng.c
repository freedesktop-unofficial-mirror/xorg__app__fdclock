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
#include <stdlib.h>
#include <stdio.h>
#include "fdface.h"

static int
dump_png (int width, int height)
{
    cairo_surface_t *surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32,
							   width, height);
    cairo_t *cr = cairo_create (surface);
    char    out[1024];
    
    cairo_move_to (cr, 0, 0);
    cairo_line_to (cr, width, 0);
    cairo_line_to (cr, width, height);
    cairo_line_to (cr, 0, height);
    cairo_close_path (cr);
    cairo_set_source_rgb (cr, 1, 1, 1);
    cairo_fill (cr);
    fdface_draw (cr, width, height);
    cairo_destroy (cr);
    
    sprintf (out, "freedesktop-clock-%d.png", width);
    cairo_surface_write_to_png (surface, out);
    cairo_surface_destroy (surface);
    return 0;
}

int
main (int argc, char **argv)
{
    return dump_png (2400, 2400);
}
