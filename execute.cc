#include "Simulator.h"

int64_t 
Simulator::syscall(int64_t a7, int64_t a0) 
{
    int64_t type = a7;
    int64_t arg = a0;
    switch (type)
    {
        case 93:
        printf("Simulator Exiting!\n");
        printf("-------------------------------------\n");
        printf("-------------------------------------\n");
        printf("Cycles: %llu\n", cycles);
        printf("Instructions: %llu\n", insts);
        printf("CPI: %.4f\n", ((float)cycles) / insts);
        printf("Good Predict: %d\n", goodPredict);
        printf("Bad Predict: %d\n", badPredict);
        printf("Predict Acc: %.4f\n", (float(goodPredict)) / (goodPredict + badPredict));
        exit(0);
        default:
            printf("Syscall %d Unknown!!\n", type);
            exit(-1);
    }
    return arg;
}

void
Simulator::execute() 
{
    if (deReg.bubble) {
        if (ifprint) {
            printf("EX: Bubble\n");
        }
		emRegNew.bubble = true;
		return;
	}

    insts++;
    emRegNew.wMem = false;
    emRegNew.rMem = false;
    emRegNew.signExt = false;

    bool takebranch = false;
    bool isbranch = false;
    bool isjump = false;
    int op_type = deReg.op_type;
    int64_t op1 = deReg.op1;
    int64_t op2 = deReg.op2;
    int rs1 = deReg.rs1;
    int rs2 = deReg.rs2;
    int64_t out = 0;
    int rd = deReg.rd;
    int imm = deReg.imm;
    long long offset = deReg.offset;
    unsigned long long updatePC = deReg.PC;
    unsigned long long branchPC = deReg.PC;

    if (ifprint) {
         printf("EX: Type is %d\n", op_type);
    }
    
    switch(op_type) 
    {   
        case LUI:
        out = offset << 12;
        break;

        case AUIPC:
        out = deReg.PC + (offset << 12);
        break;

        case JAL:
        out = deReg.PC + 4;
        PC = deReg.PC + imm;
        //cancel the fetched two wrong instructions
        fdRegNew.bubble = true;
        deRegNew.bubble = true;
        break;

        case ADD:
        case ADDI:
        out = op1 + op2;
        break;

        case ADDIW:
        case ADDW:
        out = (int64_t)((int)op1 + (int)op2);
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
        break;

        case SRLW:
        case SRLIW:
        out = uint64_t(unsigned((unsigned) op1 >> (unsigned) op2));
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
        break;

        case SLTIU:
        case SLTU:
        out = (uint64_t)op1 < (uint64_t)op2 ? 1 : 0;
        break;

        case MUL:
        out = op1 * op2;
        break;

        case XOR:
        case XORI:
        out = op1 ^ imm;
        break;

        case DIV:
        out = op1 /op2;
        break;

        case OR:
        case ORI:
        out = op1 | op2;
        break;

        case REM:
        out = op1 % op2;
        break;

        case AND:
        case ANDI:
        out = op1 & op2;
        break;

        case LB:
        emRegNew.rMem = true;
        emRegNew.memsize = 1;
        emRegNew.signExt = true;
        out = op1 + offset;
        break;

        case LH:
        emRegNew.rMem = true;
        emRegNew.memsize = 2;
        emRegNew.signExt = true;
        out = op1 + offset;
        break;

        case LW:
        emRegNew.rMem = true;
        emRegNew.memsize = 4;
        emRegNew.signExt = true;
        out = op1 + offset;
        break;

        case LD:
        emRegNew.rMem = true;
        emRegNew.memsize = 8;
        out = op1 + offset;
        break;

        case LBU:
        emRegNew.rMem = true;
        emRegNew.memsize = 1;
        out = op1 + offset;
        break;

        case LHU:
        emRegNew.rMem = true;
        emRegNew.memsize = 2;
        out = op1 + offset;
        break;

        case LWU:
        emRegNew.rMem = true;
        emRegNew.memsize = 4;
        out = op1 + offset;
        break;

        case JALR:
        out = deReg.PC + 4;
        PC = (op1 + deReg.imm) & (~(uint64_t)1);
        //cancel the fetched two wrong instructions
        fdRegNew.bubble = true;
        deRegNew.bubble = true;
        updatePC = PC;
        break;

        case ECALL:
        out = syscall(op2, op1);
        break;

        case SB:
        emRegNew.wMem = true;
        emRegNew.memsize = 1;
        op2 = op2 & 0xFF;
        out = op1 + offset;
        break;

        case SH:
        emRegNew.wMem = true;
        emRegNew.memsize = 2;
        op2 = op2 & 0xFFFF;
        out = op1 + offset;
        break;

        case SW:
        emRegNew.wMem = true;
        emRegNew.memsize = 4;
        op2 = op2 & 0xFFFFFFFF;
        out = op1 + offset;
        break;

        case SD:
        emRegNew.wMem = true;
        emRegNew.memsize = 8;
        out = op1 + offset;
        break;

        case BEQ:
        isbranch = true;
        if (op1 == op2) {
            takebranch = true;
            branchPC += offset;
        }
        break;

        case BGE:
        isbranch = true;
        if (!(op1 < op2)) {
            takebranch = true;
            branchPC += offset;
        }
        break;

        case BNE:
        isbranch = true;
        if (op1 != op2) {
            takebranch = true;
            branchPC += offset;
        }
        break;

        case BLT:
        isbranch = true;
        if (op1 < op2) {
            takebranch = true;
            branchPC += offset;
        }
        break;

        case BGEU:
        isbranch = true;
        if ((uint64_t)op1 >= (uint64_t)op2) {
            takebranch = true;
            branchPC += offset;
        }
        break;

        case BLTU:
        isbranch = true;
        if ((uint64_t)op1 < (uint64_t)op2) {
            takebranch = true;
            branchPC += offset;
        }
        break;

        default:
        printf("UNK!!\n");
        exit(-1);
    }

    // data hazard caused by memread, stall the pipeline
    if (emRegNew.rMem && (deRegNew.rs1 == rd || deRegNew.rs2 == rd)) {
        deRegNew.bubble = true;
        stall = true;
    }

    // data hazard which can be solved by forwarding
    if (!emRegNew.rMem && (rd != -1) && (rd != 0)) {
        if (deRegNew.rs1 == rd) {
            deRegNew.op1 = out;
            exeWBdest = rd;
            if (ifprint)
                printf("forwarding 0x%llx to reg:x%d\n", out, rd);
        }
        else if (deRegNew.rs2 == rd) {
            deRegNew.op2 = out;
            exeWBdest = rd;
            if (ifprint)
                printf("forwarding 0x%llx to reg:x%d\n", out, rd);
        }
    }
    
    // branch predict related
    if (isbranch) {
        // predict correctly
        if (takebranch == deReg.takeBranch) { 
            if (ifprint)
				printf("predict right!\n");
            goodPredict++;
        }
        // predict incorrectly
        else {
            badPredict++;
            PC = PC_not_taken;
            if (ifprint)
					printf("predict wrong!\n");
            //cancel the fetched two wrong instructions
            fdRegNew.bubble = true;
            deRegNew.bubble = true;
        }
    }
    
    emRegNew.op1 = op1;
    emRegNew.op2 = op2;
    emRegNew.rd = rd;
    emRegNew.out = out;
    emRegNew.bubble = false;

}