#include "Simulator.h"
#include "machine.h"
#include "MM.h"


Simulator::Simulator(bool singlestep, int strategy) 
{
	PC = 0;
	stall = 0;
	cycles = 0;
	insts = 0;
	goodPredict = 0;
	badPredict = 0;
	singlestep = singlestep;
	strategy = strategy;

	for (int i = 0; i < REG_NUM; i++) {
		reg[i] = 0;
	}
	
	//initialize the stack
	reg[REG_SP] = Stack_Base;
	for (unsigned int ad = Stack_Base; ad > Stack_Base - Stack_Size; ad--) {
		if (!check_Page(ad))
			get_Page(ad);
		setB(ad, 0);
	}
	
	//insert bubbles to simulate parrallel executing
	fdReg.bubble = true;
	deReg.bubble = true;
	emReg.bubble = true;
	mwReg.bubble = true;
}

Simulator::~Simulator() {}

void
Simulator::Run() 
{
	while(true) {
		exeWBdest = -1;
		memWBdest = -1;
		reg[0] = 0;
		fetch();
		decode();
		execute();
		memory();
		writeBack();

		deReg = deRegNew;
		emReg = emRegNew;
		mwReg = mwRegNew;

		if (!stall) {
			fdReg = fdRegNew;
			cycles++;
		}

		else {
			PC -= 4;
			stall = false;
		}

		if (!deReg.bubble) {
			if (deReg.takeBranch)
				PC = PC_taken;
		}
	
		if (singlestep) {
			Print();
			printf("Press Enter to contunue!");
			getchar();
		}
	}
}

void
Simulator::Print()
{
	printf("----------------MEM-------------------\n");
	for (int i = 0; i < 1024; ++i) {
		if (::memory[i] == NULL) {
			continue;
		}
		printf("First Level Page 0x%x-0x%x:\n", i << 22, (i + 1) << 22);
		for (int j = 0; j < 1024; ++j) {
			if (::memory[i][j] == NULL) {
				continue;
			}
			printf("  Second Level Page 0x%x-0x%x\n", (i << 22) + (j << 12),
					(i << 22) + ((j + 1) << 12));
		}
    }
	printf("-------------------------------------\n");
	printf("\n");
	printf("----------------REG------------------\n");
	for (int i = 0; i < 32; i++) {
		printf("x%d: 0x%.8llx", i, reg[i]);
		if (i % 4 == 3)
			printf("\n");
		printf("\n");
		printf("PC: 0x%llx\n", PC);
	}
	printf("-------------------------------------\n");
}