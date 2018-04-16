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

byte b_read  (adr a);					//читает из "старой памяти" mem байт с "адресом" a.
void b_write (adr a, byte val);	// пишет значение val в "старую память" mem в байт с "адресом" a.
word w_read  (adr a);					// читает из "старой памяти" mem слово с "адресом" a.
void w_write (adr a, word val);	// пишет значение val в "старую память" mem в слово с "адресом" a.
void test_mem();

void load_file(char * file_name);
void mem_dump(adr start, word n);

word get_nn();
struct Operand get_ss(word w);

void run(adr pc0);
void do_halt();
void do_mov();
void do_add();
void do_unknown();

struct Operand
{
	adr reg;
	adr mode;
} ss , dd;

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
	{0000000, 0000000, "unknown", do_unknown, NO_PARAM}
};

int main(int argc, char **argv) {
	test_mem();
	load_file(argv[1]);
	run(0x3E8);
	return 0;
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
	int i = 0;
	while(1)
	{
		word w = w_read(pc);
		printf("%06o:%06o\n", pc, w);
		for(i = 0; i <= 3; i++)
		{
			struct Command cmd = command[i];
			if((w & cmd.mask) == cmd.opcode)
			{
				if(cmd.param & HAS_NN)
				{
					//nn = get_nn(w);
				}
				if(cmd.param & HAS_SS)
				{
					ss = get_ss(w);
				}
				if(cmd.param & HAS_DD)
				{
					dd = get_ss(w<<6);
				}
				cmd.func();
				break;
			}
		}
		pc += 2;
	}
}

struct Operand get_ss(word w)
{
	struct Operand oper = {0, 0};
	word help = (w<<4);
	word helper = (help>>10);                          //word helper = (w<<4)>>10;
	oper.reg = (helper & 7);
	oper.mode = (helper >> 3);
	assert(oper.reg);
	return oper;
}

void do_halt()
{
	printf("HALT\n");
	exit(0);
}

void do_add()
{
	printf("ADD\n");
}

void do_mov()
{
	switch(ss.mode)
	{
		case 0:
		case 1:
		default:
		{
			printf("Unknown mode");
			exit(3);
		}
	}
	printf("MOVE\n");
	printf("'ss.reg = %o' 'ss.mode = %o'\n'dd.reg = %o' 'dd.mode = %o'\n", ss.reg, ss.mode, dd.reg, dd.mode);
}

void do_unknown()
{
	printf("UNKNOWN\n");
	exit(2);
}

void test_mem()
{
	
}
