#include "Simulator.h"

void
Simulator::memory()
{
    if (emReg.bubble) {
        if (ifprint)
            printf("MEM: Bubble\n");
        mwRegNew.bubble = true;
        return;
    }
    uint64_t op2 = emReg.op2;
    int64_t addr = emReg.out;
    int64_t out = 0;
    int rd = emReg.rd;

    if (emReg.wMem) {
        if (ifprint)
            printf("MEM: write size:%d value: 0x%.16llx at 0x%llx\n", emReg.memsize, (uint64_t)op2, addr);
        StoreBySize(addr, op2, emReg.memsize);
        mwRegNew.out = addr;
    }

    if (emReg.rMem) {
        out = LoadBySize((uint32_t)addr, emReg.memsize);
        if (emReg.signExt)
            out = (int64_t)out;
        else 
            out = (uint64_t) out;

        mwRegNew.out = out;
        if (ifprint)
            printf("MEM: read value 0x%.8llx from mem 0x%llx\n", out, addr);
    }

    else if (!emReg.wMem && !emReg.rMem){
        mwRegNew.out = addr;
        if (ifprint)
            printf("MEM: Nothing\n");
    }

    //data hazard, avoid overwritten
    if (rd != 0 && rd != -1) {
        if (rd == deRegNew.rs1) {
            if (exeWBdest != rd) {
                memWBdest = rd;
                deRegNew.op1 = mwRegNew.out;
                if (ifprint)
                    printf("forwarding 0x%llx to reg:x%d\n", mwRegNew.out, rd);
            }
        }
        else if (rd == deRegNew.rs2) {
            if (exeWBdest != rd) {
                memWBdest = rd;
                deRegNew.op2 = mwRegNew.out;
                if (ifprint)
                    printf("forwarding 0x%llx to reg:x%d\n", mwRegNew.out, rd);
            }
        }
    }

    mwRegNew.bubble = false;
    mwRegNew.rd = rd;
}