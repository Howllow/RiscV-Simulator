#include "machine.h"
#include "MM.h"

extern uint8_t** memory[1024];

bool check_Page(uint32_t addr)
{
    uint32_t first_id = GET_FPN(addr);
    uint32_t second_id = GET_SPN(addr);
    if (memory[first_id] && memory[first_id][second_id]) {
        return true;
    }
    return false;
}

int get_Page(uint32_t addr)
{
    uint32_t first_id = GET_FPN(addr);
    uint32_t second_id = GET_SPN(addr);
    bool exist = check_Page(addr);
    if (exist) {
        return 0;
    }
    if (!memory[first_id]) {
        memory[first_id] = new uint8_t* [1024];
        memset(memory[first_id], 0, 1024);
    }
    if (!memory[first_id][second_id]) {
        memory[first_id][second_id] = new uint8_t[4 * 1024];
        memset(memory[first_id][second_id], 0, 4 * 1024);
    }
    return 1;
}

void init_Memory()
{

}