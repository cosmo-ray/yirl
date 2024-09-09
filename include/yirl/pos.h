/*
**Copyright (C) 2016 Matthias Gatto
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

#ifndef _YIRL_POS_H_
#define _YIRL_POS_H_

#ifndef Y_INSIDE_TCC
#include <math.h>
#else
double sqrt(double x);
#endif
#include "yirl/entity.h"

#define ywSizeW(size) (ywPosX(size))
#define ywSizeH(size) (ywPosY(size))

#define ywSizeWDirect(size) (ywPosXDirect(size))
#define ywSizeHDirect(size) (ywPosYDirect(size))

static inline int ywPosX(Entity *pos)
{
  return yeGetIntAt(pos, 0);
}

static inline int ywPosY(Entity *pos)
{
  return yeGetIntAt(pos, 1);
}

static inline int ywPosXDirect(Entity *pos)
{
  if (yeType(pos) == YQUADINT)
    return ywPosX(pos);
  return yeGetIntDirect(yeGetByIdxDirect(pos, 0));
}

static inline int ywPosYDirect(Entity *pos)
{
  if (yeType(pos) == YQUADINT)
    return ywPosY(pos);
  return yeGetIntDirect(yeGetByIdxDirect(pos, 1));
}

static inline void ywPosPrint(Entity *pos)
{
  printf("x: %d - y: %d\n",
	 ywPosX(pos), ywPosY(pos));
}

static inline void ywSizePrint(Entity *s)
{
  printf("w: %d - h: %d\n",
	 ywPosX(s), ywPosY(s));
}

static inline char * ywPosSizeToString(Entity *pos, char a, char b)
{
  static char tmp[4][256];
  static int i;

  if (unlikely(!pos))
    return (char *)"(nil)";
  ++i;
  i &= 3;
  snprintf(tmp[i], 256, "%c: %d - %c: %d", a,
	   yeGetIntAt(pos, 0), b,
	   yeGetIntAt(pos, 1));
  return tmp[i];
}

#define ywPosToString(pos) ywPosSizeToString(pos, 'x', 'y')
#define ywSizeToString(pos) ywPosSizeToString(pos, 'w', 'h')

#define ywSizeCreate(w, h, father, name) (ywPosCreate(w, h, father, name))
#define ywSizeCreateAt(w, h, father, name, idx) (ywPosCreateAt(w, h, father, name, idx))

/**
 * @posX The position in X
 * @posY The position in Y
 * @father the father of the returned entity in which we store the return,
 * can be NULL.
 * @name string index at which we store the returned entity,
 * if NULL but not @father, the return is push back
 * @return an Entity that store a "Position", Must be free if @father is NULL
 */
Entity *ywPosCreateInts(int posX, int posY, Entity *father,
			const char *name);

Entity *ywPosCreateAt(int posX, int posY, Entity *father,
		      const char *name, int idx);


static inline Entity *ywPosCreateEnt(Entity *other, int useless,
				     Entity *father, const char *name)
{
  (void)useless;
  return ywPosCreateInts(ywPosX(other), ywPosY(other), father, name);
}

#define ywPosCreate(x, y, father, name)			\
  _Generic(x, Entity *: ywPosCreateEnt,			\
	   void *: ywPosCreateEnt,			\
	   const Entity *: ywPosCreateEnt,		\
	   double : ywPosCreateInts,			\
	   long long int : ywPosCreateInts,		\
	   long int : ywPosCreateInts,			\
	   int : ywPosCreateInts) (x, y, father, name)

static inline Entity *ywPosSetInts(Entity *pos, int posX, int posY)
{
	yeSetIntAt(pos, 0, posX);
	yeSetIntAt(pos, 1, posY);
	return pos;
}

static inline Entity *ywPosSetEnt(Entity *pos, Entity *other,
				  int useless)
{
	(void)useless;
	yeSetIntAt(pos, 0, yeGetIntAt(other, 0));
	yeSetIntAt(pos, 1, yeGetIntAt(other, 1));
	return pos;
}

#define ywPosSet(pos, x, y)				\
	_Generic(x, Entity *: ywPosSetEnt,		\
		 void *: ywPosSetEnt,			\
		 const Entity *: ywPosSetEnt,		\
		 double : ywPosSetInts,			\
		 long long : ywPosSetInts,			\
		 long : ywPosSetInts,			\
		 int : ywPosSetInts) (pos, x, y)


static inline Entity *ywPosSetX(Entity *pos, int posX)
{
	yeSetIntAt(pos, 0, posX);
	return pos;
}

static inline Entity *ywPosSetY(Entity *pos, int posY)
{
	yeSetIntAt(pos, 1, posY);
	return pos;
}

static inline _Bool ywPosIsSameEnt(Entity *pos1, Entity *pos2,
				   int useless)
{
	(void)useless;
	return ((yeGetIntAt(pos1, 0) == yeGetIntAt(pos2, 0)) &&
		(yeGetIntAt(pos1, 1) == yeGetIntAt(pos2, 1)));
}

static inline _Bool ywPosIsSameInts(Entity *pos1, int x, int y)
{
	return ((yeGetIntAt(pos1, 0) == x) &&
		(yeGetIntAt(pos1, 1) == y));
}

#define ywPosIsSame(pos, x, y)				\
	_Generic(x, Entity *: ywPosIsSameEnt,		\
		 void *: ywPosIsSameEnt,		\
		 const Entity *: ywPosIsSameEnt,	\
		 double : ywPosIsSameInts,		\
		 long long int: ywPosIsSameInts,	\
		 long : ywPosIsSameInts,		\
		 int : ywPosIsSameInts) (pos, x, y)


static inline int ywPosIsSameXEnt(Entity *pos1, Entity *pos2)
{
  if (yeType(pos2) == YINT)
    return (yeGetIntAt(pos1, 0) == yeGetInt(pos2));
  return (yeGetIntAt(pos1, 0) == yeGetIntAt(pos2, 0));
}

static inline int ywPosIsSameXInt(Entity *pos1, int x)
{
  return (yeGetIntAt(pos1, 0) == x);
}

#define ywPosIsSameX(pos, x)			\
  _Generic(x, Entity *: ywPosIsSameXEnt,	\
	   void *: ywPosIsSameXEnt,		\
	   const Entity *: ywPosIsSameXEnt,	\
	   double : ywPosIsSameXInt,		\
	   int : ywPosIsSameXInt) (pos, x)

static inline int ywPosIsSameYEnt(Entity *pos1, Entity *pos2)
{
  if (yeType(pos2) == YINT)
    return (yeGetIntAt(pos1, 1) == yeGetInt(pos2));
  return (yeGetIntAt(pos1, 1) == yeGetIntAt(pos2, 0));
}

static inline int ywPosIsSameYInt(Entity *pos1, int x)
{
  return (yeGetIntAt(pos1, 1) == x);
}

#define ywPosIsSameY(pos, x)			\
  _Generic(x, Entity *: ywPosIsSameYEnt,	\
	   void *: ywPosIsSameYEnt,		\
	   const Entity *: ywPosIsSameYEnt,	\
	   double : ywPosIsSameYInt,		\
	   int : ywPosIsSameYInt) (pos, x)

static inline Entity *ywPosAdd(Entity *pos1, Entity *pos2)
{
	yeAddAt(pos1, 0, yeGetIntAt(pos2, 0));
	yeAddAt(pos1, 1, yeGetIntAt(pos2, 1));
	return pos1;
}

static inline Entity *ywPosSubXY(Entity *pos1, int x, int y)
{
	yeAddAt(pos1, 0, -x);
	yeAddAt(pos1, 1, -y);
	return pos1;
}

static inline Entity *ywPosSub(Entity *pos1, Entity *pos2)
{
	yeAddAt(pos1, 0, -yeGetIntAt(pos2, 0));
	yeAddAt(pos1, 1, -yeGetIntAt(pos2, 1));
	return pos1;
}

static inline Entity *ywPosMultXY(Entity *pos1, int x, int y)
{
	if (yeType(pos1) == YQUADINT) {
		yeMultQuadIntVals(pos1, x, y, 1, 1);
	} else {
		yeMultInt(yeGet(pos1, 0), x);
		yeMultInt(yeGet(pos1, 1), y);
	}
	return pos1;
}

static inline Entity *ywPosDoPercent(Entity *pos1, int arg)
{
	yeSetIntAt(pos1, 0, yuiPercentOf(yeGetIntAt(pos1, 0), arg));
	yeSetIntAt(pos1, 1, yuiPercentOf(yeGetIntAt(pos1, 1), arg));
	return pos1;
}

static inline Entity *ywPosAddXY(Entity *pos, int x, int y)
{
	yeAddAt(pos, 0, x);
	yeAddAt(pos, 1, y);
	return pos;
}

static inline Entity *ywPosAddXYAbsMaxXY(Entity *pos, int x, int y, int xmax, int ymax)
{
	ywPosAddXY(pos, x, y);
	if (yeType(pos) == YQUADINT) {
		yeQuadIntForceBound(pos, 0, -xmax, xmax);
		yeQuadIntForceBound(pos, 1, -ymax, ymax);
	} else {
		yeIntForceBound(yeGetByIdx(pos, 0), -xmax, xmax);
		yeIntForceBound(yeGetByIdx(pos, 1), -ymax, ymax);
	}
	return pos;
}

static inline Entity *ywPosAddCopy(Entity *origin, int x, int y, Entity *parent, const char *key)
{
	Entity *ret = yeCreateCopy(origin, parent, key);
	ywPosAddXY(ret, x, y);
	return ret;
}

static inline Entity *ywPosAddXYAbsMax(Entity *pos, int x, int y, int max)
{
	return ywPosAddXYAbsMaxXY(pos, x, y, max, max);
}

static inline Entity *ywSegmentFromPos(Entity *posA, Entity *posB,
				       Entity *father, const char *name)
{
  Entity *ret = yeCreateArray(father, name);

  yeCopy(posB, ret);
  ywPosSub(ret, posA);
  return ret;
}

double ywPosAngle(Entity *p0, Entity *p1);

static inline int ywPosMoveToward2(Entity *from, Entity *to,
				   int max_x, int max_y)
{
  int x = 0, y = 0, s;

  x = ywPosX(to) - ywPosX(from);
  s = x;
  x = yuiAbs(x) > max_x ? max_x : yuiAbs(x);
  x = s < 0 ? -x : x;

  y = ywPosY(to) - ywPosY(from);
  s = y;
  y = yuiAbs(y) > max_y ? max_y : yuiAbs(y);
  y = s < 0 ? -y : y;

  ywPosAddXY(from, x, y);
  return x || y;
}


/**
 * move of 1 @from in the direction of @to
 * example: from is 0,0 and to is 0,3, this function will
 * increment X in @from, so @from will be 0,1 so if you call
 * this function with the same argument 3 times @from will be equal to
 * 0,3 (@to value)
 * @return 1 if @from has been modifie
 */
static inline int ywPosMoveToward(Entity *from, Entity *to)
{
  int x = 0, y = 0;

  if (ywPosX(from) > ywPosX(to))
    x = -1;
  else if (ywPosX(from) < ywPosX(to))
    x = 1;

  if (ywPosY(from) > ywPosY(to))
    y = -1;
  else if (ywPosY(from) < ywPosY(to))
    y = 1;

  ywPosAddXY(from, x, y);
  return x || y;
}

/* more usefull for map where rounding errors are expected
 * and when you don't allow diagonal movement
 */
static inline uint32_t ywPosTotCases(Entity *p0, Entity *p1)
{
	uint32_t x = abs(ywPosX(p0) - ywPosX(p1));
	uint32_t y = abs(ywPosY(p0) - ywPosY(p1));

	return x + y;
}

static inline int ywPosXDistance(Entity *p0, Entity *p1)
{
	return ywPosX(p1) - ywPosX(p0);
}

static inline int ywPosYDistance(Entity *p0, Entity *p1)
{
	return  ywPosY(p1) - ywPosY(p0);
}

/**
 * @brief compute distance between 2 pos
 */
static inline uint32_t ywPosDistance(Entity *p0, Entity *p1)
{
	uint32_t x = ywPosX(p0) - ywPosX(p1);
	uint32_t y = ywPosY(p0) - ywPosY(p1);

	return sqrt(x * x + y * y);
}


/**
 * @return true if the point at opos is inside the rect defined by pos and size
 * if proper is true
 * doesn't return true when on the edge
 */
static inline _Bool ywPosIsInRectPS(Entity *point, Entity *pos, Entity *size,
				      int proper)
{
	return ywPosX(point) > ywPosX(pos) - !(proper) &&
		ywPosX(point) < ywPosX(pos) + ywSizeW(size) - !!(proper) &&
		ywPosY(point) > ywPosY(pos) - !(proper) &&
		ywPosY(point) < ywPosY(pos) + ywSizeH(size) - !!(proper);
}


/**
 * @brief compute distance of a size
 */
static inline uint32_t ywSizeDistance(Entity *size)
{
  uint32_t w = ywSizeW(size);
  uint32_t h = ywSizeH(size);

  return sqrt(w * w + h * h);
}

#endif
