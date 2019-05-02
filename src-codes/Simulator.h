#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "MM.h"

class Simulator {
   public:
    unsigned long long cycles;
    unsigned long long insts;
    unsigned long long goodPredict;
    unsigned long long badPredict;
    unsigned long long memLoadHazard;
    unsigned long long controlHazard;
    unsigned long long reg[REG_NUM];
    unsigned long long PC;
    unsigned long long PC_taken;
    unsigned long long PC_not_taken;
    int exeWBdest;
    int memWBdest;
    bool stall;
    int strategy;
    bool ifprint;
    bool singlestep;
    bool usecache;
    Simulator(bool cache, bool singlestep, int strategy, bool ifprint);
    ~Simulator();

    void Run();
    bool predictBranch();
    void Print();
    int64_t syscall(int64_t a7, int64_t a0) ;

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
      long long offset;
      unsigned long long PC;
      bool takeBranch;
      int imm;
      int rs1, rs2;
      int rd;
      int64_t op1, op2;
    } deReg, deRegNew;

    struct Execute_to_Memory
    {
      bool bubble;
      int64_t op2, out;
      bool wMem, rMem; // read or write signals
      bool signExt; // if need to use sign extension
      int rd;
      int memsize;
    } emReg, emRegNew;

    struct Memory_to_WB
    {
      bool bubble;
      int64_t out;
      int rd;
    } mwReg, mwRegNew;

    void fetch();
    void decode();
    void execute();
    void memory();
    void writeBack();

        
};

#endif