#include "Simulator.h"
#include "machine.h"
#include "MM.h"

//get one instruction at given addr
unsigned
getInstruction(uint32_t addr)
{
	return getB(addr) + getB(addr + 1) + getB(addr + 2) + getB(addr + 3);
}

void
Simulator::fetch()
{
	fdRegNew.instruction = getInstruction(PC);
	fdRegNew.bubble = false;
	fdRegNew.PC = PC;
	PC += 4;
}