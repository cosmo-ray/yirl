#include <yirl/all.h>

#define YIRL_0_MODE 0
#define NES_MODE 1
#define ATARI_MODE 2

#define CARY_FLAG 1
#define ZERO_FLAG (1 << 1)
#define IRQ_DISABLE_FLAG (1 << 2)
#define DECIMAL_FLAG (1 << 3)
#define OVERFLOW_FLAG (1 << 6)
#define NEGATIVE_FLAG (1 << 7)

#define SET_ZERO(val) cpu.flag = ((cpu.flag & 0xfd) ^ ((val) << 1))
#define SET_CARY(val) cpu.flag = ((cpu.flag & 0xfe) ^ (val))
#define SET_OVERFLOW(val) cpu.flag = ((cpu.flag & 0xBf) ^ ((val) << 6))
#define SET_NEGATIVE(val) cpu.flag = ((cpu.flag & 0x7f) ^ ((val) << 7))

enum  {
	RUN_MODE = -1,
	DEBUG_MODE = 0
};

static struct cpu {
	union {
		unsigned char a;
		signed char sa;
	};
	unsigned char x;
	unsigned char y;
	unsigned char s;
	unsigned char flag;
	int16_t pc;
	int64_t cycle_cnt;
} cpu = {.s = 0xff, .pc = 0xC000};

static int breakpoints[64];
static int breakpoints_cnt;

enum {
	VSYNC       , //    ; $00   0000 00x0   Vertical Sync Set-Clear
	VBLANK      , //    ; $01   xx00 00x0   Vertical Blank Set-Clear
	WSYNC       , //    ; $02   ---- ----   Wait for Horizontal Blank
	RSYNC       , //    ; $03   ---- ----   Reset Horizontal Sync Counter
	NUSIZ0      , //    ; $04   00xx 0xxx   Number-Size player/missle 0
	NUSIZ1      , //    ; $05   00xx 0xxx   Number-Size player/missle 1
	COLUP0      , //    ; $06   xxxx xxx0   Color-Luminance Player 0
	COLUP1      , //    ; $07   xxxx xxx0   Color-Luminance Player 1
	COLUPF      , //    ; $08   xxxx xxx0   Color-Luminance Playfield
	COLUBK      , //    ; $09   xxxx xxx0   Color-Luminance Background
	CTRLPF      , //    ; $0A   00xx 0xxx   Control Playfield, Ball, Collisions
	REFP0       , //    ; $0B   0000 x000   Reflection Player 0
	REFP1       , //    ; $0C   0000 x000   Reflection Player 1
	PF0         , //    ; $0D   xxxx 0000   Playfield Register Byte 0
	PF1         , //    ; $0E   xxxx xxxx   Playfield Register Byte 1
	PF2         , //    ; $0F   xxxx xxxx   Playfield Register Byte 2
	RESP0       , //    ; $10   ---- ----   Reset Player 0
	RESP1       , //    ; $11   ---- ----   Reset Player 1
	RESM0       , //    ; $12   ---- ----   Reset Missle 0
	RESM1       , //    ; $13   ---- ----   Reset Missle 1
	RESBL       , //    ; $14   ---- ----   Reset Ball
	AUDC0       , //    ; $15   0000 xxxx   Audio Control 0
	AUDC1       , //    ; $16   0000 xxxx   Audio Control 1
	AUDF0       , //    ; $17   000x xxxx   Audio Frequency 0
	AUDF1       , //    ; $18   000x xxxx   Audio Frequency 1
	AUDV0       , //    ; $19   0000 xxxx   Audio Volume 0
	AUDV1       , //    ; $1A   0000 xxxx   Audio Volume 1
	GRP0        , //    ; $1B   xxxx xxxx   Graphics Register Player 0
	GRP1        , //    ; $1C   xxxx xxxx   Graphics Register Player 1
	ENAM0       , //    ; $1D   0000 00x0   Graphics Enable Missle 0
	ENAM1       , //    ; $1E   0000 00x0   Graphics Enable Missle 1
	ENABL       , //    ; $1F   0000 00x0   Graphics Enable Ball
	HMP0        , //    ; $20   xxxx 0000   Horizontal Motion Player 0
	HMP1        , //    ; $21   xxxx 0000   Horizontal Motion Player 1
	HMM0        , //    ; $22   xxxx 0000   Horizontal Motion Missle 0
	HMM1        , //    ; $23   xxxx 0000   Horizontal Motion Missle 1
	HMBL        , //    ; $24   xxxx 0000   Horizontal Motion Ball
	VDELP0      , //    ; $25   0000 000x   Vertical Delay Player 0
	VDELP1      , //    ; $26   0000 000x   Vertical Delay Player 1
	VDELBL      , //    ; $27   0000 000x   Vertical Delay Ball
	RESMP0      , //    ; $28   0000 00x0   Reset Missle 0 to Player 0
	RESMP1      , //    ; $29   0000 00x0   Reset Missle 1 to Player 1
	HMOVE       , //    ; $2A   ---- ----   Apply Horizontal Motion
	HMCLR       , //    ; $2B   ---- ----   Clear Horizontal Move Registers
	CXCLR       , //    ; $2C   ---- ----   Clear Collision Latches
};

enum {
	SWCHA  = 0x280, //      Port A data register for joysticks:
			//      Bits 4-7 for player 1.  Bits 0-3 for player 2.
	SWACNT = 0x281, //      Port A data direction register (DDR)
	SWCHB = 0x282,  //      Port B data (console switches)
	SWBCNT = 0x283, //      Port B DDR
	INTIM = 0x284,  //      Timer output
	TIMINT = 0x285, //

	TIM1T = 0x294,  //      set 1 clock interval
	TIM8T = 0x295,  //      set 8 clock interval
	TIM64T = 0x296, //      set 64 clock interval
	T1024T = 0x297  //      set 1024 clock interval
};

#define CYCLE_PER_FRAME 19912

// #define CYCLE_PER_SCANE_LINE 104
#define CYCLE_PER_SCANE_LINE 76

#define ATARI_SCREEN_H 262
#define ATARI_SCREEN_W 160

#define ATARI_SCREEN_THRESHOLD_Y 20

/* it seems every 104 cycle I need to update v_line */
/* another souce seems to say 76 cycles */
static struct tia {
	uint8_t col_p[2];
	uint8_t gr_p0;
	uint8_t gr_p1;
	uint8_t col_bg;
	uint8_t col_playfield;
	uint8_t vblank_mode;
	uint8_t nusiz[2];
 	uint8_t ball_p;
	uint8_t enabl;
	uint8_t enam0;
	uint8_t enam1;
	uint8_t ctrlpf;
	uint8_t pf[3];
	uint8_t hmp[2];
	uint8_t hmm[2];
	uint8_t hmbl;
	uint8_t hmove_p[2];
	uint8_t hmove_m[2];
	uint8_t hmove_bl;
 	uint8_t players_px[2];
 	uint8_t missils_px[2];
	uint64_t vsync_cycle;
} tia;

static int64_t old_sl_cycle;

static struct riot {
	uint64_t timer_cycle_start;
	uint16_t timer_l;
	uint8_t timer;
	uint8_t direction_press;
	uint8_t p_fire_press[2];
} riot = {
	.direction_press = 0xFF,
};

/**
 * NES PPU:
 * PPUCTRL 	$2000 	VPHB SINN 	NMI enable (V), PPU master/slave (P), sprite height (H),
 *                                      background tile select (B), sprite tile select (S),
 *				        increment mode (I), nametable select (NN)
 * PPUMASK 	$2001 	BGRs bMmG 	color emphasis (BGR), sprite enable (s),
 *					background enable (b), sprite left column enable (M),
 *					background left column enable (m), greyscale (G)
 * PPUSTATUS 	$2002 	VSO- ---- 	vblank (V), sprite 0 hit (S), sprite overflow (O);
 *					read resets write pair for $2005/$2006
 * OAMADDR 	$2003 	aaaa aaaa 	OAM read/write address
 * OAMDATA 	$2004 	dddd dddd 	OAM data read/write
 * PPUSCROLL 	$2005 	xxxx xxxx 	fine scroll position (two writes: X scroll, Y scroll)
 * PPUADDR 	$2006 	aaaa aaaa 	PPU read/write address
 *					(two writes: most significant byte, least significant byte)
 * PPUDATA 	$2007 	dddd dddd 	PPU data read/write
 * OAMDMA 	$4014 	aaaa aaaa 	OAM DMA high address
 */
static struct ppu {
	union {
		int16_t pc;
		struct {
			char low_pc;
			char high_pc;
		};
	};
	int16_t sprite_loc;
	unsigned char not_vblank;
} ppu;

static unsigned char ppu_mem[0x4000];

static unsigned char ram[0x1000];
static unsigned char miror0[0x800];
static unsigned char miror1[0x800];
static unsigned char miror2[0x800];
struct {
	union {
		unsigned char buf[8];
	};
} PPU_Regs;
unsigned char ppu_mirror[0x1FF8];

struct {
	union {
		unsigned char buf[0x18];
	};
} APU_IP_Regs;
unsigned char unused_apu[8];

unsigned char *cartridge;

#define CHR_ROM 0x4010

enum opcode {
#define OPCODE(opcode, num) opcode=num,
#include "opcode.h"
	UNDEFINE=0xff
};
#undef OPCODE

static char *opcode_str[0x100];

static Entity *main_canvas;
static Entity *colors_json;

static int turn_mode;

int (*set_mem)(uint16_t addr, char val);
unsigned char (*get_mem)(uint16_t addr);
int current_emu_mode;

int wid_width;
int wid_height;

int set_mem_yirl(uint16_t addr, char val)
{
	if (addr < 0x100) {
		cartridge[addr] = val;
	} else if (addr < 0x1000) {
		ram[addr] = val;
	} else if (addr < 0xc000) {
		cartridge[addr] = val;
	} else if (addr >= 0xff00) {
		ppu_mem[addr & 0xff] = val;
	} else if (addr >= 0xfc00) {
		switch (addr & 0xff) {
		case 0:
		{
			int w = ywWidth(main_canvas);
			int h = ywWidth(main_canvas);
			int addr = ppu_mem[0] | (ppu_mem[1] << 8);
			char *s = &cartridge[addr];
			int x = cpu.x * w / 255;
			int y = cpu.y * w / 255;

			printf("write txt: %x/%s at: %d - %d\n", addr, s, cpu.x, cpu.y);
			Entity *ret = ywCanvasNewTextByStr(main_canvas, x, y, s);
			ywCanvasSetStrColor(ret, "rgba: 255 255 255 255");
			yePrint(ret);
			break;
		}
		default:
			printf("unknow call\n");
		}
	}
	printf("set_mem_yirl: at %x val: %x\n", addr, val);
	return 0;
}

/**
 *   HDR:    start = $0000,  size = $1000, type = ro, file = %O, fill = yes, fillval = $00;
 *   RAM:    start = $0100,  size = $0900, type = rw, file = "", fill = yes, fillval = $ff;
 *   PRG:    start = $1000,  size = $8000, type = ro, file = %O, fill = yes, fillval = $00;
 *   DTA:    start = $9000,  size = $7000, type = ro, file = %O, fill = yes, fillval = $00;
 */
unsigned char get_mem_yirl(uint16_t addr)
{
	printf("get_mem_yirl at %d - %x\n", addr, addr);
	if (addr < 0x100) {
		return cartridge[addr];
	} else if (addr < 0x1000) {
		return ram[addr];
	} else if (addr < 0xc000) {
		return cartridge[addr];
	} else if (addr >= 0xff00) {
		return ppu_mem[addr & 0xff];
	}
	return 0;
}

static uint8_t atari_current_col(void)
{
	int line_cycle = cpu.cycle_cnt % CYCLE_PER_SCANE_LINE;
	int color_blocks = line_cycle * 3;
	if (color_blocks < 68) {
		return 161; // out of screen
	}
	int pixel_pos = color_blocks - 68;
	return pixel_pos + 4;
}

static int atari_curent_scane_line(void)
{
	return ((cpu.cycle_cnt - tia.vsync_cycle) / CYCLE_PER_SCANE_LINE) % ATARI_SCREEN_H;
}


static char *atari_get_color(unsigned int idx)
{
	return yeGetStringAt(yeGet(colors_json, (idx >> 4)), (idx & 0xf) / 2);
}

//yeGet(colors_json, atari_color_idx(tia.col_bg))


void arati_print_player_pixel(int p, int x, int i, int width,
			      int pix_per_pix_x, int pix_per_pix_y,
			      int line)
{
	ywCanvasMergeRectangle(main_canvas,
			       x * pix_per_pix_x + i * width,
			       ATARI_SCREEN_THRESHOLD_Y + (line - 40) * pix_per_pix_y,
			       width, pix_per_pix_y,
			       atari_get_color(tia.col_p[p]));

}

void atari_show_player(int p, int val, int cur)
{
	/* | Bits (2–0) | Copies | Spacing (TIA pixels) | Notes              | */
	/* | ---------- | ------ | -------------------- | ------------------ | */
	/* | `000`      | 1      | —                    | Single copy        | */
	/* | `001`      | 2      | 16                   | Close double       | */
	/* | `010`      | 2      | 32                   | Medium double      | */
	/* | `011`      | 3      | 16                   | Triple, tight      | */
	/* | `100`      | 2      | 64                   | Wide double        | */
	/* | `101`      | 1      | —                    | Double-size player | */
	/* | `110`      | 3      | 32                   | Triple, spaced     | */
	/* | `111`      | 1      | —                    | Quad-size player   | */

	int pix_per_pix_x = wid_width / 160;
	int pix_per_pix_y = wid_height / 192;
	int width = pix_per_pix_x;
	_Bool space_16 = 0;
	_Bool space_32 = 0;
	_Bool space_64 = 0;
	int x = tia.players_px[p];
	if (tia.hmove_p[p]) {
		int fine_adjuste = tia.hmove_p[p] >> 4;

		if (fine_adjuste < 8) {
			x -= fine_adjuste;
		} else {
			x += (0xf - fine_adjuste + 1);
		}
	}

	switch (tia.nusiz[p]) {
	case 3:
		space_32 = 1;
	case 1:
		space_16 = 1;
		break;
	case 2:
		space_32 = 1;
		break;
	case 4:
		space_64 = 1;
		break;
	case 5:
		width *= 2;
		break;
	case 6:
		space_32 = 1;
		space_64 = 1;
		break;
	case 7:
		width *= 4;
		break;
	default:
		break;
	}
	if (p == 0 && !tia.gr_p0)
		goto skipp_player;
	if (p == 1 && !tia.gr_p1)
		goto skipp_player;
	for (int i = 0; i < 8; ++i) {
		int pix_val = !!(val & (1 << i));
		if (!pix_val)
			continue;
		arati_print_player_pixel(p, x, i, width, pix_per_pix_x, pix_per_pix_y, cur);
		if (space_16) {
			arati_print_player_pixel(p, x + 16, i, width, pix_per_pix_x, pix_per_pix_y, cur);
		}
		if (space_32) {
			arati_print_player_pixel(p, x + 32, i, width, pix_per_pix_x, pix_per_pix_y, cur);
		}
		if (space_64) {
			arati_print_player_pixel(p, x + 64, i, width, pix_per_pix_x, pix_per_pix_y, cur);
		}
	}
skipp_player:
	if (p == 0 && !tia.enam0)
		return;
	if (p == 1 && !tia.enam1)
		return;
	ywCanvasMergeRectangle(main_canvas,
			       tia.missils_px[p] * pix_per_pix_x,
			       ATARI_SCREEN_THRESHOLD_Y + (cur - 40) * pix_per_pix_y,
			       pix_per_pix_x, pix_per_pix_y,
			       atari_get_color(tia.col_p[p]));
	if (space_16) {
		ywCanvasMergeRectangle(main_canvas,
				       (tia.missils_px[p] + 16) * pix_per_pix_x,
				       ATARI_SCREEN_THRESHOLD_Y + (cur - 40) * pix_per_pix_y,
				       pix_per_pix_x, pix_per_pix_y,
				       atari_get_color(tia.col_p[p]));
	}

	if (space_32) {
		ywCanvasMergeRectangle(main_canvas,
				       (tia.missils_px[p] + 32) * pix_per_pix_x,
				       ATARI_SCREEN_THRESHOLD_Y + (cur - 40) * pix_per_pix_y,
				       pix_per_pix_x, pix_per_pix_y,
				       atari_get_color(tia.col_p[p]));
	}

	if (space_64) {
		ywCanvasMergeRectangle(main_canvas,
				       (tia.missils_px[p] + 64) * pix_per_pix_x,
				       ATARI_SCREEN_THRESHOLD_Y + (cur - 40) * pix_per_pix_y,
				       pix_per_pix_x, pix_per_pix_y,
				       atari_get_color(tia.col_p[p]));
	}
}

static void atari_pf_show_block(int pix_per_pix_x, int pix_per_pix_y, int cur, int v, int pf_add, int bcheck)
{
	int bit = bcheck & v;
	if (bit) {
		uint8_t col_p0 = (tia.ctrlpf & 0x02) ? tia.col_p[0] : tia.col_playfield;
		uint8_t col_p1 = (tia.ctrlpf & 0x02) ? tia.col_p[1] : tia.col_playfield;
		ywCanvasMergeRectangle(main_canvas, pf_add * pix_per_pix_x,
				       ATARI_SCREEN_THRESHOLD_Y + (cur - 40) * pix_per_pix_y,
				       pix_per_pix_x * 4, pix_per_pix_y,
				       atari_get_color(col_p0));
		ywCanvasMergeRectangle(main_canvas, 160 * pix_per_pix_x - pf_add * pix_per_pix_x - 4 * pix_per_pix_x,
				       ATARI_SCREEN_THRESHOLD_Y + (cur - 40) * pix_per_pix_y,
				       pix_per_pix_x * 4, pix_per_pix_y,
				       atari_get_color(col_p1));

	}
}

static void atari_pf(int pix_per_pix_x, int pix_per_pix_y, int cur)
{
	int pf_add = 0;
	if (tia.pf[0] || tia.pf[1] || tia.pf[2]) {
		unsigned int v = tia.pf[0] & 0xf0;

		while (v) {
			atari_pf_show_block(pix_per_pix_x, pix_per_pix_y, cur, v, pf_add, 0x10);
			pf_add += 4;
			v = v >> 1;
			v &= 0xf0;
		}
		v = tia.pf[1];
		pf_add = 4 * 4;
		while (v) {
			atari_pf_show_block(pix_per_pix_x, pix_per_pix_y, cur, v, pf_add, 0x80);
			pf_add += 4;
			v = v << 1;
			v &= 0xff;
		}
		v = tia.pf[2];
		pf_add = 12 * 4;
		while (v) {
			atari_pf_show_block(pix_per_pix_x, pix_per_pix_y, cur, v, pf_add, 0x01);
			pf_add += 4;
			v = v >> 1;
			v &= 0xff;
		}
	}

}

static void atari_do_scan_line(void)
{
	if (tia.vblank_mode)
		return;
	int cur = atari_curent_scane_line();
	int pix_per_pix_x = wid_width / 160;
	int pix_per_pix_y = wid_height / 192;
	if (cur < 40) {
		/* in blank */
		return;
	}
	if (cur > 232)
		return;
	ywCanvasMergeRectangle(main_canvas, 0,
			       ATARI_SCREEN_THRESHOLD_Y + (cur - 40) * pix_per_pix_y,
			       160 * pix_per_pix_x, pix_per_pix_y,
			       atari_get_color(tia.col_bg));

	if (tia.ctrlpf & 0x04)
		atari_pf(pix_per_pix_x, pix_per_pix_y, cur);


	if (tia.gr_p0 || tia.enam0) {
		atari_show_player(0, tia.gr_p0, cur);
	}
	if (tia.gr_p1 || tia.enam1) {
		atari_show_player(1, tia.gr_p1, cur);
	}
	if (tia.enabl & 0x02) {
		ywCanvasMergeRectangle(main_canvas, tia.ball_p * pix_per_pix_x,
				       ATARI_SCREEN_THRESHOLD_Y + (cur - 40) * pix_per_pix_y,
				       pix_per_pix_x, pix_per_pix_y,
				       atari_get_color(tia.col_playfield));
	}
	if (!(tia.ctrlpf & 0x04))
		atari_pf(pix_per_pix_x, pix_per_pix_y, cur);
}

int set_mem_atari(uint16_t addr, char val)
{
	if (turn_mode == DEBUG_MODE)
		printf("set_mem_atari at %d(%x): %d\n", addr, addr, val);
	if (addr < 0x2c || (addr >= SWCHA && addr <= T1024T)) {
		/**
		 **  262 row total: **
		 * 3 vertical sync (first lines)
		 * 37 vertical blank
		 * 192 visible one
		 * 30 overscan (after the visible one)
		 ** Colums:
		 * 160 pixels
		 */
		switch (addr) {
		case VBLANK:
			tia.vblank_mode = val;
			break;
		case PF0:  //    ; $0D   xxxx 0000   Playfield Register Byte 0
			tia.pf[0] = val;
			break;
		case PF1:  //    ; $0E   xxxx xxxx   Playfield Register Byte 1
			tia.pf[1] = val;
			break;
		case PF2:  //    ; $0F   xxxx xxxx   Playfield Register Byte 2
			tia.pf[2] = val;
			break;
		case CTRLPF:
			tia.ctrlpf = val;
			break;
		case ENABL:
			tia.enabl = val;
			break;
		case RESBL:
			tia.ball_p = atari_current_col();
			break;
		case HMCLR:
			tia.hmp[0] = tia.hmp[1] = 0;
			tia.hmm[0] = tia.hmm[1] = 0;
			tia.hmbl = 0;
			break;
		case HMOVE:
			tia.hmove_p[0] = tia.hmp[0];
			tia.hmove_p[1] = tia.hmp[1];
			tia.hmove_m[0] = tia.hmm[0];
			tia.hmove_m[1] = tia.hmm[1];
			tia.hmove_bl = tia.hmbl;
			break;
		case HMP0:
			tia.hmp[0] = val;
			break;
		case HMP1:
			tia.hmp[1] = val;
			break;
		case HMM0:
			tia.hmm[0] = val;
			break;
		case HMM1:
			tia.hmm[1] = val;
			break;
		case HMBL:
			tia.hmbl = val;
			break;
		case RESP0:
			tia.players_px[0] = atari_current_col();
			break;
		case RESP1:
			tia.players_px[1] = atari_current_col();
			break;
		/* RESMP0 and RESMP1 are oversimplify here,
		 * in theory I must write 2 then 0 after 76 cpu instructions are pass */
		case RESMP0:
			if (val == 2)
				tia.missils_px[0] = tia.players_px[0] + 4;
			return 0;
		case RESMP1:
			if (val == 2)
				tia.missils_px[1] = tia.players_px[1] + 4;
			return 0;
		case NUSIZ0:
			tia.nusiz[0] = val;
			break;
		case TIM1T:
		case TIM8T:
		case TIM64T:
		case T1024T:
			riot.timer = val;
			riot.timer_cycle_start = cpu.cycle_cnt;
			if (addr == TIM1T)
				riot.timer_l = 1;
			else if (addr == TIM8T)
				riot.timer_l = 8;
			else if (addr == TIM64T)
				riot.timer_l = 64;
			else
				riot.timer_l = 1024;
			break;
		case NUSIZ1:
			tia.nusiz[1] = val;
			break;
		case VSYNC:
			/* in theory we should handle other value, and see if enough scanline have pass since 0x2 */
			if (val & 0x2) {
				tia.vsync_cycle = cpu.cycle_cnt;
				old_sl_cycle = 0;
			}
			break;
		case WSYNC:
			cpu.cycle_cnt += CYCLE_PER_SCANE_LINE - cpu.cycle_cnt % CYCLE_PER_SCANE_LINE;
			return 0;
		case GRP0:
		{
			tia.gr_p0 = val;
			return 0;
		}
		case GRP1:
		{
			tia.gr_p1 = val;
			return 0;
		}
		case COLUP0:
			tia.col_p[0] = val;
			return 0;
		case COLUP1:
			tia.col_p[1] = val;
			return 0;
		case COLUBK:
			tia.col_bg = val;
			return 0;
		case COLUPF:
			tia.col_playfield = val;
			return 0;
		case ENAM0:
			tia.enam0 = val;
			return 0;
		case ENAM1:
			tia.enam1 = val;
			return 0;
		default:
		  printf("peripheric write at %x\n", addr);
		}
		/* printf("peripheric write at %x\n", addr); */
		return 0;
	}
	ram[addr] = val;
	return 0;
}

unsigned char get_mem_atari(uint16_t addr)
{
	if (turn_mode == DEBUG_MODE)
		printf("get_mem_atari at %d - %x\n", addr, addr);
	if (addr >= 0xf000) {
		return cartridge[addr - 0xf000];
	} else if (addr >= SWCHA && addr <= T1024T) {
		switch(addr) {
		case INTIM:
		{
			int cycle_pass = cpu.cycle_cnt - riot.timer_cycle_start;
			int timer_pass = cycle_pass / riot.timer_l;
			/* printf("INTIM !\n"); */
			/* printf("%d %d\n", riot.timer_l, riot.timer); */
			/* printf("%d %d %u\n", cycle_pass, timer_pass, riot.timer - timer_pass & 0xff); */
			return (riot.timer - timer_pass) & 0xff;
		}
		case SWCHA:
			return riot.direction_press;
		default:
		}
	} else if (addr > 0x2c) {
		return ram[addr];
	} else {
		/* printf("peripheric read at %x\n", addr); */
	}
	return 0;
}

int set_mem_nes(uint16_t addr, char val)
{
	if (addr < 0x800) {
		printf("set ram\n");
		ram[addr] = val;
	} else if (addr < 0x1000) {
		printf("set mirror 0\n");
		miror0[addr - 0x800] = val;
	} else if (addr < 0x1800) {
		printf("set mirror 1\n");
		miror1[addr - 0x1000] = val;
	} else if (addr < 0x2000) {
		printf("set mirror 2\n");
		miror2[addr - 0x1800] = val;
	} else if (addr < 0x2008) {
		printf("set PPU regs\n");
		if (addr == 0x2006) {
			static unsigned int addr_highgness;

			if (!addr_highgness & 1) {
				ppu.high_pc = val;
			} else {
				ppu.low_pc = val;
			}
			++addr_highgness;
			printf("write ppu dest addr\n");
		} else if (addr == 0x2007) {
			printf("PPU drawing stuff");
			ppu_mem[ppu.pc] = val;
			++ppu.pc;
			ppu.pc &= 0x3fff;
		} else if (addr == 0x2000) {
			if (!val)
				printf("disable MMI ?\n");
		} else if (addr == 0x2001) {
			if (!val)
				printf("disable rendering ?\n");
		} else if (addr == 0x2003) {
			printf("set sprites locations low bytes\n");
			ppu.sprite_loc = (ppu.sprite_loc & 0xff00) | val;
		}
	} else if (addr < 0x4000) {
		if (addr == 0x4014) {
			printf("set sprites locations high bytes\n");
			ppu.sprite_loc = (ppu.sprite_loc & 0x00ff) | val << 8;
		}

		printf("set PPU mirros\n");
	} else if (addr < 0x4018) {
		if (addr == 0x4010) {
			if (!val)
				printf("disable DMC IRQs\n");
		}
		if (addr == 0x4017) {
			if (val == 0x40)
				printf("disable APU ? \n");
			else
				printf("apu write");
		}
		printf("set APU and IO\n");
	} else if (addr < 0x4020) {
		printf("set APU unused ?\n");
	} else if (addr < 0xc000) {
		printf("ROM don't know ?\n");
	} else {
		printf("cannot set Read Only RAM\n");
	}
	return 0;
}

unsigned char get_mem_nes(uint16_t addr)
{
	if (addr < 0x800) {
		printf("ram\n");
		return ram[addr];
	} else if (addr < 0x1000) {
		printf("miror 0\n");
		return miror0[addr - 0x800];
	} else if (addr < 0x1800) {
		printf("mirror 1\n");
		return miror1[addr - 0x1000];
	} else if (addr < 0x2000) {
		printf("mirror 2\n");
		return miror2[addr - 0x1800];
	} else if (addr < 0x2008) {
		if (addr == 0x2002) {
			/* maybe the rendering should be done here */
			unsigned char ret = 0;
			int print_part = ppu.not_vblank;

			if (ppu.not_vblank == 11) {
				printf("we'll print spritex here");
				/* sprites PPU  addr:  */
				ppu.not_vblank = 0;
				ret |= 0x80;
			} else {
				yeAutoFree Entity *chr_data = yeCreateData(&cartridge[CHR_ROM],
									   NULL, NULL);
				yeAutoFree Entity *size = ywSizeCreate(20, 20, 0, 0);
				for (int y = 3 * print_part; y < 3 * print_part + 3; ++y) {
					int sprites_per_lines = 32;

					for (int x = 0; x < 256 / 8; x+= 1) {
						/* load in nametable for now */
						int idx = ppu_mem[0x2000 + y * 32 + x];

						Entity *o = ygCall("load-chr", "ychr_draw_tile",
								   main_canvas,
								   chr_data, (0x10 * 0x10) + idx,
								   (x * 20), (y * 20), 1);
						ywCanvasForceSize(o, size);
						//ychr_draw_tile(main_canvas, chr_data, idx, x, y);
					}
				}
				/* printing here */
				++ppu.not_vblank;
			}
			printf("not vblak: %x - %x\n", ppu.not_vblank, ret);
			return ret;
		} else if (addr == 0x2003) {
			printf("get sprite location low byte ?\n");
		}
		printf("PPU regs\n");
		return 0xff;
	} else if (addr < 0x4000) {
		if (addr == 0x4014) {
			printf("get sprites locations high bytes\n");
		}
		printf("PPU mirros\n");
	} else if (addr < 0x4018) {
		printf("APU and IO\n");
		if (addr == 0x4017)
			return 0x40;
	} else if (addr < 0x4020) {
		printf("APU unused ?\n");
	} else if (addr < 0xc000) {
		printf("ROM don't know ?\n");
	} else {
		return cartridge[addr - 0xc000 + 0x10];
	}
	return 0xff;
}

static int check_new_page(int old, int new)
{
	return (old & 0xff00) != (new & 0xff00);
}

static int process_inst(void)
{
	unsigned char opcode = get_mem(cpu.pc);
	int ret = 0;
	if (turn_mode == DEBUG_MODE)
		printf("code (a: %x, x: %x, y: %x, f: %x): 0x%x: %x - %s\n", cpu.a, cpu.x, cpu.y, cpu.flag,
		       cpu.pc & 0xffff, get_mem(cpu.pc), opcode_str[get_mem(cpu.pc)]);
	if (current_emu_mode == ATARI_MODE) {
		int64_t elapse = cpu.cycle_cnt - old_sl_cycle;
		while (elapse > CYCLE_PER_SCANE_LINE) {
			atari_do_scan_line();
			old_sl_cycle += CYCLE_PER_SCANE_LINE;
			elapse = cpu.cycle_cnt - old_sl_cycle;
		}
	}

	switch (opcode) {
	case NOP:
		cpu.cycle_cnt += 2;
		break;
	case PHP:
	{
		ram[0x100 | cpu.s] = cpu.flag | 0x30;
		--cpu.s;
		cpu.cycle_cnt += 3;
		break;
	}
	case PLA:
	{
		++cpu.s;
		cpu.a = ram[0x100 | cpu.s];
		SET_ZERO(!cpu.a);
		SET_NEGATIVE(!!(0x80 & cpu.a));
		cpu.cycle_cnt += 4;
		break;
	}
	case ROL_a:
	{
		char new_c = cpu.a & 0x1;
		cpu.a = (cpu.a << 1) | cpu.flag & 0x1;
		SET_CARY(new_c);
		SET_NEGATIVE(!!(0x80 & cpu.a));
		SET_ZERO(!cpu.a);
		cpu.cycle_cnt += 2;
	}
	break;
	case ROL_ab:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		int val = get_mem(addr);
		char new_c = val & 0x1;
		val = (val << 1) | cpu.flag & 0x1;
		cpu.cycle_cnt += 6;
		set_mem(addr, val);
		SET_CARY(new_c);
		SET_NEGATIVE(!!(0x80 & val));
		SET_ZERO(!val);
	}
	break;
	case EOR_IM:
	  {
	    unsigned char addr = get_mem(++cpu.pc);

	    cpu.a ^= addr;
	    SET_NEGATIVE(!!(0x80 & cpu.a));
	    SET_ZERO(!cpu.a);
	    cpu.cycle_cnt += 2;
	  }
	  break;
	case EOR_AB:
	  {
	    int addr = get_mem(++cpu.pc);
	    addr |= get_mem(++cpu.pc) << 8;
	    char c = get_mem(addr);

	    cpu.a ^= c;
	    SET_NEGATIVE(!!(0x80 & cpu.a));
	    SET_ZERO(!cpu.a);
	    cpu.cycle_cnt += 4;
	  }
	  break;
	case LSR_A:
		SET_CARY(cpu.a & 1);
		cpu.a = cpu.a >> 1;
		SET_NEGATIVE(0);
		SET_ZERO(!cpu.a);
		cpu.cycle_cnt += 2;
		break;
	case LSR_ab:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		char c = get_mem(addr);
		SET_CARY(c & 1);
		c = c >> 1;
		SET_NEGATIVE(0);
		SET_ZERO(!c);
		cpu.cycle_cnt += 6;
		set_mem(addr, c);
	}
	break;
	case ASL_a:
	{
		SET_CARY(!!(cpu.a & 0x80));
		cpu.a = cpu.a << 1;
		cpu.a &= 0xfe;
		SET_NEGATIVE(!!(0x80 & cpu.a));
		SET_ZERO(!cpu.a);
		cpu.cycle_cnt += 2;
	}
	break;
	case TXA:
		cpu.a = cpu.x;
		SET_NEGATIVE(!!(cpu.a & 0x80));
		SET_ZERO(!cpu.a);
		cpu.cycle_cnt += 2;
		break;
	case TAX:
		cpu.x = cpu.a;
		SET_NEGATIVE(!!(cpu.x & 0x80));
		SET_ZERO(!cpu.x);
		cpu.cycle_cnt += 2;
		break;
	case TAY:
		cpu.y = cpu.a;
		SET_NEGATIVE(!!(cpu.y & 0x80));
		SET_ZERO(!cpu.y);
		cpu.cycle_cnt += 2;
		break;
	case INC_zp:
	case DEC_zp:
	{
		int addr = get_mem(++cpu.pc);
		char val = get_mem(addr);
		char result;
		cpu.cycle_cnt += 5;
		if (opcode == INC_zp)
		  result = val + 1;
		else
		  result = val - 1;
		set_mem(addr, result);
		SET_NEGATIVE(!!(result & 0x80));
		SET_ZERO(!result);
		break;
	}
	case INX:
		cpu.x += 1;
		SET_NEGATIVE(!!(cpu.x & 0x80));
		SET_ZERO(!cpu.x);
		cpu.cycle_cnt += 2;
		break;
	case INY:
		cpu.y += 1;
		SET_NEGATIVE(!!(cpu.y & 0x80));
		SET_ZERO(!cpu.y);
		cpu.cycle_cnt += 2;
		break;
	case DEX:
		cpu.x -= 1;
		SET_NEGATIVE(!!(cpu.x & 0x80));
		SET_ZERO(!cpu.x);
		cpu.cycle_cnt += 2;
		break;
	case DEY:
		cpu.y -= 1;
		SET_NEGATIVE(!!(cpu.y & 0x80));
		SET_ZERO(!cpu.y);
		cpu.cycle_cnt += 2;
		break;
	case CMP_imediate:
	{
		unsigned char addr = get_mem(++cpu.pc);

		SET_ZERO(cpu.a == addr);
		SET_OVERFLOW(addr >= cpu.a);
		SET_NEGATIVE(!!(((signed char)addr - cpu.sa) & 0x70));
		cpu.cycle_cnt += 2;
	}
	break;
	/* cycle cnt here are kinda wrong, as it should +1 on "sucess", and +2 on new page */
	case BCC:
	{
		signed char addr = get_mem(++cpu.pc);

		cpu.cycle_cnt += 2;
		if (!(cpu.flag & CARY_FLAG)) {
			int old_pc = cpu.pc;
			cpu.pc += addr + 1;
			cpu.cycle_cnt += 1 + check_new_page(old_pc, cpu.pc);
			goto out;
		}
	}
	break;
	case BCS:
	{
		signed char addr = get_mem(++cpu.pc);

		cpu.cycle_cnt += 2;
		if (cpu.flag & CARY_FLAG) {
			int old_pc = cpu.pc;
			cpu.pc +=addr + 1;
			cpu.cycle_cnt += 1 + check_new_page(old_pc, cpu.pc);
			goto out;
		}
	}
	break;
	case BEQ:
	{
		signed char addr = get_mem(++cpu.pc);

		cpu.cycle_cnt += 2;
		if (cpu.flag & ZERO_FLAG) {
			int old_pc = cpu.pc;
			cpu.pc +=addr + 1;
			cpu.cycle_cnt += 1 + check_new_page(old_pc, cpu.pc);
			goto out;
		}
	}
	break;
	case TXS:
		cpu.s = cpu.x;
		cpu.cycle_cnt += 2;
		break;
	case AND_im:
	{
		int addr = get_mem(++cpu.pc);
		cpu.a &= addr;
		SET_ZERO(!cpu.a);
		SET_NEGATIVE(!!(cpu.a & 0x80));
		cpu.cycle_cnt += 2;
	}
	break;
	case LDA_im:
	{
		int addr = get_mem(++cpu.pc);

		cpu.a = addr;
		SET_ZERO(!!cpu.a);
		SET_NEGATIVE(!!(cpu.a & 0x80));
		cpu.cycle_cnt += 2;
	}
	break;
	case CLI:
		cpu.flag &= 0xfb; // & 11111011
		cpu.cycle_cnt += 2;
		break;
	case SEI:
		cpu.flag |= IRQ_DISABLE_FLAG; // & 11111011
		cpu.cycle_cnt += 2;
		break;
	case CLD:
		cpu.flag &= 0xf7; // & 11110111
		cpu.cycle_cnt += 2;
		break;
	case CLC:
		cpu.flag &= 0xfe;
		cpu.cycle_cnt += 2;
		break;
	case CLV:
		cpu.flag &= 0xbf;
		cpu.cycle_cnt += 2;
		break;
	case SEC:
		cpu.flag |= 1;
		cpu.cycle_cnt += 2;
		break;

	case CPX:
	case CPY_im:
	case CPX_var:
	case CPY_var:
	case CPX_zp:
	case CPY_zp:
	{
		int addr = get_mem(++cpu.pc);
		unsigned char val = addr;
		if (opcode != CPX && opcode != CPY_im) {
			if (opcode == CPX_var || opcode == CPY_var)
			addr |= get_mem(++cpu.pc) << 8;
			cpu.cycle_cnt += 2;
			val = get_mem(addr);
		}
		unsigned char reg = (opcode == CPY_im || opcode == CPY_var) ? cpu.y : cpu.x;
		unsigned char res = reg - val;
		SET_ZERO(reg == val);
		SET_CARY(reg >= val);
		SET_NEGATIVE(res & 0x80);
		cpu.cycle_cnt += 2;
	}
	break;
	case JMP_ab:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		cpu.pc = addr;
		cpu.cycle_cnt += 3;
		goto out;
	}
	case JSR:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		
		ram[0x100 | cpu.s] = ((cpu.pc & 0xff00) >> 8);
		--cpu.s;
		ram[0x100 | cpu.s] = cpu.pc & 0xff;
		--cpu.s;
		cpu.pc = addr;
		cpu.cycle_cnt += 6;
		goto out;
	}
	break;
	case RTS:
	{
		++cpu.s;
		int16_t low  = ram[0x100 | cpu.s];
		++cpu.s;
		int16_t high = ram[0x100 | cpu.s];
		++cpu.s;

		cpu.pc = ((high << 8) | low) + 1;
		cpu.cycle_cnt += 6;
		goto out;
	}
	break;
	case ADC_ab:
	case ADC_zp:
	{
		int addr = get_mem(++cpu.pc);
		char old_sign = cpu.a & 0x80;

		if (opcode != ADC_zp) {
			addr |= get_mem(++cpu.pc) << 8;
			++cpu.cycle_cnt; /* small trick so I don't have to if/else cycle count below */
		}
		char res = get_mem(addr);
		int check_carry = cpu.a + res;

		cpu.a += res + (cpu.flag & CARY_FLAG);
		SET_CARY(check_carry > 255);
		SET_OVERFLOW(old_sign != (cpu.a & 0x80));
		SET_NEGATIVE(!!(0x80 & cpu.a));
		SET_ZERO(!cpu.a);
		cpu.cycle_cnt += 3;
	}
	break;
	case ADC_im:
	{
		int addr = get_mem(++cpu.pc);
		char old_sign = cpu.a & 0x80;

		int check_carry = cpu.a + addr;

		cpu.a += addr + (cpu.flag & CARY_FLAG);
		SET_CARY(check_carry > 255);
		SET_OVERFLOW(old_sign != (cpu.a & 0x80));
		SET_NEGATIVE(!!(0x80 & cpu.a));
		SET_ZERO(!cpu.a);
		cpu.cycle_cnt += 2;
	}
	break;
	case ORA_ab:
	case ORA_zp:
	case ORA_im:
	{
		char res;
		if (opcode == ORA_ab) {
			int addr = get_mem(++cpu.pc);

			addr |= get_mem(++cpu.pc) << 8;
			res = get_mem(addr);
			cpu.cycle_cnt += 2;
		} else if (opcode == ORA_zp) {
			int addr = get_mem(++cpu.pc);

			res = get_mem(addr);
			cpu.cycle_cnt += 1;
		} else {
			res = get_mem(++cpu.pc);
		}
		char old_sign = cpu.a | 0x80;
		SET_ZERO(!cpu.a);
		SET_NEGATIVE(!!(0x80 & cpu.a));
		cpu.cycle_cnt += 2;
	}
	break;
	case SBC_ab:
	case SBC_zp:
	case SBC_im:
	{
		char res;
		if (opcode == SBC_ab || opcode == SBC_zp) {
			int addr = get_mem(++cpu.pc);
			cpu.cycle_cnt += 1;

			if (opcode == SBC_ab) {
				cpu.cycle_cnt += 1;
				addr |= get_mem(++cpu.pc) << 8;
			}
			res = get_mem(addr);
		} else {
			res = get_mem(++cpu.pc);
		}
		char old_sign = cpu.a & 0x80;
		int check_carry = cpu.a - res - 1 + (cpu.flag & CARY_FLAG);

		cpu.a -= res;
		SET_CARY(check_carry >= 0);
		SET_OVERFLOW(old_sign != (cpu.a & 0x80));
		SET_NEGATIVE(!!(0x80 & cpu.a));
		SET_ZERO(!cpu.a);
		cpu.cycle_cnt += 2;
	}
	break;
	case LDA_addx:
	case LDA_addy:
	{
		unsigned char base_addr = get_mem(++cpu.pc);
		unsigned char to_add = opcode == LDA_addx ? cpu.x : cpu.y;
		unsigned char base_addr2 = base_addr + to_add;
		int page_cross = 0;
		if (base_addr2 < base_addr) {
			page_cross = 1;
		}
		int addr = base_addr2;

		addr |= (get_mem(++cpu.pc) + page_cross) << 8;
		cpu.a = get_mem(addr);
		SET_ZERO(!!cpu.a);
		SET_NEGATIVE(!!(cpu.a & 0x80));
		cpu.cycle_cnt += (4 + page_cross); // + 1 if page is cross ?
	}
	break;
	case LDA_ab:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;

		cpu.a = get_mem(addr);
		SET_ZERO(!cpu.a);
		SET_NEGATIVE(!!(cpu.a & 0x80));
		cpu.cycle_cnt += 4;
	}
	break;
	case LDA_zp:
	case LDX_zp:
	case LDY_zp:
	{
		int addr = get_mem(++cpu.pc);
		int res;

		if (opcode == LDA_zp) {
			cpu.a = get_mem(addr);
			res = cpu.a;
		} else if (opcode == LDX_zp) {
			cpu.x = get_mem(addr);
			res = cpu.x;
		} else {
			cpu.y = get_mem(addr);
			res = cpu.y;
		}
		SET_ZERO(!res);
		SET_NEGATIVE(!!(res & 0x80));
		cpu.cycle_cnt += 3;
		break;
	}
	case LDX_im:
	case LDX_ab:
	case LDY_im:
	case LDY_ab:
	{
		char res;
		unsigned char flag_check;

		if (opcode == LDY_ab || opcode == LDX_ab) {
			int addr = get_mem(++cpu.pc);

			addr |= get_mem(++cpu.pc) << 8;
			res = get_mem(addr);
			cpu.cycle_cnt += 2;
		} else {
			res = get_mem(++cpu.pc);
		}

		if (opcode == LDX_im || opcode == LDX_ab) {
			cpu.x = res;
			flag_check = cpu.x;
		} else {
			flag_check = cpu.y;
			cpu.y = res;
		}
		SET_ZERO(!flag_check);
		SET_NEGATIVE(!!(flag_check & 0x80));
		cpu.cycle_cnt += 2;
	}
	break;
	case STX_ab:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		cpu.cycle_cnt += 4;
		set_mem(addr, cpu.x);
	}
	break;
	case BIT_ab:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		/* set zero flag */
		char res = get_mem(addr);
		SET_ZERO(!(res & cpu.a));
		SET_OVERFLOW(!!(res & 0x40));
		SET_NEGATIVE(!!(res & 0x80));
		cpu.cycle_cnt += 4;
	}
	break;
	case BPL:
	{
		signed char addr = get_mem(++cpu.pc);

		cpu.cycle_cnt += 2;
		if (!(cpu.flag & NEGATIVE_FLAG)) {
			int old_pc = cpu.pc;
			cpu.pc += addr + 1;
			cpu.cycle_cnt += 1 + check_new_page(old_pc, cpu.pc);
			goto out;
		}
	}
	break;
	case BNE:
	{
		signed char addr = get_mem(++cpu.pc);

		cpu.cycle_cnt += 2;
		if (!(cpu.flag & ZERO_FLAG)) {
			int old_pc = cpu.pc;
			cpu.pc +=addr + 1;
			cpu.cycle_cnt += 1 + check_new_page(old_pc, cpu.pc);
			goto out;
		}
	}
	break;
	case STA_ab:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		cpu.cycle_cnt += 4;
		set_mem(addr, cpu.a);
	}
	break;
	case STA_z:
	case STX_zp:
	case STY_zp:
	{
		int addr = get_mem(++cpu.pc);

		cpu.cycle_cnt += 3;
		if (opcode == STX_zp)
		  set_mem(addr, cpu.x);
		else if (opcode == STY_zp)
		  set_mem(addr, cpu.y);
		else
		  set_mem(addr, cpu.a);
	}
	break;
	case STA_zaddr:
	{
		int addr = get_mem(++cpu.pc) + cpu.x;

		cpu.cycle_cnt += 4;
		set_mem(addr, cpu.a);
	}
	break;
	case STA_xaddr:
	{
		int addr = get_mem(++cpu.pc) + cpu.x;

		addr |= get_mem(++cpu.pc) << 8;
		cpu.cycle_cnt += 5;
		set_mem(addr, cpu.a);
	}
	break;
	default:
		printf("(%x)%s: UNIMPLEMENTED\n", opcode, opcode_str[opcode]);
		turn_mode = DEBUG_MODE;
		break;
	}
	++cpu.pc;
out:
	if (turn_mode == DEBUG_MODE)
		printf("\n");
	return ret;
}

void *fy_action(int nbArgs, void **args)
{
	Entity *wid = args[0];
	Entity *events = args[1];

	wid_width = ywWidth(wid);
	wid_height = ywHeight(wid);
	current_emu_mode = yeGetIntAt(wid, "mode");
	if (current_emu_mode == NES_MODE) {
		set_mem = set_mem_nes;
		get_mem = get_mem_nes;
	} else if (current_emu_mode == YIRL_0_MODE) {
		set_mem = set_mem_yirl;
		get_mem = get_mem_yirl;
	} else {
		get_mem = get_mem_atari;
		set_mem = set_mem_atari;
	}

	/* handle atari events, curently, we'll use arrow + enter for player 2 and wasd, + e for player 1 */
	if (current_emu_mode == ATARI_MODE) {
		/* player 1: wasd + e, bits 4-7 of SWCHA */
		if (yevIsKeyDown(events, 'w'))
			riot.direction_press &= ~(1 << 4);
		else if (yevIsKeyUp(events, 'w'))
			riot.direction_press |= (1 << 4);
		if (yevIsKeyDown(events, 's'))
			riot.direction_press &= ~(1 << 5);
		else if (yevIsKeyUp(events, 's'))
			riot.direction_press |= (1 << 5);
		if (yevIsKeyDown(events, 'a'))
			riot.direction_press &= ~(1 << 6);
		else if (yevIsKeyUp(events, 'a'))
			riot.direction_press |= (1 << 6);
		if (yevIsKeyDown(events, 'd'))
			riot.direction_press &= ~(1 << 7);
		else if (yevIsKeyUp(events, 'd'))
			riot.direction_press |= (1 << 7);
		if (yevIsKeyDown(events, 'e'))
			riot.p_fire_press[0] = 1;
		else if (yevIsKeyUp(events, 'e'))
			riot.p_fire_press[0] = 0;

		/* player 2: arrows + enter, bits 0-3 of SWCHA */
		if (yevIsKeyDown(events, Y_UP_KEY))
			riot.direction_press &= ~(1 << 0);
		else if (yevIsKeyUp(events, Y_UP_KEY))
			riot.direction_press |= (1 << 0);
		if (yevIsKeyDown(events, Y_DOWN_KEY))
			riot.direction_press &= ~(1 << 1);
		else if (yevIsKeyUp(events, Y_DOWN_KEY))
			riot.direction_press |= (1 << 1);
		if (yevIsKeyDown(events, Y_LEFT_KEY))
			riot.direction_press &= ~(1 << 2);
		else if (yevIsKeyUp(events, Y_LEFT_KEY))
			riot.direction_press |= (1 << 2);
		if (yevIsKeyDown(events, Y_RIGHT_KEY))
			riot.direction_press &= ~(1 << 3);
		else if (yevIsKeyUp(events, Y_RIGHT_KEY))
			riot.direction_press |= (1 << 3);
		if (yevIsKeyDown(events, '\n'))
			riot.p_fire_press[1] = 1;
		else if (yevIsKeyUp(events, '\n'))
			riot.p_fire_press[1] = 0;
	}

	if (yevIsKeyDown(events, 'i')) {
		printf("fast mode activate\n");
		turn_mode = RUN_MODE;
		ywSetTurnLengthOverwrite(-1);
	} else if (yevIsKeyDown(events, 'o')) {
		printf("await mode activate\n");
		turn_mode = DEBUG_MODE;
		ywSetTurnLengthOverwrite(0);
	}

	if (turn_mode == DEBUG_MODE && !yevIsKeyDown(events, 's')) {
		Entity *eve;

		YEVE_FOREACH(eve, events) {
			if (ywidEveType(eve) == YKEY_DOWN) {
				int action = ywidEveKey(eve);
				if (action == 'r') {
					printf("breakpoint reset\n");
					breakpoints[breakpoints_cnt] = 0;
				} else if (action >= '0' && action <= '9') {
					breakpoints[breakpoints_cnt] = breakpoints[breakpoints_cnt] << 4;
					breakpoints[breakpoints_cnt] += action - '0';
					printf("breakpoint value %x\n", breakpoints[breakpoints_cnt]);
				} else if (action >= 'a' && action <= 'f') {
					breakpoints[breakpoints_cnt] = breakpoints[breakpoints_cnt] << 4;
					breakpoints[breakpoints_cnt] += (action - 'a' + 10);
					printf("breakpoint value %x\n", breakpoints[breakpoints_cnt]);
				} else if (action == 'z') {
					putchar('[');
					for (int i = 0; i <= 0xff; ++i) {
						if ((i & 0xf) == 0) {
							printf("\n\t");
						} else {
							putchar(',');
						}
						printf("%x", ram[i]);
					}
					putchar('\n');
					putchar(']');
					putchar('\n');
				} else if (action == 'p') {
					printf("CPU:\na: %x(%d)\n"
					       "x: %x(%d)\n"
					       "y: %x(%d)\n"
					       "s: %x\n"
					       "flag: %x\n"
					       "PC: %hx\n"
					       "cycle_cnt: %lld\n",
					       (int)cpu.a, (int)cpu.a,
					       (int)cpu.x, (int)cpu.x,
					       (int)cpu.y, (int)cpu.y,
					       (int)cpu.s,
					       (int)cpu.flag, cpu.pc,
					       (long long int)cpu.cycle_cnt);
					if (current_emu_mode == NES_MODE) {
						printf("PPU:\n"
						       "pc: %hx\n"
						       "sprite_loc: %hu\n"
						       "not_vblank: %d\n",
						       ppu.pc,
						       ppu.sprite_loc,
						       (int)ppu.not_vblank);
					} else if (current_emu_mode == ATARI_MODE) {
						printf("PPU:\n"
						       "col p0: %x\n"
						       "col p1: %x\n"
						       "col bg: %x\n"
						       "col playfield: %x\n"
						       "current line: $d\n",
						       (int)tia.col_p[0],
						       (int)tia.col_p[1],
						       (int)tia.col_bg,
						       (int)tia.col_playfield,
						       atari_curent_scane_line());
					}
				}
			}
		}
		goto out;
	}

	uint64_t time = y_get_time();
	if (turn_mode == DEBUG_MODE) {
		process_inst();
	} else {
		int64_t start_cycle = cpu.cycle_cnt;
		for (;turn_mode != DEBUG_MODE;) {
			uint64_t cur = y_get_time();
			int frm_length = 1000000 / 60;
			if (cur - time > frm_length) {
				break; /* VBLANK ? */
			}
			if (cpu.cycle_cnt - start_cycle > CYCLE_PER_FRAME) {
				int sleep_time = frm_length - ((cur-time));
				if (sleep_time > 0)
					usleep(sleep_time);
				
				break;
			}
			/* 35 is kind of random, so I don't use y_get_time at each instruction,
			   which is pretty heavy */
			for (int i = 0; i < 35; ++i) {
				process_inst();
				for (int i = 0; i < breakpoints_cnt; ++i) {
					if ((cpu.pc & 0x0000ffff) == breakpoints[breakpoints_cnt]) {
						turn_mode = DEBUG_MODE;
						break;
					}
				}
				if (turn_mode == DEBUG_MODE)
					break;
			}
		}
	}

out:
	
	return NULL;
}

void *fy_init(int nbArgs, void **args)
{
	Entity *wid = args[0];
	char *rom_path;
	yeAutoFree Entity *atari_color = ygFileToEnt(YJSON, "./color-atari.json", NULL);

	yeConvert(wid, YHASH);
	main_canvas = wid;
	yeCreateString("rgba: 0 0 0 255", wid, "background");
	yeCreateFunction("fy_action", ygGetTccManager(), wid, "action");
	yePushBack(wid, atari_color, "atari_color");
	yeAutoFree Entity *rom;
	if (ygGetProgramArg()) {
		rom_path = ygGetProgramArg();
	} else if ((rom_path = yeGetStringAt(wid, "rom")) == NULL) {
		rom_path = "background.nes";
	}

	if (yeGetIntAt(wid, "release")) {
		turn_mode = RUN_MODE;
	} else {
		turn_mode = DEBUG_MODE;
		for (int i = 0; i < sizeof ram; ++i)
			ram[i] = (unsigned char)yuiRand();
	}
	rom = ygFileToEnt(YRAW_FILE_DATA, rom_path, NULL);

	printf("rom %s\n", rom_path);
	printf("len r: %x - %x\n", yeLen(rom), yeLen(rom) + 0x4020);
	yePushBack(wid, rom, "rom");
	cartridge = yeGetData(rom);
	if (cartridge[0] == 'N' && cartridge[1] == 'E' && cartridge[2] == 'S') {
		printf("NES mode\n");
		yeCreateInt(NES_MODE, wid, "mode");
	} else if (!strcmp(cartridge, "YIRL 0")) {
		printf("YIRL 0 mode\n");
		yeCreateInt(YIRL_0_MODE, wid, "mode");
		cpu.pc = 0x1000;
	} else {
		printf("ATARI MODE\n");
		yeCreateInt(ATARI_MODE, wid, "mode");
		yeCreateInt(YC_MERGE_NO_MERGE, wid, "mergable");
		current_emu_mode = ATARI_MODE;
		colors_json = yeGet(wid, "atari_color");
		cpu.pc = 0xf000;
	}
	void *ret = ywidNewWidget(wid, "canvas");
	for (int i = 0; i < 0x100; ++i) {
		switch (i) {
		case 0xff:
			opcode_str[i] = "uninitialized";
			break;
#define OPCODE(opcode, _) case opcode: { opcode_str[i] = #opcode; break; }
#include "opcode.h"
#undef OPCODE
		default:
			opcode_str[i] = "unknow";
		}
	}


	return ret;
}

void *mod_init(int nbArgs, void **args)
{
	Entity *mod = args[0];
	yeAutoFree Entity *init = yeCreateFunction("fy_init", ygGetTccManager(), NULL, NULL);

	ygInitWidgetModule(mod, "famiyirl", init);
	return mod;
}
