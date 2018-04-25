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

#include <yirl/game.h>
#include <yirl/texture.h>
#include <yirl/canvas.h>

const unsigned FLIPPED_HORIZONTALLY_FLAG = 0x80000000;
const unsigned FLIPPED_VERTICALLY_FLAG   = 0x40000000;
const unsigned FLIPPED_DIAGONALLY_FLAG   = 0x20000000;

char *assetsPath;

void *deinit(int nbArg, void **args)
{
  if (assetsPath) {
    free(assetsPath);
    assetsPath = NULL;
  }
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
}

void *fileToCanvas(int nbArg, void **args)
{
  const char *path = args[0];
  Entity *canvas = args[1];
  Entity *tiledEnt;
  Entity *tileset_array;
  void *ret = NULL;
  Entity *layers;

  if (nbArg < 2) {
    DPRINT_ERR("wrong number of arguments:"
	       "should be fileToCanvas(path, canvas)");
    return NULL;
  }
  printf("tile to canvas !, %s %p\n", args[0], args[1]);
  tiledEnt = ygFileToEnt(YJSON, path, NULL);
  tileset_array = yeGet(tiledEnt, "tilesets");
  layers = yeGet(tiledEnt, "layers");
  YE_ARRAY_FOREACH(layers, layer) {
    Entity *layer_data  = yeGet(layer, "data");
    int data_len = yeLen(layer_data);
    int layer_w  = yeGetIntAt(layer, "width");
    Entity *properties = yeGet(layer, "properties");
    Entity *objects = yeGet(layer, "objects");

    if (objects)
      yeTryCreateArray(canvas, "objects");

    YE_ARRAY_FOREACH(tileset_array, tileset) {
      Entity *tmp;
      int firstgid = yeGetIntAt(tileset, "firstgid");
      const char *tmp_path = yeGetString(yeGet(tileset, "source"));
      const char *img_path;
      int tileheight, tilewidth, tilecount, columns, spacing, margin;
      Entity *texture;

      if (assetsPath) {
	/* strings length + 1 for \0 and +1 for '\' */
	int bux_size = strlen(assetsPath) + strlen(tmp_path) + 2;
	char buf[1024]; /* VLA */

	snprintf(buf, bux_size, "%s/%s", assetsPath, tmp_path);
	tmp = ygFileToEnt(YJSON, buf, tileset);
      } else {
	tmp = ygFileToEnt(YJSON, tmp_path, tileset);
      }
      if (!tmp) {
	DPRINT_ERR("can't load '%s'\n", tmp_path);
	goto exit;
      }
      tileheight = yeGetIntAt(tmp, "tileheight");
      tilewidth = yeGetIntAt(tmp, "tilewidth");
      spacing = yeGetIntAt(tmp, "spacing");
      margin = yeGetIntAt(tmp, "margin");
      columns = yeGetIntAt(tmp, "columns");
      tilecount = yeGetIntAt(tmp, "tilecount");
      printf("%d %d %d %d %d %d\n", tileheight, tilewidth,
	     tilecount, columns, spacing, margin);
      yeRenamePtrStr(tileset, tmp, "_ent");
      img_path = yeGetString(yeGet(tmp, "image"));
      Entity *src_rect = ywRectCreateInts(0, 0, tilewidth,
					  tileheight, NULL, NULL);

      if (assetsPath) {
	int bux_size = strlen(assetsPath) + strlen(img_path) + 2;
	char buf[1024]; /* VLA */

	snprintf(buf, bux_size, "%s/%s", assetsPath, img_path);
	texture = ywTextureNewImg(buf, NULL, tileset, "_texture");
      } else {
	texture = ywTextureNewImg(img_path, NULL, tileset, "_texture");
      }
      printf("texture %p %p\n", texture, yeGet(tileset, "_texture"));

      YE_ARRAY_FOREACH(objects, object) {
	Entity *obj = yeCreateArray(yeGet(canvas, "objects"), NULL);
	yeGetPush(object, obj, "visible");
	yeGetPush(object, obj, "id");
	yeGetPush(object, obj, "name");
	ywRectCreateInts(yeGetIntAt(object, "x"),
			 yeGetIntAt(object, "y"),
			 yeGetIntAt(object, "width"),
			 yeGetIntAt(object, "height"),
			 obj, "rect");
      }

      int i = 0;
      int y = yeGetIntAt(layer, "x");
      int x = yeGetIntAt(layer, "y");

      YE_ARRAY_FOREACH(layer_data, tile_id) {
	uint64_t orig_tid = yeGetInt(tile_id);
	uint64_t tid = orig_tid - firstgid;
	uint32_t flags = 0;
	Entity *cur_img;

	if (i && !(i % (layer_w))) {
	  y += tileheight;
	  x = 0;
	}
	//printf("%u(%x, %d,%d) ", yeGetInt(tile_id), yeGetInt(tile_id), x, y);
	if (tid >= tilecount) {
	  /* some transformation happen */
	  flags = tid & (FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG |
			 FLIPPED_DIAGONALLY_FLAG);
	  tid &= ~(FLIPPED_HORIZONTALLY_FLAG | FLIPPED_VERTICALLY_FLAG |
		   FLIPPED_DIAGONALLY_FLAG);
	}
	if (orig_tid < firstgid || tid > 1024)
	  goto next_tile;
	ywRectSetX(src_rect, margin + ((tilewidth + spacing) * (tid % columns)));
	ywRectSetY(src_rect, margin + ((tileheight + spacing) * (tid / columns)));
	cur_img = ywCanvasNewImgFromTexture(canvas, x, y, texture, src_rect);
	if (flags == (FLIPPED_VERTICALLY_FLAG | FLIPPED_HORIZONTALLY_FLAG)) {
	  ywCanvasRotate(cur_img, 180);
	}
	Entity *proTypes = yeGet(layer, "propertytypes");
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
	  if (!yeStrCmp(proType, "int")) {
	    printf("property: %s - %d\n", name,
		   yeGetIntAt(property, "value"));
	    yeCreateInt(yeGetInt(property), cur_img,
			name);
	  } else if (!yeStrCmp(proType, "string")) {
	    yeCreateString(yeGetString(property), cur_img,
			   name);
	  } else if (!yeStrCmp(proType, "float")) {
	    yeCreateFloat(yeGetFloat(property), cur_img,
			  name);
	  }
	}
      next_tile:
	x += tilewidth;
	++i;
      }
      yeDestroy(src_rect);
    }
  }
  yePushBack(canvas, tiledEnt, "tiled-ent");
  ret = (void *)1;
 exit:
  printf("out !!!!\n");
  yeDestroy(tiledEnt);
  return ret;
}

void *init_tiled(int nbArg, void **args)
{
  printf("hi\n");
  Entity *t = YE_TO_ENTITY(args[0]);

  printf("hi\n");
  yeCreateFunction("setAssetPath", ygGetManager("tcc"), t, "setAssetPath");
  yeCreateFunction("deinit", ygGetManager("tcc"), t, "deinit");
  yeCreateFunction("fileToCanvas", ygGetManager("tcc"), t, "fileToCanvas");
  return NULL;
}
