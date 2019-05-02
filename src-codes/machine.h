#ifndef MACHINE_H
#define MACHINE_H

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <cmath>
#include <elfio/elfio.hpp>
#include "cache.h"

#define REG_NUM 32
#define Stack_Base 0xc0000000
#define Stack_Size 0x400000
#define Max_Addr 0xFFFFFFFF

// registers
#define REG_ZERO 0
#define REG_RA  1
#define REG_SP  2
#define REG_GP  3
#define REG_TP  4
#define REG_T0  5
#define REG_T1  6
#define REG_T2  7
#define REG_S0  8
#define REG_S1  9
#define REG_A0  10
#define REG_A1  11
#define REG_A2  12
#define REG_A3  13
#define REG_A4  14
#define REG_A5  15
#define REG_A6  16
#define REG_A7  17
#define REG_S2  18
#define REG_S3  19
#define REG_S4  20
#define REG_S5  21  
#define REG_S6  22
#define REG_S7  23
#define REG_S8  24
#define REG_S9  25
#define REG_S10  26
#define REG_S11  27
#define REG_T3  28
#define REG_T4  29
#define REG_T5  30
#define REG_T6  31

//opcode
#define OP_R 0x33
#define OP_LUI 0x37
#define OP_STORE 0x23
#define OP_LOAD 0x03
#define OP_BRANCH 0x63
#define OP_AUI 0x17
#define OP_JAL 0x6F
#define OP_JALR 0x67
#define OP_I 0x13
#define OP_SYS 0x73
#define OP_I32 0x1B
#define OP_R32 0x3B

//instructions
#define LUI 0
#define AUIPC 1
#define JAL 2
#define JALR 3
#define BEQ 4
#define BNE 5
#define BLT 6
#define BGE 7
#define BLTU 8
#define BGEU 9
#define LB 10
#define LH 11
#define LW 12
#define LBU 13
#define LHU 14
#define SB 15
#define SH 16
#define SW 17
#define ADDI 18
#define SLTI 19
#define SLTIU 20
#define XORI 21
#define ORI 22
#define ANDI 23
#define SLLI 24
#define SRLI 25
#define SRAI 26
#define ADD 27
#define SUB 28
#define SLL 29
#define SLT 30
#define SLTU 31
#define XOR 32
#define SRL 33
#define SRA 34
#define OR 35
#define AND 36
#define ECALL 37
#define LWU 38
#define LD 39
#define SD 40
#define ADDIW 41
#define SLLIW 42
#define SRLIW 43
#define SRAIW 44
#define ADDW 45
#define SUBW 46
#define SLLW 47
#define SRLW 48
#define SRAW 49
#define MUL 50
#define MULH 51
#define DIV 52
#define REM 53
#define UNK 54
extern unsigned char** mem[1024];
Cache l1;
Cache l2;
Cache llc;
#endif