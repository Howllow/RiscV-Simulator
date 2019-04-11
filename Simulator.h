#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "machine.h"


class Simulator {
   public:
    uint64_t reg[REG_NUM];
    uint64_t PC;

    Simulator();
    ~Simulaor();

    void Run();

   private:
    void fetch();
    void decode();
    void execute();
    void memory();
    void writeBack();

    // four pipeline registers
   	struct Fetch_to_Decode
    {
   		bool bubble;
   		bool stall;
   		unsigned int instruction;
      int PC;
   	} fdReg, fdRegNew;

    struct Decode_to_Execute
    {
      bool bubble;
      bool stall;
      int PC;
      int rs, rt, rd;
    } deReg, deRegNew;

    struct Execute_to_Memory
    {
      bool bubble;
      bool stall;
      int op1, op2, out;
      bool wReg, wMem, rMem; // read or write signals
      bool signExt; // if need to use sign extension
      int rd;
    } emReg, emRegNew;

    struct Memory_to_WB
    {
      bool bubble;
      bool stall;
      int out;
      bool wReg;
      int dest;
    } mwReg, mwRegNew;

        
};





#endif