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
		fetch();
		decode();
		execute();
		memory();
		writeBack();

		fdReg = fdRegNew;
		deReg = deRegNew;
		emReg = emRegNew;
		mwReg = mwRegNew;
		reg[0] = 0;
	}
}