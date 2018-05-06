#include "prototypes.h"

FILE * f_out;
byte mem[64 * 1024];
word reg[8];
word r;
word psw;
struct Operand ss, dd, nn;
char xx;
struct STA stack = {{0}, 1};
struct Command command[] = {
	{0, 0xFFFF, "halt", do_halt, NO_PARAM},
	{0010000, 0170000, "mov", do_mov, HAS_SS | HAS_DD},
	{0110000, 0170000, "movb", do_movb, HAS_SS | HAS_DD},
	{0060000, 0170000, "add", do_add, HAS_SS | HAS_DD},
	{0005000, 0177700, "clr", do_clr, HAS_DD},
	{0077000, 0177000, "sob", do_sob, HAS_NN},
	{0001400, 0xFF00, "beq", do_beq, HAS_XX},
	{0005700, 0177700, "tst", do_tst, HAS_DD},
	{0105700, 0177700, "tstb",do_tstb, HAS_DD},
	{0100000, 0xFF00, "bpl", do_bpl, HAS_XX},
	{0000400, 0XFF00, "br", do_br, HAS_XX},
	{0004000, 0177000, "jsr", do_jsr, HAS_DD},
	{0000200, 0177700, "rts", do_rts, NO_PARAM},
	{0005300, 0177700, "dec", do_dec, HAS_DD},
	{0070000, 0177000, "mul", do_mul, HAS_DD},
	{0071000, 0177000, "div", do_div, HAS_DD},
	{0005200, 0177700, "inc", do_inc, HAS_DD},
	{0105200, 0177700, "incb", do_incb, HAS_DD},
	{0160000, 0170000, "sub", do_sub, HAS_SS | HAS_DD},
	{0001000, 0177000, "bne", do_bne, HAS_XX},
	{0020000, 0070000, "cmp", do_cmp, HAS_DD | HAS_SS},
	{0000100, 0177700, "jmp", do_jmp, HAS_DD},
	{0005500, 0177700, "adc", do_adc, HAS_DD},
	{0100400, 0177700, "bmi", do_bmi, HAS_XX},
	{0003000, 0177700, "bgt", do_bgt, HAS_XX},
	{0002400, 0177700, "blt", do_blt, HAS_XX},
	{0040000, 0170000, "bic", do_bic, HAS_SS | HAS_DD},
	{0006200, 0177700, "asr", do_asr, HAS_DD},
	{0006300, 0177700, "asl", do_asl, HAS_DD},
	{0072000, 0177000, "ash", do_ash, HAS_DD},
	{0000000, 0000000, "unknown", do_unknown, NO_PARAM}
};

int main(int argc, char **argv) 
{
	mem[ostat] = 0xFF;
	test_mem();
	load_file(argv[1]);
	f_out = fopen("list", "w");
	if(!f_out)
	{
		perror("f_out");
	}
	run(0x200);
	fclose(f_out);
	return 0;
}

void run(adr pc0)
{
	pc = (word)pc0;
	fprintf(stderr,"%06d:\t\t. = %o", 0, pc);
	int i = 0;
	while(1)
	{
		word w = w_read(pc);
		fprintf(stderr,"\n%06o:", pc);
		fprintf(stderr,"\t%06o\t", w);
		pc += 2;
		for(i = 0; ; i++)
		{
			struct Command cmd = command[i];
			if((w & cmd.mask) == cmd.opcode)
			{
				fprintf(stderr, "\t%s", cmd.name);
				r_mean(cmd.name, w);
				if(cmd.param & HAS_NN)
				{
					nn = get_nn(w);
				}
				if(cmd.param & HAS_SS)
				{
					ss = get_dd(w>>6);
				}
				if(cmd.param & HAS_DD)
				{
					dd = get_dd(w);
				}
				if(cmd.param & HAS_XX)
				{
					xx = get_xx(w);
				}
				cmd.func();
				break;
			}
		}
	}
}
