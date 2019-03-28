#ifndef MACHINE_H
#define MACHINE_H

#include <cstdint>
#include <cstdio>
#include <cstring>

#define REG_NUM 32
#define Stack_Base 0x80000000
#define Stack_Size 0x400000
#define Max_Addr 0xFFFFFFFF
uint8_t** memory[1024]; //virtual memory of the simulator(first level pt entry)

#endif