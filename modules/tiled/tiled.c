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

  if (nbArg < 2) {
    DPRINT_ERR("wring number of arguments:"
	       "should be fileToCanvas(path, canvas)");
  }
  printf("tile to canvas !, %s %p\n", args[0], args[1]);
  tiledEnt = ygFileToEnt(YJSON, path, NULL);
  tileset_array = yeGet(tiledEnt, "tilesets");
  YE_ARRAY_FOREACH(tileset_array, tileset) {
    Entity *tmp;
    const char *tmp_path = yeGetString(yeGet(tileset, "source"));
    const char *img_path;
    int tileheight, tilewidth, tilecount, columns, spacing, margin;
    Entity *layers = yeGet(tiledEnt, "layers");
    Entity *texture;

    if (assetsPath) {
      /* strings length + 1 for \0 and +1 for '\' */
      int bux_size = strlen(assetsPath) + strlen(tmp_path) + 2;
      char buf[bux_size]; /* VLA */

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
    if (assetsPath) {
      int bux_size = strlen(assetsPath) + strlen(img_path) + 2;
      char buf[bux_size]; /* VLA */

      snprintf(buf, bux_size, "%s/%s", assetsPath, img_path);
      texture = ywTextureNewImg(buf, NULL, tileset, "_texture");
    } else {
      texture = ywTextureNewImg(img_path, NULL, tileset, "_texture");
    }
    printf("texture %p %p\n", texture, yeGet(tileset, "_texture"));
    YE_ARRAY_FOREACH(layers, layer) {
      Entity *layer_data  = yeGet(layer, "data");
      int data_len = yeLen(layer_data);
      int layer_w  = yeGetIntAt(layer, "width");
      int i = 0;
      int y_tild = margin, y = yeGetIntAt(layer, "x");
      int x_tild = margin, x = yeGetIntAt(layer, "y");
      Entity *src_rect = ywRectCreateInts(0, 0, tilewidth,
					  tileheight, NULL, NULL);

      YE_ARRAY_FOREACH(layer_data, tile_id) {
	int tid = yeGetInt(tile_id) - 1;

	if (i && !(i % (layer_w))) {
	  y += tileheight;
	  x = 0;
	}
	printf("%d(%d,%d) ", yeGetInt(tile_id), x, y);
	if (tid >= tilecount)
	  goto next; /* some transformation happen */
	ywRectSetX(src_rect, margin + ((tilewidth + spacing) * (tid % columns)));
	ywRectSetY(src_rect, margin + ((tileheight + spacing) * (tid / columns)));
	ywCanvasNewImgFromTexture(canvas, x, y, texture, src_rect);
	next:
	x += tilewidth;
	++i;
      }
      yeDestroy(src_rect);
      printf("\n");
      printf("%d\n", yeLen(layer_data));
    }
  }
  yePushBack(canvas, tiledEnt, "tiled-ent");
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
