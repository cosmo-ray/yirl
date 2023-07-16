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

#include <yirl/entity.h>
#include <yirl/game.h>
#include <yirl/texture.h>
#include <yirl/canvas.h>
#include <yirl/condition.h>

static const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
static const unsigned FLIPPED_VERTICALLY_FLAG   = 0x40000000;
static const unsigned FLIPPED_DIAGONALLY_FLAG   = 0x20000000;

static const unsigned TILED_MERGE_LAYER_0 = 1;

char *assetsPath;

void *deinit(int nbArg, void **args)
{
    if (assetsPath) {
	free(assetsPath);
	assetsPath = NULL;
    }
    return NULL;
}

void *setAssetPath(int nbArg, void **args)
{
    const char *newPath = args[0];

    if (nbArg < 1) {
	DPRINT_ERR("wrong number of arguments:"
		   "should be setAssetPath(path)");
    }
    deinit(0, NULL);
    assetsPath = strdup(newPath);
    return assetsPath;
}

static void handle_properties(Entity *properties,
			      Entity *proTypes, Entity *dest)
{
    int proTypesIt = 0;

    YE_ARRAY_FOREACH_EXT(properties, property, it) {
	Entity *val = property;
	const char *name;
	Entity *proType = yeGet(proTypes, proTypesIt);

	++proTypesIt;
	if (proTypes) {
	    name = yBlockArrayIteratorGetPtr(it, ArrayEntry)->name;
	} else {
	    name = yeGetStringAt(property, "name");
	    proType = yeGet(property, "type");
	    val = yeGet(property, "value");
	}
	if (!yeStrCmp(proType, "int") || !yeStrCmp(proType, "bool")) {
	    yeCreateInt(yeGetInt(val), dest, name);
	} else if (!yeStrCmp(proType, "string")) {
	    yeCreateString(yeGetString(val), dest, name);
	} else if (!yeStrCmp(proType, "float")) {
	    yeCreateFloat(yeGetFloat(val), dest, name);
	}
    }
}

void *fileToCanvas(int nbArg, void **args)
{
    const char *path = args[0];
    Entity *canvas = args[1];
    Entity *upCanvas = nbArg > 2 ? args[2] : NULL;
    int main_flags = nbArg > 3 ? (intptr_t)args[3] : NULL;
    Entity *tiledEnt;
    Entity *tileset_array;
    void *ret = NULL;
    Entity *layers;
    int layer_cnt = -1;

    if (nbArg < 2) {
	DPRINT_ERR("wrong number of arguments:"
		   "should be fileToCanvas(path, canvas)");
	return NULL;
    }

    tiledEnt = ygFileToEnt(YJSON, path, NULL);
    tileset_array = yeGet(tiledEnt, "tilesets");

    handle_properties(yeGet(tiledEnt, "properties"),
		      yeGet(tiledEnt, "propertytypes"),
		      canvas);
    YE_ARRAY_FOREACH(tileset_array, tileset) {
	const char *tmp_path = yeGetString(yeGet(tileset, "source"));
	Entity *tmp;

	if (assetsPath) {
	    /* strings length + 1 for \0 and +1 for '\' */
	    int buf_size = strlen(assetsPath) + strlen(tmp_path) + 2;
	    char buf[1024]; /* should be a VLA, but need fix on windows */

	    snprintf(buf, buf_size, "%s/%s", assetsPath, tmp_path);
	    tmp = ygFileToEnt(YJSON, buf, NULL);
	} else {
	    tmp = ygFileToEnt(YJSON, tmp_path, NULL);
	}

	if (!tmp) {
	    DPRINT_ERR("can't load '%s'\n", tmp_path);
	    goto exit;
	}
	yePushBack(tileset, tmp, "_ent");
	yeDestroy(tmp);
    }

    layers = yeGet(tiledEnt, "layers");
    int tiled_wpix = yeGetIntAt(tiledEnt, "width") *
	    yeGetIntAt(tiledEnt, "tilewidth");
    int tiled_hpix = yeGetIntAt(tiledEnt, "height") *
	    yeGetIntAt(tiledEnt, "tileheight");

    yeReCreateInt(tiled_wpix, canvas, "tiled-wpix");
    yeReCreateInt(tiled_hpix, canvas, "tiled-hpix");

    if (main_flags & TILED_MERGE_LAYER_0) {
	    if (tiled_wpix > 2024 || tiled_hpix > 2024)
		    main_flags |= ~TILED_MERGE_LAYER_0;
    }

    Entity *resources = yeReCreateArray(canvas, "resources", NULL);

    YE_ARRAY_FOREACH(layers, layer) {
	Entity *layer_name = yeGet(layer, "name");
	Entity *layer_data  = yeGet(layer, "data");
	int data_len = yeLen(layer_data);
	int layer_w  = yeGetIntAt(layer, "width");
	Entity *properties = yeGet(layer, "properties");
	Entity *objects = yeGet(layer, "objects");
	Entity *proTypes = yeGet(layer, "propertytypes");

	++layer_cnt;
	if (objects) {
	    yeTryCreateArray(canvas, "objects");

	    YE_ARRAY_FOREACH(objects, object) {
		Entity *obj = yeCreateArray(yeGet(canvas,
						  "objects"),
					    NULL);
		Entity *properties = yeGet(object, "properties");
		Entity *proTypes = yeGet(object, "propertytypes");
		int proTypesIt = 0;

		yeGetPush(object, obj, "visible");
		yeGetPush(object, obj, "id");
		yeGetPush(object, obj, "name");
		yePushBack(obj, layer_name, "layer_name");
		ywRectCreateInts(yeGetIntAt(object, "x"),
				 yeGetIntAt(object, "y"),
				 yeGetIntAt(object, "width"),
				 yeGetIntAt(object, "height"),
				 obj, "rect");

		YE_ARRAY_FOREACH_EXT(properties, property, it) {
		    Entity *val = property;
		    const char *name;
		    Entity *proType = yeGet(proTypes, proTypesIt);

		    ++proTypesIt;
		    if (proTypes) {
			name = yBlockArrayIteratorGetPtr(it,
							 ArrayEntry)->name;
		    } else {
			name = yeGetStringAt(property, "name");
			proType = yeGet(property, "type");
			val = yeGet(property, "value");
		    }
		    if (!yeStrCmp(proType, "int") ||
			!yeStrCmp(proType, "bool")) {
			yeCreateInt(yeGetInt(val), obj, name);
		    } else if (!yeStrCmp(proType, "string")) {
			yeCreateString(yeGetString(val), obj, name);
		    } else if (!yeStrCmp(proType, "float")) {
			yeCreateFloat(yeGetFloat(val), obj, name);
		    }
		}
	    }
	    continue;
	}

	YE_ARRAY_FOREACH(tileset_array, tileset) {
	    Entity *tmp = yeGet(tileset, "_ent");
	    int firstgid = yeGetIntAt(tileset, "firstgid");
	    const char *img_path;
	    int tileheight, tilewidth, tilecount, columns, spacing, margin;
	    Entity *texture;
	    int isUpLayer = 0;

	    tileheight = yeGetIntAt(tmp, "tileheight");
	    tilewidth = yeGetIntAt(tmp, "tilewidth");
	    spacing = yeGetIntAt(tmp, "spacing");
	    margin = yeGetIntAt(tmp, "margin");
	    columns = yeGetIntAt(tmp, "columns");
	    tilecount = yeGetIntAt(tmp, "tilecount");
	    img_path = yeGetString(yeGet(tmp, "image"));
	    Entity *src_rect = ywRectCreateInts(0, 0, tilewidth,
						tileheight, NULL, NULL);

	    if (assetsPath) {
		int buf_size = strlen(assetsPath) + strlen(img_path) + 2;
		char buf[2048]; /* VLA */

		snprintf(buf, buf_size, "%s/%s", assetsPath, img_path);
		texture = ywTextureNewImg(buf, NULL, tileset, "_texture");
	    } else {
		texture = ywTextureNewImg(img_path, NULL, tileset, "_texture");
	    }

	    int i = 0;
	    int y = yeGetIntAt(layer, "x");
	    int x = yeGetIntAt(layer, "y");

	    YE_ARRAY_FOREACH_EXT(properties, property, it) {
		int isAyyay = yeType(property) == YARRAY;
		const char *name;
		name = yBlockArrayIteratorGetPtr(it, ArrayEntry)->name;
		if (isAyyay) {
		    name = yeGetStringAt(property, "name");
		}
		if (!name)
		    continue;
		if (upCanvas && !strcmp(name, "upLayer")) {
		    isUpLayer = 1;
		} else if (!strcmp(name, "Condition")) {
		    Entity *condition = yeCreateArray(NULL, NULL);
		    int ret;

		    if (!isAyyay) {
			yeCreateString(yeGetString(property), condition, NULL);
			yePushBack(condition, yeGet(properties, "ConditionArg0"),
				   NULL);
			yePushBack(condition, yeGet(properties, "ConditionArg1"),
				   NULL);
		    } else {
			yeCreateString(yeGetStringAt(property, "value"),
				       condition, NULL);
			YE_FOREACH(properties, ca0) {
			    if (!strcmp(yeGetStringAt(ca0, "name"),
					"ConditionArg0")) {
				yePushBack(condition, yeGet(ca0, "value"),
					   NULL);
			    }
			}
			YE_FOREACH(properties, ca1) {
			    if (!strcmp(yeGetStringAt(ca1, "name"),
					"ConditionArg1")) {
				yePushBack(condition, yeGet(ca1, "value"),
					   NULL);
			    }
			}

		    }
		    ret = yeCheckCondition(condition);
		    yeDestroy(condition);
		    if (!ret)
			goto next_tileset;
		}
	    }

	    YE_ARRAY_FOREACH(layer_data, tile_id) {
		uint64_t orig_tid = yeGetInt(tile_id);
		uint64_t tid;
		uint32_t flags = 0;
		Entity *cur_img;

		if (i && !(i % (layer_w))) {
		    y += tileheight;
		    x = 0;
		}
		if (orig_tid >= tilecount) {
		    /* some transformation happen */
		    flags = orig_tid &
			(FLIPPED_HORIZONTALLY_FLAG |
			 FLIPPED_VERTICALLY_FLAG |
			 FLIPPED_DIAGONALLY_FLAG);
		    orig_tid &= ~(FLIPPED_HORIZONTALLY_FLAG |
				  FLIPPED_VERTICALLY_FLAG |
				  FLIPPED_DIAGONALLY_FLAG);
		}
		tid = orig_tid - firstgid;
		if (orig_tid < firstgid || tid >= tilecount)
		    goto next_tile;
		ywRectSetX(src_rect, margin + ((tilewidth + spacing) *
					       (tid % columns)));
		ywRectSetY(src_rect, margin + ((tileheight + spacing) *
					       (tid / columns)));
		if (isUpLayer) {
		    cur_img = ywCanvasNewImgFromTexture(upCanvas, x, y,
							texture, src_rect);
		} else if (main_flags & TILED_MERGE_LAYER_0 && !layer_cnt) {
			yeAutoFree Entity *dst_rect =
				ywRectCreateEnt(src_rect, NULL, NULL);

			ywRectSetX(dst_rect, x);
			ywRectSetY(dst_rect, y);
			ywCanvasMergeTexture(canvas, texture, src_rect, dst_rect);
		} else {
		    Entity *resource = yeGet(resources, orig_tid);
		    if (!resource) {
			resource = yeCreateArrayAt(resources, NULL, orig_tid);
			yePushBack(resource, texture, "texture");
			yeCreateCopy(src_rect, resource, "img-src-rect");
		    }
		    cur_img = ywCanvasNewObj(canvas, x, y, orig_tid);
		}
		if (flags == (FLIPPED_VERTICALLY_FLAG |
			      FLIPPED_HORIZONTALLY_FLAG)) {
		    ywCanvasRotate(cur_img, 180);
		}
		handle_properties(properties, proTypes, cur_img);

	      next_tile:
		x += tilewidth;
		++i;
	    }
	  next_tileset:
	    yeDestroy(src_rect);
	}
    }
    yeReplaceBack(canvas, tiledEnt, "tiled-ent");
    ret = (void *)1;
  exit:
    yeDestroy(tiledEnt);
    return ret;
}

void *init_tiled(int nbArg, void **args)
{
    Entity *t = YE_TO_ENTITY(args[0]);

    yeCreateFunction("setAssetPath", ygGetManager("tcc"), t, "setAssetPath");
    yeCreateFunction("deinit", ygGetManager("tcc"), t, "deinit");
    yeCreateFunction("fileToCanvas", ygGetManager("tcc"), t, "fileToCanvas");
    return NULL;
}
