#include "MM.h"
using namespace ELFIO;

bool read_ELF(char* filename, elfio* elf_reader)
{
    if (!elf_reader->load(filename)){
            printf("Load ELF Failed!! Check Your path!!\n");
            return false;
        }
    Elf_Half seg_num = elf_reader->segments.size();
    while(seg_num--) {
        segment* seg = elf_reader->segments[seg_num];
        uint32_t memory_size = seg->get_memory_size();
        uint32_t v_addr = (uint32_t)seg->get_virtual_address();
        uint32_t file_size = seg->get_file_size();
        for (uint32_t x = 0; x < memory_size; x++) {
            uint32_t ad = x + v_addr;
            if (!check_Page(ad))
                get_Page(ad);
            uint32_t first = GET_FPN(ad);
            uint32_t second = GET_SPN(ad);
            uint32_t offset = GET_OFF(ad);
            if (x < file_size)
                mem[first][second][offset] = seg->get_data()[x];
            else {
                mem[first][second][offset] = 0;
            }
        }        
    }
    return true;
}

bool check_Page(uint32_t addr)
{
    uint32_t first_id = GET_FPN(addr);
    uint32_t second_id = GET_SPN(addr);
    if (mem[first_id] && mem[first_id][second_id]) {
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
    if (!mem[first_id]) {
        mem[first_id] = new unsigned char* [1024];
        memset(mem[first_id], 0, 1024);
    }
    if (!mem[first_id][second_id]) {
        mem[first_id][second_id] = new unsigned char[4 * 1024];
        memset(mem[first_id][second_id], 0, 4 * 1024);
    }
    return 1;
}

void setB(uint32_t addr, uint8_t val)
{
    uint32_t first_id = GET_FPN(addr);
    uint32_t second_id = GET_SPN(addr);
    uint32_t offset = GET_OFF(addr);
    mem[first_id][second_id][offset] = val;
}

uint8_t getB(uint32_t addr)
{
    uint32_t first_id = GET_FPN(addr);
    uint32_t second_id = GET_SPN(addr);
    uint32_t offset = GET_OFF(addr);
    return mem[first_id][second_id][offset];
}

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


void Memory::HandleRequest(uint32_t addr, int bytes, int read, char* content, int &hit, int &time) 
{
    if (read == 1) {
        uint32_t first_id = GET_FPN(addr);
        uint32_t second_id = GET_SPN(addr);
        uint32_t offset = GET_OFF(addr);
        memcpy(content, &mem[first_id][second_id][offset], bytes);
        /*for (int i = 0; i < bytes; i++) {
            if (!check_Page(addr + i))
                get_Page(addr + i);
            *(content + i) = getB(addr + i);
        }*/
    }
        
    else {
        if (!check_Page(addr)) {
            get_Page(addr);
            //printf("Invalid address!\n");
        }
        uint32_t first_id = GET_FPN(addr);
        uint32_t second_id = GET_SPN(addr);
        uint32_t offset = GET_OFF(addr);
        memcpy(&mem[first_id][second_id][offset], content, bytes);
    }
    hit = 1;
    time = latency_.hit_latency + latency_.bus_latency;
    stats_.access_time += time;
}