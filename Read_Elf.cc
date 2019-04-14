#include <elfio/elfio.hpp>
#include <cstdio>
#include <cstdint>

#include "machine.h"
#include "Read_Elf.h"
#include "MM.h"

using namespace ELFIO;

extern char** memory[1024];

bool read_ELF(char* filename, elfio* elf_reader)
{
    if (!elf_reader->load(filename)){
            printf("Load ELF Failed!! Check Your path!!\n");
            return false;
        }
    Elf_Half seg_num = elf_reader->segments.size();

    while(seg_num--) {
        segment* seg = elf_reader->segments[seg_num - 1];
        uint32_t memory_size = seg->get_memory_size();
        uint32_t v_addr = seg->get_virtual_address();
        uint32_t file_size = seg->get_file_size();
        for (uint32_t x = 0; x < memory_size; x++) {
            uint32_t ad = x + v_addr;
            if (!check_Page(ad))
                get_Page(ad);
            uint32_t first = GET_FPN(ad);
            uint32_t second = GET_SPN(ad);
            uint32_t offset = GET_OFF(ad);
            if (x < file_size)
                memory[first][second][offset] = seg->get_data()[x];
            else
                memory[first][second][offset] = 0;
        }        
    }
    return true;
}