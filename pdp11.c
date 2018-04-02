#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define _CHECK( x , y ) if ( (x) == EOF)\
{\
assert((y));\
fclose((f_in));\
break;\
}\

typedef unsigned char byte;
typedef unsigned short int word;
typedef short adr;

byte mem[64 * 1024];

byte b_read  (adr a);					//читает из "старой памяти" mem байт с "адресом" a.
void b_write (adr a, byte val);	// пишет значение val в "старую память" mem в байт с "адресом" a.
word w_read  (adr a);					// читает из "старой памяти" mem слово с "адресом" a.
void w_write (adr a, word val);	// пишет значение val в "старую память" mem в слово с "адресом" a.
void load_file(char * file_name);

void mem_dump(adr start, word n);
void test_mem();

int main(int argc, char **argv) {
	test_mem();
	load_file(argv[1]);
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

void test_mem()
{
	mem_dump(0x40, 4);
}
