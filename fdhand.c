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

#include "fdhand.h"
#include <sys/time.h>
#include <time.h>
#include <math.h>

#define PI_6	(M_PI/6.0)

static void
draw_hand_helper (cairo_t *cr, double width, double length, int draw_disc)
{
    double  r = width / 2;
    cairo_move_to (cr,	length,		    -r);
    cairo_arc (cr,	length,		    0,		r, -M_PI_2, M_PI_2);
    if (draw_disc)
    {
	cairo_line_to (cr,  width * M_SQRT2,    r);
	cairo_arc (cr,	0,		    0,		r*2, PI_6, M_PI - PI_6);
    }
    cairo_line_to (cr,	-length / 10,	    r);
    cairo_arc (cr,	-length / 10,	    0,		r, M_PI_2, -M_PI_2);
    if (draw_disc)
    {
	cairo_line_to (cr,	-width * M_SQRT2,   -r);
	cairo_arc (cr,	0,		    0,		r*2, M_PI + PI_6, -PI_6);
    }
    cairo_close_path (cr);
}

static void
draw_hand (cairo_t *cr, double angle, double width, double length, double alt, int draw_disc)
{
    cairo_save (cr);
    {
	cairo_translate (cr, alt, alt);
	cairo_rotate (cr, angle);
	draw_hand_helper (cr, width, length, draw_disc);
	cairo_set_rgb_color (cr, 0, 0, 0);
	cairo_set_alpha (cr, 0.5);
	cairo_fill (cr);
    }
    cairo_restore (cr);
    cairo_save (cr);
    {
	cairo_rotate (cr, angle);
	cairo_set_rgb_color (cr, 0, 0, 0);
	draw_hand_helper (cr, width, length, draw_disc);
	cairo_fill (cr);
    }
    cairo_restore (cr);
}

static void
draw_time (cairo_t *cr, double width, double height, struct timeval *tv, int seconds)
{
    double  hour_angle, minute_angle, second_angle;
    struct tm	tm_ret;
    struct tm	*tm;

    tm = localtime_r (&tv->tv_sec, &tm_ret);
    
    second_angle = (tm->tm_sec + tv->tv_usec / 1000000.0) * 6.0;
    minute_angle = tm->tm_min * 6.0 + second_angle / 60.0;
    hour_angle = tm->tm_hour * 30.0 + minute_angle / 12.0;

    cairo_save (cr);
    {
	cairo_scale (cr, width, height);
	cairo_translate (cr, 0.5, 0.5);
	draw_hand (cr, hour_angle * M_PI / 180.0 - M_PI_2, 0.03, 0.25, 0.005, 1);
	draw_hand (cr, minute_angle * M_PI / 180.0 - M_PI_2, 0.02, 0.4, 0.010, 0);
	if (seconds)
	    draw_hand (cr, second_angle * M_PI / 180.0 - M_PI_2, 0.01, 0.3, 0.015, 0);
    }
    cairo_restore (cr);
}

void
fdhand_draw_now (cairo_t *cr, double width, double height, int seconds)
{
    struct timeval  tv;
    struct timezone tz;
    gettimeofday (&tv, &tz);

    draw_time (cr, width, height, &tv, seconds);
}

