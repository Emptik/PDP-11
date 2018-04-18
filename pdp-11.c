#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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

typedef unsigned char byte;
typedef unsigned short int word;
typedef short adr;

byte mem[64 * 1024];
word reg[8];

void reg_write(adr a, word val);
word reg_read(adr a);
byte b_read  (adr a);					//читает из "старой памяти" mem байт с "адресом" a.
void b_write (adr a, byte val);	// пишет значение val в "старую память" mem в байт с "адресом" a.
word w_read  (adr a);					// читает из "старой памяти" mem слово с "адресом" a.
void w_write (adr a, word val);	// пишет значение val в "старую память" mem в слово с "адресом" a.
void test_mem();

void load_file(char * file_name);
void mem_dump(adr start, word n);

struct Operand get_nn(word w);
struct Operand get_dd(word w);
void reg_print();

void run(adr pc0);
void do_halt();
void do_mov();
void do_add();
void do_clr();
void do_sob();
void do_unknown();

struct Operand
{
	adr a;
	word val;
} ss , dd, nn;

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
	{0060000, 0170000, "add", do_add, HAS_SS | HAS_DD},
	{0077000, 0177000, "sob", do_sob, HAS_NN},
	{0005000, 0017000, "clr", do_clr, HAS_DD},
	{0000000, 0000000, "unknown", do_unknown, NO_PARAM}
};

int main(int argc, char **argv) 
{
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
	assert(a <= 7 && a >= 0);
	reg[a] = val;
}

word reg_read(adr a)
{
	assert(a <= 7 && a >= 0);
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

void run(adr pc0)
{
	pc = (word)pc0;
	fprintf(f_out,"%06d:\t\t. = %o\n", 0, pc);
	int i = 0;
	while(1)
	{
		word w = w_read(pc);
		fprintf(f_out,"%06o:", pc);
		fprintf(f_out,"\t%06o\t", w);
		pc += 2;
		for(i = 0; i <= 5; i++)
		{
			struct Command cmd = command[i];
			if((w & cmd.mask) == cmd.opcode)
			{
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
				cmd.func();
				break;
			}
		}
	}
}

struct Operand get_dd(word w)
{
	struct Operand res = {0, 0};
	int rn = w & 7;
	int mode = (w >> 3) & 7;
	switch(mode)
	{
		case 0:
			res.a = rn;
			res.val = reg[rn];
			fprintf(f_out,"\tR%d", rn);
			break;
		case 1:
			res.a = reg[rn];
			res.val = w_read(res.a);
			fprintf(f_out,"\t\t\tCLR (R%d)", rn);
			break;
		case 2:
			res.a = reg[rn];
			res.val = w_read(res.a);
			reg[rn] += 2;
			if(rn == 7)
			{
				fprintf(f_out,"\t#%o ", res.val);
			}
			else
				fprintf(f_out,"\t(R%d)+", rn);
			break;
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

void do_halt()
{
	reg_print();
	fprintf(f_out,"HALT\n");
	fclose(f_out);
	exit(0);
}

void do_add() 
{
	fprintf(f_out, "\tadd\n");
	reg_write(dd.a, ss.val + dd.val);
}

void do_mov() 
{
	fprintf(f_out,"\tmov\n");
	reg_write(dd.a, ss.val);
}

void do_unknown()
{
	printf("UNKNOWN!\n");
	exit(2);
}

void do_clr()
{
	fprintf(f_out,"\tclr\n");
	reg_write(dd.a, 0);
}

void do_sob()
{
	reg_write(nn.a, reg_read(nn.a) - 1);
	word w = reg_read(nn.a);
	if( w != 0)
	{
		fprintf(f_out,"\tsob\tR%o\t", nn.a);
		pc -= 2 * nn.val;
		fprintf(f_out, "%06o\n", pc);
	}
	else 
	{
		fprintf(f_out,"NO_sob\n");
	}
}

void reg_print()
{
	int i = 0;
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
	printf("\n");
}

void test_mem()
{
	
}
