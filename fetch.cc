#include "Simulator.h"

//get one instruction at given addr
unsigned
getInstruction(uint32_t addr)
{
	return getB(addr) + (getB(addr + 1) << 8) + (getB(addr + 2) << 16) + (getB(addr + 3) << 24);
}

void
Simulator::fetch()
{
	if (stall) {
		stall = false;
		if (ifprint) {
		printf("IF: Stall and fetch 0x%.8x at 0x%llx\n", fdRegNew.instruction, PC);
		}
		PC += 4;
		return;
	}
	fdRegNew.instruction = getInstruction(PC);
	if (ifprint) {
		printf("IF: fetch 0x%.8x at 0x%llx\n", fdRegNew.instruction, PC);
	}
	fdRegNew.bubble = false;
	fdRegNew.PC = PC;
	PC += 4;
}