#ifndef READELF_H
#define READELF_H

#include <elfio/elfio.hpp>

bool read_ELF(char* filename, ELFIO::elfio* elf_reader);

#endif
