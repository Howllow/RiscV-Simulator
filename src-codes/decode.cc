#include "Simulator.h"

#define ALWAYS_TAKE 0
#define NEVER_TAKE 1

bool
Simulator::predictBranch()
{
	if (!strategy) {
		return true;
	}
	
	else if (strategy == 1) {
		return false;
	}
	return true;
}

bool
MemRead(int type)
{
	if (type == LB || type == LH || type == LW || type == LD || type == LBU ||
      	type == LHU || type == LWU) 
    	return true;
  return false;
}

void
Simulator::decode()
{
	if (fdReg.bubble) {
		if (ifprint)
			printf("ID: Bubble\n");
		deRegNew.bubble = true;
		deRegNew.takeBranch = false;
		deRegNew.rs1 = -1;
		deRegNew.rs2 = -1;
		deRegNew.rd = -1;

		return;
	}

	unsigned instruction = fdReg.instruction;
	int type = UNK;
	bool takeBranch = false;
	int opcode = instruction & 0x7F;
	unsigned funct7 = (instruction >> 25) & 0x7F;
	unsigned funct3 = (instruction >> 12) & 0x7;
	int rd = (instruction >> 7) & 0x1F;
	int rs1 = (instruction >> 15) & 0x1F;
	int rs2 = (instruction >> 20) & 0x1F;
	deRegNew.rs1 = -1;
	deRegNew.rs2 = -1;
	deRegNew.rd = -1;
	long long offset;

	//R-Type instruction
	if (opcode == OP_R) {
		deRegNew.rs1 = rs1;
		deRegNew.rs2 = rs2;
		deRegNew.op1 = reg[rs1];
		deRegNew.op2 = reg[rs2];
		deRegNew.rd = rd;
		if (funct3 == 0x0) {
			if (funct7 == 0x00) 
				type = ADD;
			else if (funct7 == 0x01)
				type = MUL;
			else if (funct7 == 0x20)
				type = SUB;
		}
		else if (funct3 == 0x1) {
			if (funct7 == 0x00)
				type = SLL;
			else if (funct7 == 0x01)
				type = MULH;
		}
		else if (funct3 == 0x2) {
			if (funct7 == 0x00)
				type = SLT;
		}
		else if (funct3 == 0x4) {
			if (funct7 == 0x00)
				type = XOR;
			else if (funct7 == 0x01)
				type = DIV;
		}
		else if (funct3 == 0x5) {
			if (funct7 == 0x00)
				type = SRL;
			else if (funct7 == 0x20)
				type = SRA;
		}
		else if (funct3 == 0x6) {
			if (funct7 == 0x00)
				type = OR;
			else if (funct7 == 0x01)
				type = REM;
		}
		else if (funct3 == 0x7) {
			if (funct7 == 0x00)
				type = AND;
		}
	}

	else if (opcode == OP_LOAD) {
		deRegNew.offset = int(instruction) >> 20;
		deRegNew.rd = rd;
		deRegNew.rs1 = rs1;
		deRegNew.op1 = reg[rs1];
		deRegNew.op2 = deRegNew.offset;
		if (funct3 == 0x0)
			type = LB;
		else if (funct3 == 0x1)
			type = LH;
		else if (funct3 == 0x2)
			type = LW;
		else if (funct3 == 0x3)
			type = LD;
		else if (funct3 == 0x4)
			type = LBU;
		else if (funct3 == 0x5)
			type = LHU;
		else if (funct3 == 0x6)
			type = LWU;
	}

	else if (opcode == OP_I) {
		deRegNew.imm = int(instruction) >> 20;
		deRegNew.rs1 = rs1;
		deRegNew.op1 = reg[rs1];
		deRegNew.op2 = deRegNew.imm;
		deRegNew.rd = rd;
		if (funct3 == 0x0)
			type = ADDI;
		else if (funct3 == 0x1) {
			type = SLLI;
			deRegNew.op2 = deRegNew.op2 & 0x3F;
		}
		else if (funct3 == 0x2)
			type = SLTI;
		else if (funct3 == 0x3)
			type = SLTIU;
		else if (funct3 == 0x4)
			type = XORI;
		else if (funct3 == 0x5) {
			if (((instruction >> 26) & 0x3F) == 0x00) {
				type = SRLI;
				deRegNew.op2 = deRegNew.op2 & 0x3F;
			}
			else if (((instruction >> 26) & 0x3F) == 0x10) {
				type = SRAI;
				deRegNew.op2 = deRegNew.op2 & 0x3F;
			}
		}
		else if (funct3 == 0x6)
			type = ORI;
		else if (funct3 == 0x7)
			type = ANDI;
	}

	else if (opcode == OP_I32) {
		deRegNew.imm = int(instruction) >> 20;
		deRegNew.rs1 = rs1;
		deRegNew.op1 = reg[rs1];
		deRegNew.op2 = deRegNew.imm;
		deRegNew.rd = rd;
		if (funct3 == 0x0)
			type = ADDIW;
		else if (funct3 == 0x1)
			type = SLLIW;
		else if (funct3 == 0x5) {
			if (funct7 == 0x00)
				type = SRLIW;
			else if (funct7 == 0x20)
				type = SRAIW;
		}
	}

	else if (opcode == OP_JALR) {
		deRegNew.imm = int(instruction) >> 20;
		deRegNew.op1 = reg[rs1];
		deRegNew.op2 = deRegNew.imm;
		deRegNew.rs1 = rs1;
		deRegNew.rd = rd;
		if (funct3 == 0x0)
			type = JALR;
	}

	else if (opcode == OP_SYS) {
		if (funct3 == 0x0) {
			if (funct7 == 0x000) { 
				deRegNew.op1 = reg[REG_A0];
				deRegNew.op2 = reg[REG_A7];
				deRegNew.rs1 = REG_A0;
				deRegNew.rs2 = REG_A7;
				type = ECALL;
			}
		}
	}

	else if (opcode == OP_STORE) {
		deRegNew.offset = int(((instruction >> 7) & 0x1F) | ((instruction >> 20) & 0xFE0)) << 20 >> 20;
		deRegNew.rs2 = rs2;
		deRegNew.rs1 = rs1;
		deRegNew.op1 = reg[rs1];
		deRegNew.op2 = reg[rs2];
		if (funct3 == 0x0)
			type = SB;
		else if (funct3 == 0x1)
			type = SH;
		else if (funct3 == 0x2)
			type = SW;
		else if (funct3 == 0x3)
			type = SD;
	}

	else if (opcode == OP_BRANCH) {
		deRegNew.offset = int(((instruction >> 7) & 0x1E) | ((instruction >> 20) & 0x7E0) |
                    ((instruction << 4) & 0x800) | ((instruction >> 19) & 0x1000)) << 19 >> 19;
		offset = deRegNew.offset;
		deRegNew.rs1 = rs1;
		deRegNew.rs2 = rs2;
		deRegNew.op1 = reg[rs1];
		deRegNew.op2 = reg[rs2];
		if (funct3 == 0x0) 
			type = BEQ;
		else if (funct3 == 0x1)
			type = BNE;
		else if (funct3 == 0x4)
			type = BLT;
		else if (funct3 == 0x5)
			type = BGE;
		else if (funct3 == 0x6)
			type = BLTU;
		else if (funct3 == 0x7)
			type = BGEU;

		if (type != UNK) {
			if (predictBranch()) {
				fdRegNew.bubble = true; // in order to get the correct PC 
				controlHazard += 4;
				takeBranch = true;
				PC_taken = fdReg.PC + offset;
				if (ifprint)
					printf("predict PC 0x%.8llx\n", PC_taken);
				PC_not_taken = fdReg.PC + 4;
			}
			else {
				takeBranch = false;
				PC_not_taken = fdReg.PC + offset;
				PC_taken = fdReg.PC + 4;
				if (ifprint)
					printf("predict PC 0x%.8llx\n", PC_taken);
			}
		}
		else {
			printf("Unknown inst 0x%llx\n", instruction);
			exit(-1);
		}		
	}

	else if (opcode == OP_R32) {
		deRegNew.rs1 = rs1;
		deRegNew.rs2 = rs2;
		deRegNew.rd = rd;
		deRegNew.op1 = reg[rs1];
		deRegNew.op2 = reg[rs2];
		if (funct3 == 0x0) {
			if (funct7 == 0x00)
				type = ADDW;
			else if (funct7 == 0x20)
				type = SUBW;
		}
		else if (funct3 == 0x1) 
			type = SLLW;
		else if (funct3 == 0x5) {
			if (funct7 == 0x00) 
				type = SRLW;
			else if (funct7 == 0x20)
				type = SRAW;
		}
	}

	else if (opcode == OP_LUI) {
		deRegNew.offset = int(instruction) >> 12; 
		deRegNew.rd = rd;
		type = LUI;
	}

	else if (opcode == OP_AUI) {
		deRegNew.offset = int(instruction) >> 12; 
		deRegNew.rd = rd;
		type = AUIPC;
	}

	else if (opcode == OP_JAL) {
		deRegNew.imm = int(((instruction >> 1) & 0x7F800) | ((instruction >> 10) & 0x400) | 
					((instruction >> 12) & 0x80000) | ((instruction >> 21) & 0x3FF)) << 12 >> 11;
		deRegNew.rd = rd;
		type = JAL;
	}

	if (ifprint) {
		printf("ID: decode 0x%.8x, type is %d\n", instruction, type);
	}
	deRegNew.PC = fdReg.PC;
	deRegNew.op_type = type;
	deRegNew.bubble = false;
	deRegNew.takeBranch = takeBranch;

}