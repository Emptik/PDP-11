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
#define N_AND_Z(A) if((A) > 0)\
	{\
		CL(N);\
	}\
	else if((A) < 0)\
	{\
		SE(N);\
	}\
	if((A) == 0)\
	{\
		SE(Z);\
	}\
	else if((A) != 0)\
	{\
		CL(Z);\
	}
#define C_AND_V(Arg1, dest_field) int val_macro = (int)(Arg1);\
	if(val_macro != (dest_field))\
		SE(C);\
	else if(val_macro == (dest_field)) \
		CL(C);

typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned short adr;

enum Token {
	MEM,
	REG,
	N = 0,
	Z,
	C,
	V,
};

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

void SE(byte type);
void CL(byte type);
byte RE(byte type);

void run(adr pc0);
void do_halt();
void do_mov();
void do_movb();
void do_add();
void do_clr();
void do_sob();
void do_beq();
void do_br();
void do_tst();
void do_tstb();
void do_bpl();
void do_jsr();
void do_rts();
void do_dec();
void do_mul();
void do_div();
void do_inc();
void do_incb();
void do_sub();
void do_bne();
void do_cmp();
void do_jmp();
void do_adc();
void do_bmi();
void do_asr();
void do_asl();
void do_bgt();
void do_blt();
void do_bic();
void do_ash();
void do_unknown();

struct Operand
{
	word a;
	word val;
	word reg_or_mem;
};

struct Command {
	word opcode;
	word mask;
	char * name;
	void (*func)();
	byte param;
};
