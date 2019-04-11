#include "Simulator.h"
#include "machine.h"
#include "MM.h"

Simulator::Simulator() 
{
	PC = 0;
	for (int i = 0; i < REG_NUM; i++) {
		reg[i] = 0;
	}
	
	//initialize the stack
	reg[REG_SP] = StackBase;
	for (unsigned int ad = StackBase; ad > StackBase - StackSize; ad--) {
		if (!checkPage(ad))
			getPage(ad);
		setB(ad, 0);
	}

	//initialize the  pipeline registers
	memset(&fdReg, 0, sizeof(fdReg));
	memset(&fdRegNew, 0, sizeof(fdRegNew));
	memset(&deReg, 0, sizeof(deReg));
	memset(&deRegNew, 0, sizeof(deRegNew));
	memset(&emReg, 0, sizeof(emReg));
	memset(&emRegNew, 0, sizeof(emRegNew));
	memset(&mwReg, 0, sizeof(mwReg));
	memset(&mwRegNew, 0, sizeof(mwRegNew));

	//insert bubbles to simulate parrallel executing
	fdReg.bubble = TRUE;
	deReg.bubble = TRUE;
	emReg.bubble = TRUE;
	mwReg.bubble = TRUE;
}

Simulator::~Simulator() {}

void
Simulator::Run() 
{
	while(true) {
		fetch();
		decode();
		execute();
		memory();
		writeback();

		fdReg = fdRegNew;
		deReg = deRegNew;
		emReg = emRegNew;
		mwReg = mwRegNew;
		reg[0] = 0;
	}
}