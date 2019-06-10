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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <glib.h>

#include <src/duktape.h>
#include <extras/print-alert/duk_print_alert.h>
#include <extras/console/duk_console.h>

#include "game.h"
#include "canvas.h"
#include "widget.h"
#include "pos.h"
#include "entity-script.h"
#include "events.h"

static int t = -1;

#define OPS(s) ((YScriptDuk *)s)->ops
#define CTX(s) ((YScriptDuk *)s)->ctx

typedef struct {
	YScriptOps ops;
	duk_context *ctx;
} YScriptDuk;

static void *call(void *sm, const char *name, va_list ap)
{
	duk_context *ctx = CTX(sm);
	int i = 0;

	duk_get_global_string(ctx, name);
	for (void *tmp = va_arg(ap, void *); tmp != Y_END_VA_LIST;
	     tmp = va_arg(ap, void *)) {
		if (!yeIsPtrAnEntity(tmp))
			duk_push_int(ctx, (intptr_t)tmp);
		else
			duk_push_pointer(ctx, tmp);
		++i;
	}
	duk_call(ctx, i);
	return (void *)duk_get_pointer(ctx, duk_get_top(ctx));
}

static int destroy(void *sm)
{
	duk_destroy_heap(CTX(sm));
	free(sm);
	return 0;
}

static int init(void *sm, void *args)
{
	duk_context *ctx = duk_create_heap_default();

	duk_print_alert_init(ctx, 0);
	duk_console_init(ctx, 0);
	CTX(sm) = ctx;
	return 0;
}

static int loadString(void *s, const char *str)
{
	duk_eval_string(CTX(s), str);
	return 0;
}

static int loadFile(void *s, const char *file)
{
	int fd = open(file, O_RDONLY);
	yeAutoFree Entity *fstr = yeCreateString(NULL, NULL, NULL);
	struct stat st;

	if (stat(file, &st) < 0 || fd < 0) {
		DPRINT_ERR("cannot open/stat '%s'", file);
		return -1;
	}
	yeAddStrFromFd(fstr, fd, st.st_size);
	loadString(s, yeGetString(fstr));
	close(fd);
	return 0;
}

static void *allocator(void)
{
	YScriptDuk *ret;

	ret = g_new0(YScriptDuk, 1);
	ret->ops.init = init;
	ret->ops.destroy = destroy;
	ret->ops.loadFile = loadFile;
	ret->ops.loadString = loadString;
	ret->ops.call = call;
	return (void *)ret;
}

int ysDukInit(void)
{
	t = ysRegister(allocator);
	return t;
}

int ysDukEnd(void)
{
	return ysUnregiste(t);
}

int ysDukGetType(void)
{
	return t;
}
