#include "machine.h"
#include "Simulator.h"
#include "MM.h"

void 
Simulator::memory()
{
    if (emReg.bubble) {
        if (singlestep)
            printf("MEM: Bubble\n");
        mwRegNew.bubble = true;
        return;
    }
    int64_t op1 = emReg.op1;
    int64_t op2 = emReg.op2;
    int64_t addr = emReg.out;
    int64_t out = 0;
    int rd = emReg.rd;

    if (emReg.wMem) {
        if (singlestep)
            printf("MEM: write mem at 0x%llx\n", addr);
        setGivenSize(addr, op2, emReg.memsize);
    }

    if (emReg.rMem) {
        if (singlestep)
            printf("MEM: read mem at 0x%llx\n", addr);
        for (int i = 0; i < emReg.memsize; i++) {
            out += (getB(addr + i) << (i * 8));
        }
        if (emReg.signExt)
            out = (int64_t)out;

        //data hazard, avoid overwritten
        if (rd != 0 && rd != -1) {
            if (rd = deRegNew.rs1) {
                if (exeWBdest != rd) {
                    memWBdest = rd;
                    deRegNew.op1 = out;
                }
            }
            else if (rd = deRegNew.rs2) {
                if (exeWBdest != rd) {
                    memWBdest = rd;
                    deRegNew.op2 = out;
                }
            }
        }
    }

    mwRegNew.bubble = false;
    mwRegNew.rd = rd;
    mwRegNew.out = out;
}