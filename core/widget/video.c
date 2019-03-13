/*
**Copyright (C) 2019 Matthias Gatto
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

#include "glib.h"
#include "widget.h"

static int t = -1;

struct YVideoState{
	YWidgetState sate;
};

static int videoInit(YWidgetState *opac, Entity *entity, void *args)
{
	return 0;
}

static int rend(YWidgetState *opac)
{
	ywidGenericRend(opac, t, render);
	return 0;
}

static void *alloc(void)
{
	struct YVideoState *ret = g_new0(struct YVideoState, 1);
	YWidgetState *wstate = (YWidgetState *)ret;

	if (!ret)
		return NULL;
	wstate->render = rend;
	wstate->init = videoInit;
	wstate->destroy = ywDestroyState;
	wstate->handleEvent = ywidEventCallActionSin;
	wstate->type = t;
	return  ret;
}

int ywVideoInit(void)
{
	if (t != -1)
		return t;
	t = ywidRegister(alloc, "video");
	return t;
}
