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
} cpu = {.a = 0, .x = 0, .y = 0, .s = 0xff, .flag = 0, .pc = 0xC000};


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
	int16_t pc;
	unsigned char not_vblank;
} ppu;

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

void set_mem(uint16_t addr, char val)
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
			printf("write ppu dest addr\n");
		} else if (addr == 0x2000) {
			if (!val)
				printf("disable MMI ?\n");
		} else if (0x2001) {
			if (!val)
				printf("disable rendering ?\n");			
		}
	} else if (addr < 0x4000) {
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

unsigned char get_mem(uint16_t addr)
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
			unsigned char ret = 0;

			++ppu.not_vblank;
			if (ppu.not_vblank & 0x08)
				ret |= 0x80;
			printf("not vblak: %x - %x\n", ppu.not_vblank, ret);
			return ret;
		}
		printf("PPU regs\n");
		return ram[addr - 0x1800];
	} else if (addr < 0x4000) {
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

void *fy_action(int nbArgs, void **args)
{
	unsigned char opcode = get_mem(cpu.pc);
	printf("code (a: %x, x: %x, y: %x, f: %x): 0x%x: %x - %s\n", cpu.a, cpu.x, cpu.y, cpu.flag,
	       cpu.pc & 0xffff, get_mem(cpu.pc), opcode_str[get_mem(cpu.pc)]);
	switch (opcode) {
	case INX:
		cpu.x += (1 + (cpu.flag & CARY_FLAG));
		SET_NEGATIVE(!!(cpu.a & 0x80));
		SET_ZERO(!cpu.x);
		break;
	case CMP_2:
	{
		unsigned char addr = get_mem(++cpu.pc);

		SET_ZERO(cpu.a == addr);
		SET_OVERFLOW(addr >= cpu.a);
		SET_NEGATIVE(!!(((signed char)addr - cpu.sa) & 0x70));
		printf("to: %x", addr);
	}
	break;
	case BEQ:
	{
		int addr = get_mem(++cpu.pc);

		if (cpu.flag & ZERO_FLAG) {
			if (addr > 0)
				cpu.pc +=addr;
			else
				cpu.pc -= addr;
			goto out;
		}

		printf("to: %x", addr);
	}
	break;
	case TXS:
		cpu.s = cpu.x;
		break;
	case AND:
	{
		int addr = get_mem(++cpu.pc);

		cpu.a &= addr;
		SET_ZERO(!cpu.a);
		SET_NEGATIVE(!!(cpu.a & 0x70));
		printf("to: %x", addr);
	}
	break;
	case LDA_2:
	{
		int addr = get_mem(++cpu.pc);

		cpu.a = addr;
		SET_ZERO(!!cpu.a);
		SET_NEGATIVE(!!(cpu.a & 0x80));
		printf("to: %x", addr);
	}
	break;
	case CLC:
		cpu.flag &= 0xfe;
		break;
	case SEC:
		cpu.flag |= 1;
	case CPX:
	case CPY:
	case CPX_var:
	{
		int addr = get_mem(++cpu.pc);
		if (opcode == CPX_var)
			addr |= get_mem(++cpu.pc);

		printf("to: %x", addr);			
	}
	break;
	case JMP:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		printf("to: %x", addr);
		cpu.pc = addr;
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
		printf("to: %x", addr);
		goto out;
	}
	break;
	case ADC:
	{
		int addr = get_mem(++cpu.pc);

		cpu.a += addr;
		printf("to: %x", addr);
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
		printf("to: %x-%x", addr, cpu.a);
	}
	break;
	case LDA_addr:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;

		cpu.a = get_mem(addr);
		printf("to: %x-%x", addr, cpu.a);
		SET_ZERO(!cpu.a);
		SET_NEGATIVE(!!(cpu.a & 0x80));
	}
	break;
	case LDX:
	case LDY:
	{
		int addr = get_mem(++cpu.pc);

		if (opcode == LDX)
			cpu.x = addr;
		else
			cpu.y = addr;
		SET_ZERO(!cpu.x);
		SET_NEGATIVE(!!(cpu.x & 0x80));
		printf("to: %x", addr);
	}
	break;
	case STX_2:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		printf("to: %x", addr);
		set_mem(addr, cpu.x);
	}
	break;
	case BIT:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		/* set zero flag */
		char res = get_mem(addr);
		SET_ZERO(!(res & cpu.a));
		SET_OVERFLOW(!!(res & 0x40));
		SET_NEGATIVE(!!(res & 0x80));
		printf("to: %x - %x", addr, res);
	}
	break;
	case BPL:
	{
		signed char addr = get_mem(++cpu.pc);

		if (!(cpu.flag & NEGATIVE_FLAG)) {
			printf("pc += %d\n", addr);
			printf("pc: %x\n", cpu.pc);
			cpu.pc += addr + 1;
			printf("pc: %x\n", cpu.pc);
			goto out;
		}
			

		/* addr |= get_mem(++cpu.pc) << 8; */

		printf("to: %x", addr);
	}
	break;
	case BNE:
	{
		signed char addr = get_mem(++cpu.pc);

		if (!(cpu.flag & ZERO_FLAG)) {
			if (addr > 0)
				cpu.pc +=addr;
			else
				cpu.pc -= addr;
			goto out;
		}
		/* addr |= get_mem(++cpu.pc) << 8; */

		printf("to: %x", addr);
	}
	break;
	case STA_addr:
	{
		int addr = get_mem(++cpu.pc);

		addr |= get_mem(++cpu.pc) << 8;
		set_mem(addr, cpu.a);
		printf("to: %x", addr);
	}
	break;
	case STA_xaddr:
	{
		int addr = get_mem(++cpu.pc) + cpu.x;

		addr |= get_mem(++cpu.pc) << 8;
		set_mem(addr, cpu.a);
		printf("to: %x", addr);
	}
	break;
	}
	++cpu.pc;
out:
	printf("\n");
	return NULL;
}

void *fy_init(int nbArgs, void **args)
{
	Entity *wid = args[0];
	yeConvert(wid, YHASH);
	yeCreateString("rgba: 0 0 0 255", wid, "backgroung");
	yeCreateFunction("fy_action", ygGetTccManager(), wid, "action");
	yeAutoFree Entity *rom = ygFileToEnt(YRAW_FILE_DATA, "cq.nes", NULL);
	printf("len r: %x - %x\n", yeLen(rom), yeLen(rom) + 0x4020);
	yePushBack(wid, rom, "rom");
	cartridge = yeGetData(rom);
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

	ygAddModule(Y_MOD_YIRL, mod, "load-chr");
	ygInitWidgetModule(mod, "famiyirl", init);
	return mod;
}
