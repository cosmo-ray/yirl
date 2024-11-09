#include <yirl/all.h>

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

struct cpu {
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

int breakpoints[64];
int breakpoints_cnt;


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
struct ppu {
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

unsigned char ppu_mem[0x4000];

unsigned char ram[0x800];
unsigned char miror0[0x800];
unsigned char miror1[0x800];
unsigned char miror2[0x800];
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

char *opcode_str[0x100];

Entity *main_canvas;

void (*set_mem)(uint16_t addr, char val);
unsigned char (*get_mem)(uint16_t addr);

void set_mem_yirl(uint16_t addr, char val)
{
	printf("set_mem_yirl\n");
}

void get_mem_yirl(uint16_t addr)
{
	printf("get_mem_yirl");
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
	printf("code (a: %x, x: %x, y: %x, f: %x): 0x%x: %x - %s\n", cpu.a, cpu.x, cpu.y, cpu.flag,
	       cpu.pc & 0xffff, get_mem(cpu.pc), opcode_str[get_mem(cpu.pc)]);
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
		cpu.x += (1 + (cpu.flag & CARY_FLAG));
		SET_NEGATIVE(!!(cpu.x & 0x80));
		SET_ZERO(!cpu.x);
		cpu.cycle_cnt += 2;
		break;
	case INY:
		cpu.y += (1 + (cpu.flag & CARY_FLAG));
		SET_NEGATIVE(!!(cpu.y & 0x80));
		SET_ZERO(!cpu.y);
		cpu.cycle_cnt += 2;
		break;
	case DEX:
		cpu.x -= (2 - (cpu.flag & CARY_FLAG));
		SET_NEGATIVE(!!(cpu.x & 0x80));
		SET_ZERO(!cpu.x);
		cpu.cycle_cnt += 2;
		break;
	case DEY:
		cpu.y -= (2 - (cpu.flag & CARY_FLAG));
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
	{
		int addr = get_mem(++cpu.pc);
		if (opcode == CPX_var) {
			addr |= get_mem(++cpu.pc);
			cpu.cycle_cnt++;
		}

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
	break;
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
	case STA_xaddr:
	{
		int addr = get_mem(++cpu.pc) + cpu.x;

		addr |= get_mem(++cpu.pc) << 8;
		set_mem(addr, cpu.a);
		cpu.cycle_cnt += 5;
	}
	break;
	default:
		printf("%s: UNIMPLEMENTED", opcode_str[opcode]);
		break;
	}
	++cpu.pc;
out:
	printf("\n");
	return ret;
}

enum  {
	RUN_MODE = -1,
	DEBUG_MODE = 0
};

void *fy_action(int nbArgs, void **args)
{
	Entity *events = args[1];
	static int turn_mode;

	set_mem = set_mem_nes;
	get_mem = get_mem_nes;

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
				}
			}
		}
		return NULL;
	}

	if (turn_mode == DEBUG_MODE) {
		process_inst();
	} else {
		for (int i = 0; i < 35; ++i) {
			process_inst();
			if ((cpu.pc & 0x0000ffff) == breakpoints[breakpoints_cnt]) {
				turn_mode = DEBUG_MODE;
				break;
			}
		}
	}

	return NULL;
}

void *fy_init(int nbArgs, void **args)
{
	Entity *wid = args[0];
	yeConvert(wid, YHASH);
	main_canvas = wid;
	yeCreateString("rgba: 0 0 0 255", wid, "backgroung");
	yeCreateFunction("fy_action", ygGetTccManager(), wid, "action");
	yeAutoFree Entity *rom;
	if (ygGetProgramArg()) {
		rom = ygFileToEnt(YRAW_FILE_DATA, ygGetProgramArg(), NULL);
	} else {
		rom = ygFileToEnt(YRAW_FILE_DATA, "background.nes", NULL);
	}
	printf("len r: %x - %x\n", yeLen(rom), yeLen(rom) + 0x4020);
	yePushBack(wid, rom, "rom");
	cartridge = yeGetData(rom);
	printf("%s\n", cartridge);
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
