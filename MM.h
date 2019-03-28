#ifndef MM_H
#define MM_H

#include "machine.h"

#define GET_FPN(addr) (addr >> 22) & 0x3FF // get first level page table entry id
#define GET_SPN(addr) (addr >> 12) & 0x3FF // get second level page table entry id
#define GET_OFF(addr) addr & 0xFFF
void init_Memory();
bool check_Page(uint32_t addr);
int get_Page(uint32_t addr);
#endif