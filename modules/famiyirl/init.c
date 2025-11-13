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
} cpu = {.a = 0, .x = 0, .y = 0, .s = 0xff,
	.flag = 0, .pc = 0xC000, .cycle_cnt = 0};

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

#define CYCLE_PER_SCANE_LINE 105

#define ATARI_SCREEN_H 262

/* it seems every 105 cycle I need to update v_line */
/* another souce seems to say 76 cycles */
static struct atari_ppu {
	uint8_t col_p0;
	uint8_t col_p1;
	uint8_t col_bg;
	uint8_t col_playfield;
	uint8_t vblank_mode;
} atari_ppu;

/**
 * PPU:
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

void (*set_mem)(uint16_t addr, char val);
unsigned char (*get_mem)(uint16_t addr);
int current_emu_mode;

int wid_width;
int wid_height;

void set_mem_yirl(uint16_t addr, char val)
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
}

static int atari_curent_scane_line(void)
{
	return (cpu.cycle_cnt / CYCLE_PER_SCANE_LINE) % ATARI_SCREEN_H;
}


static char *atari_get_color(unsigned int idx)
{
	return yeGetStringAt(yeGet(colors_json, (idx >> 4)), (idx & 0xf) / 2);
}

//yeGet(colors_json, atari_color_idx(atari_ppu.col_bg))


static int atari_do_scan_line(void)
{
	if (atari_ppu.vblank_mode)
		return 0;
	int cur = atari_curent_scane_line();
	int pix_per_pix = wid_width / 160;
	if (wid_height / 192 < pix_per_pix)
		pix_per_pix = wid_height / 192;
	if (cur < 40) {
		/* in blank */
		return 0;
	}
	if (cur > 232)
		return 0;
	/* printf("l: %d %d| %d: %d %d %d %d\n", cur, cur-40, pix_per_pix, wid_height, wid_width, 160 * pix_per_pix, 192 * pix_per_pix); */
	/* printf("pix per pix: %d\n%d %d %d% d\n", */
	/*        pix_per_pix, */
	/*        0, (cur - 40) * pix_per_pix, */
	/*        160 * pix_per_pix, pix_per_pix); */
	ywCanvasMergeRectangle(main_canvas, 0, (cur - 40) * pix_per_pix,
			       160 * pix_per_pix, pix_per_pix,
			       atari_get_color(atari_ppu.col_bg));
}

void set_mem_atari(uint16_t addr, char val)
{
	if (turn_mode == DEBUG_MODE)
		printf("set_mem_atari at %d(%x): %d\n", addr, addr, val);
	if (addr < 0x2c) {
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
			atari_ppu.vblank_mode = val;
			break;
		case VSYNC:
			cpu.cycle_cnt = 0;
		case WSYNC:
			/* printf("cycle %d, cycle per sl: %d, next: %d\n", */
			/*        cpu.cycle_cnt, CYCLE_PER_SCANE_LINE, */
			/*        CYCLE_PER_SCANE_LINE - cpu.cycle_cnt % CYCLE_PER_SCANE_LINE); */
			/* printf("cur sl: %d, col %x\n", atari_curent_scane_line(), */
			/*        atari_ppu.col_bg); */
			cpu.cycle_cnt += CYCLE_PER_SCANE_LINE - cpu.cycle_cnt % CYCLE_PER_SCANE_LINE;
			return;
		case COLUBK:
			atari_ppu.col_bg = val;
			return;
		}
		/* printf("peripheric write at %x\n", addr); */
		return;
	}
	ram[addr] = val;
}

unsigned char get_mem_atari(uint16_t addr)
{
	if (turn_mode == DEBUG_MODE)
		printf("get_mem_atari at %d - %x\n", addr, addr);
	if (addr >= 0xf000)
		return cartridge[addr - 0xf000];
	else if (addr > 0x2c)
		return ram[addr];
	else {
		printf("peripheric read at %x\n", addr);
	}
	return 0;
}

void set_mem_nes(uint16_t addr, char val)
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

static int process_inst(void)
{
	unsigned char opcode = get_mem(cpu.pc);
	int ret = 0;
	if (turn_mode == DEBUG_MODE)
		printf("code (a: %x, x: %x, y: %x, f: %x): 0x%x: %x - %s\n", cpu.a, cpu.x, cpu.y, cpu.flag,
		       cpu.pc & 0xffff, get_mem(cpu.pc), opcode_str[get_mem(cpu.pc)]);
	if (current_emu_mode == ATARI_MODE) {
		static old_sl_cycle;
		int elapse = cpu.cycle_cnt - old_sl_cycle;
		if (elapse > CYCLE_PER_SCANE_LINE) {
			atari_do_scan_line();
			old_sl_cycle += elapse / CYCLE_PER_SCANE_LINE;
		}
	}

	switch (opcode) {
	case NOP:
		cpu.cycle_cnt += 2;
		break;
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
		set_mem(addr, val);
		SET_CARY(new_c);
		SET_NEGATIVE(!!(0x80 & val));
		SET_ZERO(!val);
		cpu.cycle_cnt += 6;
	}
	break;
	case EOR_IM:
	  {
	    unsigned char addr = get_mem(++cpu.pc);

	    cpu.a |= addr;
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

	    cpu.a |= c;
	    SET_NEGATIVE(!!(0x80 & cpu.a));
	    SET_ZERO(!cpu.a);
	    cpu.cycle_cnt += 3;
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
		set_mem(addr, c);
		cpu.cycle_cnt += 6;
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
	case BEQ:
	{
		int addr = get_mem(++cpu.pc);

		if (cpu.flag & ZERO_FLAG) {
			cpu.pc +=addr + 1;
			goto out;
		}
		cpu.cycle_cnt += 2;
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
		SET_NEGATIVE(!!(cpu.a & 0x70));
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
	case SEI:
		cpu.flag &= 0xfb; // & 11111011
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
	case SEC:
		cpu.flag |= 1;
		cpu.cycle_cnt += 2;
		break;

	case CPX:
	case CPY:
	case CPX_var:
	case CPY_var:
	{
		int addr = get_mem(++cpu.pc);
		if (opcode == CPX_var || opcode == CPY_var) {
			addr |= get_mem(++cpu.pc) << 8;
			cpu.cycle_cnt += 1;
		}
		unsigned char reg = (opcode == CPY || opcode == CPY_var) ? cpu.y : cpu.x;
		unsigned char val = get_mem(addr);
		unsigned char res = reg - val;
		SET_ZERO(reg == val);
		SET_CARY(reg >= val);
		SET_NEGATIVE(res & 0x80);
		cpu.cycle_cnt += 2 + ((opcode == CPX_var) || (opcode == CPY_var));
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
		ram[0x100 | cpu.s] = cpu.pc & 0xff;
		--cpu.s;
		ram[0x100 | cpu.s] = ((cpu.pc & 0xff00) >> 8);
		--cpu.s;
		cpu.pc = addr;
		cpu.cycle_cnt += 6;
		goto out;
	}
	break;
	case ADC_ab:
	{
		int addr = get_mem(++cpu.pc);
		char old_sign = cpu.a & 0x80;

		addr |= get_mem(++cpu.pc) << 8;
		char res = get_mem(addr);
		int check_carry = cpu.a + res;

		cpu.a += res + (cpu.flag & CARY_FLAG);
		SET_CARY(check_carry > 255);
		SET_OVERFLOW(old_sign != (cpu.a & 0x80));
		SET_NEGATIVE(!!(0x80 & cpu.a));
		SET_ZERO(!cpu.a);
		cpu.cycle_cnt += 4;
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
	case SBC_im:
	{
		char res;
		if (opcode == SBC_ab) {
			int addr = get_mem(++cpu.pc);

			addr |= get_mem(++cpu.pc) << 8;
			res = get_mem(addr);
			cpu.cycle_cnt += 2;
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
	{
		unsigned char base_addr =  get_mem(++cpu.pc);
		base_addr += cpu.x;
		int addr = base_addr;

		addr |= get_mem(++cpu.pc) << 8;
		cpu.a = get_mem(addr);
		SET_ZERO(!!cpu.a);
		SET_NEGATIVE(!!(cpu.a & 0x80));
		cpu.cycle_cnt += 4; // + 1 if page is cross ?
	}
	break;
	case LDA_ab:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;

		cpu.a = get_mem(addr);
		SET_ZERO(!cpu.a);
		SET_NEGATIVE(!!(cpu.a & 0x80));
		cpu.cycle_cnt += 3;
	}
	break;
	case LDX_im:
	case LDX_ab:
	case LDY_im:
	case LDY_ab:
	{
		char res;

		if (opcode == LDY_ab || opcode == LDX_ab) {
			int addr = get_mem(++cpu.pc);

			addr |= get_mem(++cpu.pc) << 8;
			res = get_mem(addr);
			cpu.cycle_cnt += 2;
		} else {
			res = get_mem(++cpu.pc);
		}

		if (opcode == LDX_im || opcode == LDX_ab)
			cpu.x = res;
		else
			cpu.y = res;
		SET_ZERO(!cpu.x);
		SET_NEGATIVE(!!(cpu.x & 0x80));
		cpu.cycle_cnt += 2;
	}
	break;
	case STX_ab:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		set_mem(addr, cpu.x);
		cpu.cycle_cnt += 4;
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
		cpu.cycle_cnt += 3;
	}
	break;
	case BPL:
	{
		signed char addr = get_mem(++cpu.pc);

		cpu.cycle_cnt += 2; // +p ?
		if (!(cpu.flag & NEGATIVE_FLAG)) {
			cpu.pc += addr + 1;
			cpu.cycle_cnt++;
			goto out;
		}
	}
	break;
	case BNE:
	{
		signed char addr = get_mem(++cpu.pc);

		cpu.cycle_cnt += 2; // +p ?
		if (!(cpu.flag & ZERO_FLAG)) {
			cpu.pc +=addr + 1;
			cpu.cycle_cnt++;
			goto out;
		}
	}
	break;
	case STA_ab:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		set_mem(addr, cpu.a);
		cpu.cycle_cnt += 4;
	}
	break;
	case STA_z:
	{
		int addr = get_mem(++cpu.pc);

		set_mem(addr, cpu.a);
		cpu.cycle_cnt += 3;
	}
	break;
	case STA_zaddr:
	{
		int addr = get_mem(++cpu.pc) + cpu.x;

		set_mem(addr, cpu.a);
		cpu.cycle_cnt += 4;
	}
	break;
	case STA_xaddr:
	{
		int addr = get_mem(++cpu.pc) + cpu.x;

		addr |= get_mem(++cpu.pc) << 8;
		set_mem(addr, cpu.a);
		cpu.cycle_cnt += 5;
	}
	break;
	default:
		printf("(%x)%s: UNIMPLEMENTED", opcode, opcode_str[opcode]);
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
						       (int)atari_ppu.col_p0,
						       (int)atari_ppu.col_p1,
						       (int)atari_ppu.col_bg,
						       (int)atari_ppu.col_playfield,
						       atari_curent_scane_line());
					}
				}
			}
		}
		return NULL;
	}

	uint64_t time = y_get_time();
	if (turn_mode == DEBUG_MODE) {
		process_inst();
	} else {
		for (;turn_mode != DEBUG_MODE;) {
			uint64_t cur = y_get_time();
			if (cur - time > (1000000 / 60))
				break; /* VBLANK ? */
			for (int i = 0; i < 35; ++i) {
				process_inst();
				for (int i = 0; i < breakpoints_cnt; ++i) {
					if ((cpu.pc & 0x0000ffff) == breakpoints[breakpoints_cnt]) {
						turn_mode = DEBUG_MODE;
						break;
					}
				}
			}
		}
	}

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
