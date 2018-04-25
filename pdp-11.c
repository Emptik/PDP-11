#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define _CHECK( x , y ) if ( (x) == EOF)\
{\
assert((y));\
fclose((f_in));\
break;\
}
#define NO_PARAM 0
#define HAS_SS 1
#define HAS_DD 2
#define HAS_NN 4
#define HAS_XX 8
#define pc reg[7]
#define sp reg[6]
#define prn fprintf(stderr, "%d %s\n", __LINE__, __FUNCTION__)
#define ostat 0177564
#define odata 0177566

typedef unsigned char byte;
typedef unsigned short int word;
typedef short adr;

void reg_write(adr a, word val);
word reg_read(adr a);
byte b_read(adr a);
void b_write(adr a, byte val);
word w_read(adr a);
void w_write(adr a, word val);
void test_mem();

void load_file(char * file_name);
void mem_dump(adr start, word n);
void reg_print();

struct Operand get_dd(word w);
struct Operand get_nn(word w);
char get_xx(word w);
void r_mean(char * model, word w);

void SE(byte * x);
void CL(byte * x);

void run(adr pc0);
void do_halt();
void do_mov();
void do_movb();
void do_add();
void do_clr();
void do_sob();
void do_beq();
void do_br();
void do_tstb();
void do_bpl();
void do_jsr();
void do_rts();
void do_unknown();

byte mem[64 * 1024];
word reg[8];

word r;
word help;

struct Operand
{
	word a;
	word val;
} ss , dd, nn;
char xx;

struct psw
{
	byte N;
	byte Z;
	byte V;
	byte C;
} flag;

struct STA
{
	word arr[1000];
	word size;
} stack = {{0}, 1};

FILE * f_out;

struct Command {
	word opcode;
	word mask;
	char * name;
	void (*func)();
	byte param;
} command[] = {
	{0, 0xFFFF, "halt", do_halt, NO_PARAM},
	{0010000, 0170000, "mov", do_mov, HAS_SS | HAS_DD},
	{0110000, 0170000, "movb", do_movb, HAS_SS | HAS_DD},
	{0060000, 0170000, "add", do_add, HAS_SS | HAS_DD},
	{0005000, 0017700, "clr", do_clr, HAS_DD},
	{0077000, 0177000, "sob", do_sob, HAS_NN},
	{0001400, 0xFF00, "beq", do_beq, HAS_XX},
	{0005700, 0017700, "tstb", do_tstb, HAS_DD},
	{0100000, 0xFF00, "bpl", do_bpl, HAS_XX},
	{0000400, 0XFF00, "br", do_br, HAS_XX},
	{0004000, 0174000, "jsr", do_jsr, HAS_DD},
	{0000200, 0177270, "rts", do_rts, NO_PARAM},
	{0000000, 0000000, "unknown", do_unknown, NO_PARAM}
};

int main(int argc, char **argv) 
{
	mem[ostat] = -1;
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

void reg_write(adr a, word val)
{
	if(!(a <= 7 && a >= 0))
	{
		fprintf(stderr,"\nadr is to big. It is : %o\n", a);
		abort();
	}
	reg[a] = val;
}

word reg_read(adr a)
{
	if(!(a <= 7 && a >= 0))
	{
		fprintf(stderr,"\nadr is to big. It is : %o\n", a);
		abort();
	}
	return reg[a];
}

word w_read(adr a) {
	assert(!(a % 2));
	word val = 0xffff;
	val = val & (((word)mem[a + 1]) << 8);
	val = val + (word)mem[a];
	return val;
}

void w_write(adr a, word val) {
	assert(!(a % 2));
	mem[a + 1] = (byte)(val >> 8);
	mem[a] = (byte)((val << 8) >> 8);
}

byte b_read(adr a){
	return mem[a];
}

void b_write(adr a, byte val){
	mem[a] = val;
}

void test_mem() 
{
	byte b0, b1;
	word w;
	w = 0x0d0c;
	w_write(2, w);
	b0 = b_read(2);
	b1 = b_read(3); 
	assert(b0 == 0x0c);
	assert(b1 == 0x0d);
	b_write(4, 0x0c);
	b_write(5, 0x0d);
	w = w_read(4);
	assert(w == 0x0d0c);
}

void load_file(char * file_name)
{
	adr counter = 0;
	adr check = 0;
	unsigned int  val = 0;
	unsigned int  num = 0;
	unsigned int  a = 0;
	FILE * f_in = NULL;
	f_in = fopen(file_name, "r");
	if(!f_in) {
		perror("f_in");
		exit(1);
	}
	for( ; ; )
	{
		check = fscanf(f_in, "%04x", &a);
		_CHECK(check, 1);
		check = fscanf(f_in, "%04x", &num);
		_CHECK(check, 0);
		for(counter = 0; counter < num; counter++)
		{
			check = fscanf(f_in, "%02x", &val);
			_CHECK(check, 0);
			b_write(a + counter, val);
		}
	}
}

void mem_dump(adr start, word n) {
	int i = 0;
	for(; i < n; i += 2)
	{
		printf("%06o : ", start + i);
		printf("%06o\n", w_read(start + i));
	}
}

void reg_print()
{
	int i = 0;
	printf("\n");
	for( ; i <= 6; i++)
	{
		if(!(i % 2))
			printf("r%d=%06o ", i, reg[i]); 
	}
	printf("\n");
	for(i = 0; i <= 7; i++)
	{
		if(i % 2)
			printf("r%d=%06o ", i, reg[i]); 
	}
	printf("\npsw=%o%o%o%o\n", flag.N, flag.Z, flag.V, flag.C);
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
				if(!strcmp(cmd.name,"bpl") || !strcmp(cmd.name,"jsr")) 
				{
					if(!strcmp(cmd.name,"bpl"))
						do_bpl();
					else
						do_jsr();
				}
				else cmd.func();
				break;
			}
		}
	}
}

struct Operand get_dd(word w)
{
	struct Operand res = {0, 0};
	word rn = w & 7;
	word mode = (w >> 3) & 7;
	word by = (w & 0x200)>>9;
	assert(by == 1 || by == 0);
	switch(mode)
	{
		case 0:
			res.a = rn;
			res.val = reg[rn];
			fprintf(stderr,"\tR%d", rn);
			break;
		case 1:
			res.a = reg[rn];
			res.val = w_read(res.a);
			fprintf(stderr,"\t\t\tCLR (R%d)", rn);
			break;
		case 2:
		{
			if(rn == 7)
			{
				res.a = reg[rn];
				res.val = w_read(res.a);
				reg[rn] += 2;
				fprintf(stderr,"\t#%o ", res.val);
			}
			else
			{
				if(by == 1)
				{
					res.a = reg[rn];
					res.val = (char)b_read(res.a);
					reg[rn] += 1;
				}
				else if(by == 0)
				{
					res.a = reg[rn];
					res.val = w_read(res.a);
					reg[rn] += 2;
				}
				fprintf(stderr,"\t(R%d)+", rn);
			}
			break;
		}
		case 3:
			res.a = reg[rn];
			reg[rn] += 2;
			assert(!(res.a % 2));
			res.a = w_read(res.a);
			res.val = w_read(res.a);
			if(rn == 7)
				fprintf(stderr,"\t@#%06o ", res.a);
			else fprintf(stderr,"\t@(R%d)+", rn);
			break;
		case 4:
			if(rn == 7)
			{
				reg[rn] -= 2;
				res.a = reg[rn];
				res.val = w_read(res.a);
				fprintf(stderr,"\t-(pc)");
			}
			else
			{
				if(by == 1)
				{
					reg[rn] -= 1;
					res.a = reg[rn];
					res.val = (char)b_read(res.a);
				}
				else if(by == 0)
				{
					reg[rn] -= 2;
					res.a = reg[rn];
					res.val = w_read(res.a);
				}
				fprintf(stderr,"\t(R%d)-", rn);
			}
			break;
		case 6:
		{
			pc += 2;
			res.a =  reg_read(rn) + w_read(pc-2);
			res.val = w_read(res.a);
			if( rn != 7) fprintf(stderr, "\t%06o(R%o)", w_read(pc-2), rn);
			else fprintf (stderr, "\t#%06o", w_read(pc-2) + pc);
			break;
		}
		default:
			fprintf(f_out,"MODE %d NOT IMPLEMENTED YET!\n", mode);
			exit(3);
	}
	return res;
}

struct Operand get_nn(word w)
{
	struct Operand res;
	res.val = w & 63; 
	res.a = (w&(7<<6))>>6;
	return res;
}

char get_xx(word w)
{
	return (char)w;
}

void r_mean(char * model, word w)
{
	if(!strcmp(model,"jsr"))
	{
		r = (w>>6)&7;
	}
	else if(!strcmp(model, "rts"))
	{
		r = w&7;
	}
}

void CL(byte * x)
{
	*(x) = 0;
}

void SE(byte * x)
{
	*(x) = 1;
}

void do_halt()
{
	reg_print();
	fclose(f_out);
	exit(0);
}

void do_mov() 
{
	if(dd.a <= 7)
	{
		reg[dd.a] = ss.val;
		if(reg_read(dd.a) == 0)
		{
			if(!flag.Z) SE(&flag.Z);
			else CL(&flag.Z);
		}
	}
	else
	{
		w_write(dd.a, ss.val);
		if(w_read(dd.a) == 0)
		{
			if(!flag.Z) SE(&flag.Z);
			else CL(&flag.Z);
		}
	}
}

void do_movb()
{
	if(dd.a == odata)
	{
		fprintf(f_out, "%c", ss.val);
	}
	if(dd.a <= 7)
	{
		reg[dd.a] = ss.val;
		if(reg_read(dd.a) == 0)
		{
			if(!flag.Z) SE(&flag.Z);
			else CL(&flag.Z);
		}
	}
	else
	{
		w_write(dd.a, ss.val);
		if(w_read(dd.a) == 0)
		{
			if(!flag.Z) SE(&flag.Z);
			else CL(&flag.Z);
		}
	}
	if(flag.Z)
	{
		help = 1;
	}
}

void do_add() 
{
	if(dd.a <= 7)
	{
		reg[dd.a] = ss.val + dd.val;
		if(reg_read(dd.a) == 0)
		{
			if(!flag.Z) SE(&flag.Z);
			else CL(&flag.Z);
		}
	}
	else
	{
		w_write(dd.a, ss.val + dd.val);
		if(w_read(dd.a) == 0)
		{
			if(!flag.Z) SE(&flag.Z);
			else CL(&flag.Z);
		}
	}
}

void do_clr()
{
	if(dd.a <= 7) reg_write(dd.a, 0);
	else w_write(dd.a, 0);
}

void do_sob()
{
	assert(nn.a < 6);
	reg_write(nn.a, reg_read(nn.a) - 1);
	word w = reg_read(nn.a);
	if( w != 0)
	{
		pc -= 2 * nn.val;
		fprintf(f_out, "\tR%o\t", nn.a);
		fprintf(f_out, "%06o", pc);
	}
}

void do_beq()
{
	if(help)
	{
		//printf("\t%o", flag.Z);
	}
	assert(flag.Z == 1 || flag.Z == 0);
	if(flag.Z) do_br();
}

void do_br()
{
	pc += 2 * xx;
	fprintf(stderr, "\t%o", pc);
}

void do_tstb()
{
	if(dd.val > 0)
	{
		CL(&flag.N);
	}
	else if(dd.val < 0)
	{
		SE(&flag.N);
	}
	if(dd.val == 0)
	{
		SE(&flag.Z);
	}
	else if(dd.val != 0)
	{
		CL(&flag.Z);
	}
	CL(&flag.C);
	CL(&flag.V);
}

void do_bpl()
{
	if(flag.N) do_br();
}

void do_jsr()
{
	stack.arr[stack.size] = pc;
	reg_write(r, pc);
	pc = dd.a;
	stack.size++;
}

void do_rts()
{
	pc = reg_read(r);
	reg_write(r, stack.arr[stack.size-1]);
	stack.size--;
	fprintf(stderr,"\t%06o", pc);
}

void do_unknown()
{
	printf("UNKNOWN!\n");
	exit(2);
}
