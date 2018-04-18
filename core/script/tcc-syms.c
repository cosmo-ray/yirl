/*
**Copyright (C) 2018 Matthias Gatto
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

#include <string.h>
#include <glib.h>

#include "tcc-script.h"
#include "texture.h"
#include "canvas.h"
#include "entity-script.h"
#include "game.h"

void	fflushout(void)
{
  fflush(stdout);
}

void	tccAddSyms(TCCState *l)
{
#ifdef WIN32
    tcc_add_symbol(l, "free", free);
    tcc_add_symbol(l, "fflushout", fflushout);
    tcc_add_symbol(l, "strdup", strdup);
    tcc_add_symbol(l, "printf", printf);
    tcc_add_symbol(l, "ygFileToEnt", ygFileToEnt);
    tcc_add_symbol(l, "yeGetByStrFast", yeGetByStrFast);
    tcc_add_symbol(l, "yBlockArrayIteratorCreate", yBlockArrayIteratorCreate);
    tcc_add_symbol(l, "memmove", memmove);
    tcc_add_symbol(l, "yBlockArrayIteratorIncr", yBlockArrayIteratorIncr);
    tcc_add_symbol(l, "yeGetString", yeGetString);
    tcc_add_symbol(l, "strlen", strlen);
    tcc_add_symbol(l, "snprintf", snprintf);
    tcc_add_symbol(l, "yeRenamePtrStr", yeRenamePtrStr);
    tcc_add_symbol(l, "ywTextureNewImg", ywTextureNewImg);
    tcc_add_symbol(l, "ywRectCreateInts", ywRectCreateInts);
    tcc_add_symbol(l, "yeCreateArrayByCStr", yeCreateArrayByCStr);
    tcc_add_symbol(l, "yeGetInt", yeGetInt);
    tcc_add_symbol(l, "ywCanvasNewImgFromTexture", ywCanvasNewImgFromTexture);
    tcc_add_symbol(l, "ywCanvasRotate", ywCanvasRotate);
    tcc_add_symbol(l, "yeLen", yeLen);
    tcc_add_symbol(l, "yeStrCmp", yeStrCmp);
    tcc_add_symbol(l, "yeCreateString", yeCreateString);
    tcc_add_symbol(l, "yeCreateInt", yeCreateInt);
    tcc_add_symbol(l, "yeCreateFloat", yeCreateFloat);
    tcc_add_symbol(l, "yeDestroy", yeDestroy);
    tcc_add_symbol(l, "yePushBack", yePushBack);
    tcc_add_symbol(l, "ygGetManager", ygGetManager);
    tcc_add_symbol(l, "yeCreateFunction", yeCreateFunction);
    tcc_add_symbol(l, "yeGetFloat", yeGetFloat);
    tcc_add_symbol(l, "yeGetByIdx", yeGetByIdx);
    tcc_add_symbol(l, "yeSetInt", yeSetInt);
    tcc_add_symbol(l, "yesCallInt", yesCallInt);
#else
    (void)l;
#endif
}
