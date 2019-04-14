#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "machine.h"


class Simulator {
   public:
    unsigned long long reg[REG_NUM];
    unsigned long long PC;
    bool executeWB;
    bool memoryWB;
    int stall;
    int strategy;
    Simulator();
    ~Simulator();

    void Run();
    bool predictBranch();

   private:
     // four pipeline registers
    struct Fetch_to_Decode
    {
      bool bubble;
      unsigned instruction;
      unsigned long long PC;
    } fdReg, fdRegNew;

    struct Decode_to_Execute
    {
      bool bubble;
      int op_type;
      int offset;
      bool takeBranch;
      unsigned long long PC;
      unsigned long long PC_taken;
      unsigned long long PC_not_taken;
      int imm;
      unsigned rs1, rs2, rd;
      int64_t op1, op2;
    } deReg, deRegNew;

    struct Execute_to_Memory
    {
      bool bubble;
      int64_t op1, op2, out;
      bool wReg, wMem, rMem; // read or write signals
      bool signExt; // if need to use sign extension
      unsigned rd;
    } emReg, emRegNew;

    struct Memory_to_WB
    {
      bool bubble;
      int out;
      bool wReg;
      unsigned rd;
    } mwReg, mwRegNew;

    void fetch();
    void decode();
    void execute();
    void memory();
    void writeBack();

        
};





#endif