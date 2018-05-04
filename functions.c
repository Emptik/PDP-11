#include "prototypes.h"

extern FILE * f_out;
extern byte mem[];
extern word reg[];
extern word r;
extern struct Operand ss, dd, nn;
extern char xx;
extern struct psw flag;
extern struct STA stack;
extern struct Command command[];

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
			res.reg_or_mem = REG;
			res.a = rn;
			res.val = reg[rn];
			fprintf(stderr,"\tR%d", rn);
			break;
		case 1:
			res.a = reg[rn];
			res.val = w_read(res.a);
			fprintf(stderr,"\tCLR (R%d)", rn);
			break;
		case 2:
		{
			if(rn == 7)
			{
				res.a = reg[rn];
				res.val = w_read(res.a);
				reg[rn] += 2;
				fprintf(stderr,"\t#%06o ", res.val);
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
			if(!by)
			{
				res.a = w_read(res.a);
				res.val = w_read(res.a);
			}
			if(by)
			{
				res.a = w_read(res.a);
				res.val = b_read(res.a);
			}
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
			else fprintf (stderr, "\t#%06o", res.a);
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
	if(!strcmp(model,"jsr") || !strcmp(model,"mul") || !strcmp(model,"div"))
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
	if(dd.a == odata)
	{
		fprintf(f_out, "%c", ss.val);
	}
	if(dd.reg_or_mem == REG)
	{
		reg_write(dd.a, ss.val);
		N_AND_Z((short)(reg[dd.a]));
	}
	else
	{
		w_write(dd.a, ss.val);
		N_AND_Z((short)(w_read(dd.a)));
	}
}

void do_movb()
{
	if(dd.a == odata)
	{
		fprintf(f_out, "%c", ss.val);
	}
	if(dd.reg_or_mem == REG)
	{
		reg_write(dd.a, ss.val);
		N_AND_Z((short)(reg[dd.a]));
	}
	else
	{
		b_write(dd.a, ss.val);
		N_AND_Z((char)(b_read(dd.a)));
	}
}

void do_add() 
{
	if(dd.reg_or_mem == REG)
	{
		reg_write(dd.a, dd.val + ss.val);
		N_AND_Z((short)(reg_read(dd.a)))
		C_AND_V(dd.val, ss.val, reg_read(dd.a));
	}
	else
	{
		w_write(dd.a, ss.val + dd.val);
		N_AND_Z((short)(w_read(dd.a)))
		C_AND_V(dd.val, ss.val, w_read(dd.a));
	}
}

void do_clr()
{
	SE(&flag.Z);
	if(dd.reg_or_mem == REG) reg_write(dd.a, 0);
	else w_write(dd.a, 0);
}

void do_sob()
{
	reg_write(nn.a, reg_read(nn.a) - 1);
	word w = reg_read(nn.a);
	if( w != 0)
	{
		pc -= 2 * nn.val;
		fprintf(stderr, "\tR%o\t", nn.a);
		fprintf(stderr, "%06o", pc);
	}
}

void do_beq()
{
	assert(flag.Z == 1 || flag.Z == 0);
	if(flag.Z) do_br();
}

void do_br()
{
	pc += 2 * xx;
	fprintf(stderr, "\t%o", pc);
}

void do_tst()
{
	N_AND_Z((short)(dd.val))
	CL(&flag.C);
	CL(&flag.V);
}

void do_tstb()
{
	N_AND_Z((char)(dd.val))
	CL(&flag.C);
	CL(&flag.V);
}

void do_bpl()
{
	assert(flag.N == 1 || flag.N == 0);
	if(!flag.N) do_br();
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

void do_dec()
{
	if(dd.reg_or_mem == REG)
	{
		reg_write(dd.a, dd.val - 1);
		N_AND_Z(reg_read(dd.a))
	}
	else
	{
		w_write(dd.a, dd.val - 1);
		N_AND_Z(w_read(r))
	}
}

void do_mul()
{
	fprintf(stderr, "\tR%o", r);
	reg_write(r, reg_read(r) * dd.val);
	N_AND_Z((short)reg_read(r))
}

void do_div()
{
	fprintf(stderr, "\tR%o", r);
	reg_write(r, reg_read(r) / dd.val);
	N_AND_Z((short)reg_read(r))
}

void do_inc()
{
	if(dd.reg_or_mem == REG)
	{
		reg_write(dd.a, dd.val + 1);
		N_AND_Z((short)reg_read(dd.a))
	}
	else
	{
		w_write(dd.a, dd.val+1);
		N_AND_Z((short)w_read(dd.a))
	}
}

void do_bne()
{
	if(!flag.Z)
		do_br();
}

void do_sub()
{
	if(dd.reg_or_mem == REG)
	{
		reg_write(dd.a, dd.val - ss.val);
		N_AND_Z((short)reg_read(dd.a))
	}
	else
	{
		w_write(dd.a, dd.val - ss.val);
		N_AND_Z((short)w_read(dd.a))
	}
}

void do_cmp()
{
	word k = dd.val - ss.val;
	N_AND_Z((short)k)
}

void do_jmp()
{
	pc = dd.val;
}

void do_adc()
{
	if(dd.reg_or_mem == REG)
	{
		reg_write(dd.a, dd.val + flag.C);
		N_AND_Z((short)(reg_read(dd.a)))
		C_AND_V(dd.val, ss.val, reg_read(dd.a));
	}
	else
	{
		w_write(dd.a, dd.val + flag.C);
		N_AND_Z((short)(w_read(dd.a)))
		C_AND_V(dd.val, ss.val, w_read(dd.a));
	}
}

void do_unknown()
{
	fprintf(stderr, "\n");
	exit(2);
}

