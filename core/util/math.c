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

#define	DONT_INTERSECT    0
#define	DO_INTERSECT      1

/**************************************************************
 *                                                            *
 *    NOTE:  The following macro to determine if two numbers  *
 *    have the same sign, is for 2's complement number        *
 *    representation.  It will need to be modified for other  *
 *    number systems.                                         *
 *                                                            *
 **************************************************************/

#define SAME_SIGNS( a, b )	\
		(((int) ((unsigned int) a ^ (unsigned int) b)) >= 0 )

int yuiLinesIntersect(int x1, int y1, int x2, int y2,
		      int x3, int y3, int x4, int y4,
		      int *x, int *y)
{
	int a1, a2, b1, b2, c1, c2; /* Coefficients of line eqns. */
	int r1, r2, r3, r4;         /* 'Sign' values */
	int denom, offset, num;     /* Intermediate values */

	/* Compute a1, b1, c1, where line joining points 1 and 2
	 * is "a1 x  +  b1 y  +  c1  =  0".
	 */

	a1 = y2 - y1;
	b1 = x1 - x2;
	c1 = x2 * y1 - x1 * y2;

	/* Compute r3 and r4.
	 */


	r3 = a1 * x3 + b1 * y3 + c1;
	r4 = a1 * x4 + b1 * y4 + c1;

	/* Check signs of r3 and r4.  If both point 3 and point 4 lie on
	 * same side of line 1, the line segments do not intersect.
	 */

	if ( r3 != 0 &&
	     r4 != 0 &&
	     SAME_SIGNS( r3, r4 ))
		return ( DONT_INTERSECT );

	/* Compute a2, b2, c2 */

	a2 = y4 - y3;
	b2 = x3 - x4;
	c2 = x4 * y3 - x3 * y4;

	/* Compute r1 and r2 */

	r1 = a2 * x1 + b2 * y1 + c2;
	r2 = a2 * x2 + b2 * y2 + c2;

	/* Check signs of r1 and r2.  If both point 1 and point 2 lie
	 * on same side of second line segment, the line segments do
	 * not intersect.
	 */

	if ( r1 != 0 &&
	     r2 != 0 &&
	     SAME_SIGNS(r1, r2))
		return DONT_INTERSECT;

	/* Line segments intersect: compute intersection point. 
	 */

	denom = a1 * b2 - a2 * b1;
	if ( denom == 0 )
		return DONT_INTERSECT;

	offset = denom < 0 ? - denom / 2 : denom / 2;

	/* The denom/2 is to get rounding instead of truncating.  It
	 * is added or subtracted to the numerator, depending upon the
	 * sign of the numerator.
	 */

	/* x/y are optionals */
	if (x) {
		num = b1 * c2 - b2 * c1;
		*x = ( num < 0 ? num - offset : num + offset ) / denom;
	}
	if (y) {
		num = a2 * c1 - a1 * c2;
		*y = ( num < 0 ? num - offset : num + offset ) / denom;
	}

	return DO_INTERSECT;
} /* lines_intersect */

int yuiLinesRectIntersect(int x1, int y1, int x2, int y2,
			  int rx, int ry, int rw, int rh,
			  int *x, int *y)
{
	int r;

	if (x1 < rx) {
		/* left seg */
		r = yuiLinesIntersect(x1, y1, x2, y2, rx, ry, rx, ry + rh, x, y);
		if (r)
			return r;
	} else {
		/* right seg */
		r = yuiLinesIntersect(x1, y1, x2, y2, rx + rw,
				      ry, rx + rw, ry + rh, x, y);
		if (r)
			return r;
	}

	if (y1 < ry) {
		/* top seg */
		r = yuiLinesIntersect(x1, y1, x2, y2, rx, ry, rx + rw, ry, x, y);
		if (r)
			return r;
	} else {
		/* bottom */
		r = yuiLinesIntersect(x1, y1, x2, y2,
				      rx, ry + rh, rx + rw, ry + rh, x, y);
		if (r)
			return r;
	}
	return 0;
}
