/*
**Copyright (C) 2015 Matthias Gatto
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

#ifndef _YIRL_KEYDEF_H_
#define _YIRL_KEYDEF_H_

/* For ascii char use char representation 'a' '\n'... */

/* Steal that from sdl, I guess "0x40000000" mean something,
 * I just didn't know what at the moment I write those lines */
#define Y_LSHIFT_KEY 0x400000e1
#define Y_RSHIFT_KEY 0x400000e5
#define Y_LCTRL_KEY 0x400000e0
#define Y_RCTRL_KEY 0x400000e4

#define Y_ESC_KEY 27
#define Y_UP_KEY 259
#define Y_DOWN_KEY 258
#define Y_LEFT_KEY 260
#define Y_RIGHT_KEY 261

#endif
