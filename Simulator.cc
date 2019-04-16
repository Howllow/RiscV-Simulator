#include "Simulator.h"

Simulator::Simulator(bool singlestep, int strategy, bool ifprint) 
{
	PC = 0;
	stall = false;
	cycles = 0;
	insts = 0;
	goodPredict = 0;
	badPredict = 0;
	this->singlestep = singlestep;
	this->strategy = strategy;
	this->ifprint = ifprint;

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
		reg[0] = 0;
		cycles++;
		deReg = deRegNew;
		emReg = emRegNew;
		mwReg = mwRegNew;
		if (stall) {
			PC -= 4;
			if (fdReg.bubble)
				fdReg.bubble =false;
		}
		else {
			fdReg = fdRegNew;
		}
		if (deReg.takeBranch && !stall && !deReg.bubble){
			PC = PC_taken;
		}
	
		if (ifprint) {
			Print();
		}
		if (singlestep) {
			printf("Press Enter to contunue!");
			getchar();
		}
		
	}
}

void
Simulator::Print()
{
	/*
	printf("----------------MEM-------------------\n");
	for (int i = 0; i < 1024; ++i) {
		if (mem[i] == NULL) {
			continue;
		}
		printf("First Level Page 0x%x-0x%x:\n", i << 22, (i + 1) << 22);
			for (int j = 0; j < 1024; ++j) {
				if (mem[i][j] == NULL) {
					continue;
				}
				printf("  Second Level Page 0x%x-0x%x\n", (i << 22) + (j << 12),
						(i << 22) + ((j + 1) << 12));
			}
    }
		*/
	printf("-------------------------------------\n");
	printf("\n");
	printf("----------------REG------------------\n");
	for (int i = 0; i < 32; i++) {
		printf("x%d: 0x%.16llx", i, reg[i]);
		if (i % 4 == 3)
			printf("\n");
		printf("\n");
	}
	printf("PC: 0x%llx\n", PC);
	printf("-------------------------------------\n");
}