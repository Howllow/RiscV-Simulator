#ifndef READELF_H
#define READELF_H

#include <elfio/elfio.hpp>

ELFIO::elfio* elf_reader;
bool read_ELF(char* filename);

#endif
