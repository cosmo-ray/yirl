/*
**Copyright (C) 2020 Matthias Gatto
**
**This program is free software: you can redistribute it and/or modify
**it under the terms of the GNU Lesser General Public License as published by
**the Free Software Foundation, either version 3 of the License, or
**(at your option) any later version.
**
**This program is distributed in the hope that it will be useful,
**but WITHOUT ANY WARRANTY; without even the implied warranty of
**MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**GNU General Public License for more details.
**
**You should have received a copy of the GNU Lesser General Public License
**along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* functions greatly inspired(copied) by Mukesh Prasad work "GraphicsGems" */
/* here: https://github.com/erich666/GraphicsGems */

#include "utils.h"

#define	DONT_INTERSECT    0
#define	DO_INTERSECT      1

static inline int SAME_SIGNS(int a, int b)
{
	return ((a < 0 && b < 0) || (a > 0 && b > 0));
}

int yuiLinesIntersect(int x1, int y1, int x2, int y2,
		      int x3, int y3, int x4, int y4,
		      int *x, int *y)
{
	int64_t seg1_w ,seg2_w,dist_x,seg1_h,seg2_h,dist_y;
	int64_t num_d,num_e,denominator,num,offset;
	int x1lo,x1hi,y1lo,y1hi;

	seg1_w = x2-x1;
	seg2_w = x3-x4;

	if(seg1_w<0) {                                              /* X bound box test*/
		x1lo=x2; x1hi=x1;
	} else {
		x1hi=x2; x1lo=x1;
	}
	if(seg2_w>0) {
		if(x1hi < x4 || x3 < x1lo) return DONT_INTERSECT;
	} else {
		if(x1hi <x3 || x4 < x1lo) return DONT_INTERSECT;
	}

	seg1_h = y2-y1;
	seg2_h = y3-y4;

	if(seg1_h<0) {                                              /* Y bound box test*/
		y1lo=y2; y1hi=y1;
	} else {
		y1hi=y2; y1lo=y1;
	}
	if(seg2_h>0) {
		if(y1hi < y4 || y3 < y1lo) return DONT_INTERSECT;
	} else {
		if(y1hi < y3 || y4 < y1lo) return DONT_INTERSECT;
	}

	dist_x = x1-x3;
	dist_y = y1-y3;

	denominator = seg1_h*seg2_w - seg1_w*seg2_h;

	if(denominator==0) return DONT_INTERSECT;

	num_d = seg2_h*dist_x - seg2_w*dist_y;  /* alpha numerator*/
	if(denominator>0) {    /* alpha tests*/
		if(num_d<0 || num_d>denominator) return DONT_INTERSECT;
	} else {
		if(num_d>0 || num_d<denominator) return DONT_INTERSECT;
	}

	num_e = seg1_w*dist_y - seg1_h*dist_x;  /* beta numerator*/
	if(denominator>0) {   /* beta tests*/
		if(num_e<0 || num_e>denominator) return DONT_INTERSECT;
	} else {
		if(num_e>0 || num_e<denominator) return DONT_INTERSECT;
	}

	if (x) {
		num = num_d*seg1_w;   /* numerator */
		offset = SAME_SIGNS(num,denominator) ? denominator/2 :
			-denominator/2;  /* round direction*/
		*x = x1 + (num+offset) / denominator; /* intersection x */
	}

	if (y) {
		num = num_d*seg1_h;
		offset = SAME_SIGNS(num,denominator) ? denominator/2 :
			-denominator/2;
		*y = y1 + (num+offset) / denominator; /* intersection y */
	}

	return DO_INTERSECT;

} /* lines_intersect */

int yuiLinesRectIntersect(int x1, int y1, int x2, int y2,
			  int rx, int ry, int rw, int rh,
			  int *x, int *y)
{
	int r_x;
	int r_y;
	int x_h;
	int y_h;

	if (unlikely(!x || !y)) {
		DPRINT_ERR("need y and x");
		return 0;
	}

	if (x1 < rx) {
		r_x = yuiLinesIntersect(x1, y1, x2, y2,
					rx, ry, rx, ry + rh,
					x, y);
	} else {
		r_x = yuiLinesIntersect(x1, y1, x2, y2, rx + rw,
					ry, rx + rw, ry + rh, x, y);
	}

	if (y1 < ry) {
		r_y = yuiLinesIntersect(x1, y1, x2, y2,
					rx, ry, rx + rw, ry, &x_h, &y_h);
	} else {
		r_y = yuiLinesIntersect(x1, y1, x2, y2,
					rx, ry + rh, rx + rw, ry + rh, &x_h, &y_h);
	}

	/* I want to rename r_y to black, so I could have black rx */
	/* Did you know that I've learn the henshin coregraphie of the Kamen Rider Black ? */
	if (r_x && r_y) {
		int w_d = yuiPointsDist(x1, y1, *x, *y);
		int h_d = yuiPointsDist(x1, y1, x_h, y_h);

		if (w_d < h_d) {
			return w_d;
		}
		*x = x_h;
		*y = y_h;
		return h_d;
	} else if (r_x) {
		return yuiPointsDist(x1, y1, *x, *y);
	} else if (r_y) {
		*x = x_h;
		*y = y_h;
		return yuiPointsDist(x1, y1, x_h, y_h);
	}
	return 0;
}
