#include "machine.h"
#include "Simulator.h"

void
Simulator::execute() 
{
    if (deReg.bubble) {
		emRegNew.bubble = true;
		return;
	}

	if (stall) {
		emRegNew.bubble = true;
		return;
	}

    int op_type = deReg.op_type;
    int op1 = deReg.op1;
    int op2 = deReg.op2;
    int rs1 = deReg.rs1;
    int rs2 = deReg.rs2;
    int imm = deReg.imm;
    int offset = deReg.offset;
}