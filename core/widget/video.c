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

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include "glib.h"
#include "widget.h"

static int t = -1;

struct YVideoState {
	YWidgetState sate;
	AVFormatContext *pFormatCtx;
};

static int videoInit(YWidgetState *opac, Entity *entity, void *args)
{
	struct YVideoState *s = (void *)opac;
	const char *v = yeGetStringAt(entity, "video");

	if (!v) {
		DPRINT_ERR("no video set");
		return -1;
	}
	printf("init video '%s'!!!\n", v);

	s->pFormatCtx = NULL;
	if (avformat_open_input(&s->pFormatCtx, v, NULL, NULL) < 0) {
		DPRINT_ERR("Could not open file %s\n", v);
		return -1;
	}

	if (avformat_find_stream_info(s->pFormatCtx, NULL) < 0) {
		DPRINT_ERR("Could not find stream information %s\n", v);
		return -1;
	}

	// print stuff
	av_dump_format(s->pFormatCtx, 0, v, 0);

	int videoStream = -1;
	AVCodecContext * pCodecCtx = NULL;

	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoStream = i;
			break;
		}
	}

	if (videoStream == -1)
	{
		printf("Could not find video stream.");
		return -1;
	}

	pCodec = avcodec_find_decoder(pFormatCtx->streams[videoStream]->codecpar->codec_id);
	if (pCodec == NULL)
	{
		printf("Unsupported codec!\n");
		return -1;
	}

	pCodecCtx = avcodec_alloc_context3(pCodec);
	ret = avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoStream]->codecpar);
	if (ret != 0)
	{
		printf("Could not copy codec context.\n");
		return -1;
	}

	ret = avcodec_open2(pCodecCtx, pCodec, NULL);
	if (ret < 0)
	{
		printf("Could not open codec.\n");
		return -1;
	}

	return 0;
}

static int rend(YWidgetState *opac)
{
	ywidGenericRend(opac, t, render);
	return 0;
}

static int destroy(YWidgetState *opac)
{
	struct YVideoState *s = (void *)opac;
	avformat_close_input(&s->pFormatCtx);
	return ywDestroyState(opac);
}

static void *alloc(void)
{
	struct YVideoState *ret = g_new0(struct YVideoState, 1);
	YWidgetState *wstate = (YWidgetState *)ret;

	if (!ret)
		return NULL;
	wstate->render = rend;
	wstate->init = videoInit;
	wstate->destroy = destroy;
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

int ywVideoEnd(void)
{
	if (ywidUnregiste(t) < 0)
		return -1;
	t = -1;
	return 0;
}
