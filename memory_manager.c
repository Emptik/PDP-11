#include "prototypes.h"

extern byte mem[];
extern word reg[];
extern word r;
extern word psw;
extern struct Operand ss, dd, nn;
extern char xx;
extern struct STA stack;
extern struct Command command[];

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
	if(a % 2)
	{
		fprintf(stderr, "You odd adr is:%06o\n", a);
		exit(102);
	}
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
	short counter = 0;
	short check = 0;
	unsigned int val = 0;
	unsigned int num = 0;
	unsigned int a = 0;
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
	printf("\npsw=%06o\n", psw);
}
