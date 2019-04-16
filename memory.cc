#include "Simulator.h"

void
StoreBySize(uint32_t addr, uint64_t val, int memsize)
{
    if (!check_Page(addr)) {
        printf("Invalid address!\n");
    }
    uint32_t first_id = GET_FPN(addr);
    uint32_t second_id = GET_SPN(addr);
    uint32_t offset = GET_OFF(addr);
    memcpy(&mem[first_id][second_id][offset], &val, memsize);
}

uint64_t
LoadBySize(uint32_t addr, int memsize)
{
    uint64_t x = 0;
    for (int i = 0; i < memsize; i++) {
        x += (((uint64_t)getB(addr + i)) << (i * 8));
    }
    return x;
}

void
Simulator::memory()
{
    if (emReg.bubble) {
        if (ifprint)
            printf("MEM: Bubble\n");
        mwRegNew.bubble = true;
        return;
    }
    int64_t op1 = emReg.op1;
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