#ifndef MM_H
#define MM_H

#include "machine.h"

#define GET_FPN(addr) (addr >> 22) & 0x3FF // get first level page table entry id
#define GET_SPN(addr) (addr >> 12) & 0x3FF // get second level page table entry id
#define GET_OFF(addr) addr & 0xFFF
void init_Memory();
bool check_Page(uint64_t addr);
int get_Page(uint64_t addr);
void setB(uint64_t addr, uint8_t val);
char getB(uint64_t addr);
#endif