#include "print.h"

void PrintInt(int n)
{
	asm("li a7, 0;" "ecall");
}

void PrintChar(char cha)
{
	asm("li a7, 1;" "ecall");
}