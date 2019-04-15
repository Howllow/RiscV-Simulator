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
    emRegNew.wMem = false;
    emRegNew.rMem = false;
    bool branch = false;
    int op_type = deReg.op_type;
    int op1 = deReg.op1;
    int op2 = deReg.op2;
    int rs1 = deReg.rs1;
    int rs2 = deReg.rs2;
    int out = 0;
    int rd = deReg.rd;
    int imm = deReg.imm;
    int offset = deReg.offset;
    unsigned long long jumpPC;

    switch(op_type) 
    {   
        case LUI:
        out = offset << 12;
        break;
        case AUIPC:
        out = deReg.PC + offset << 12;
        break;
        case JAL:
        branch = true;
        out = deReg.PC + 4;
        jumpPC = deReg.PC + imm;
        break;
        case ADD:
        case ADDI:
        out = op1 + op2;
        break;
        case SUB:
        out = op1 - op2;
        break;
        case SLL:
        case SLLI:
        out = op1 << op2;
        break;
        case SLLW:
        case SLLIW:
        out = int64_t(int(op1 << op2));
        break;
        case SRL:
        case SRLI:
        out = (uint64_t) op1 >> (uint64_t) op2;
        case SRLW:
        case SRLIW:
        out = uint64_t((unsigned) op1 >> (unsigned) op2);
        break;
        case SRA:
        case SRAI:
        out = op1 >> op2;
        break;
        case SRAW:
        case SRAIW:
        out = int64_t(int ((int) op1 >> (int) op2));
        break;
        case SLT:
        case SLTI:
        out = op1 < op2 ? 1 : 0;
        case SLTIU:
        case SLTU:
        out = (uint64_t)op1 < (uint64_t)op2 ? 1 : 0;
        




    }
}