#include "machine.h"
#include "Simulator.h"

void
Simulator:: writeBack()
{
    if (mwReg.bubble) {
        if (singlestep) {
            printf("WB: Bubble\n");
        }
        return;
    }
    
    int rd = mwReg.rd;
    if (rd != -1 && rd != 0) {
        reg[rd] = mwReg.out; // write back the value;
        if (singlestep) {
            printf("WB: write value back to reg x%d\n", rd);
        }
        //check for data hazard
        if (deRegNew.rs1 == rd) {
            //avoid overwritting, least priority
            if (rd != exeWBdest && rd != memWBdest) {
                deRegNew.op1 = mwReg.out;
            }
        }
        else if (deRegNew.rs2 == rd) {
            if (rd != exeWBdest && rd != memWBdest) {
                deRegNew.op2 = mwReg.out;
            }
        }
        return;
    }
    if (singlestep) {
        printf("WB: Nothing\n");
    }
}