#include <elfio/elfio.hpp>
#include <cstdio>
#include <cmath>
#include <string>

#include "machine.h"
#include "Read_Elf.h"
#include "Simulator.h"
#include "MM.h"


char* filename;
bool single_step = false;
bool print_elf = false;
int strategy = 0;
bool Parser(int argc, char** argv);
Simulator* simulator;
ELFIO::elfio* elf_reader;
int main(int argc, char** argv)
{
    if (!Parser(argc, argv)) 
        return -1;

    read_ELF(filename, elf_reader);
    simulator = new Simulator(single_step, strategy);
    simulator->PC = elf_reader->get_entry();
    simulator->Run();

    return 0;
}

/*
 * Parameter parser
 */ 
bool Parser(int argc, char** argv)
{
    filename = argv[0]; // load filename and use it for elfreader
    int i = 1;
    for (i; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]){
                case 'p':
                print_elf = true;
                break;
                case 's':
                single_step = true;
                break;
                case 'q':
                if (i + 1 < argc) {
                    std::string branchstr = argv[i + 1];
                    i++;
                    if (branchstr == "Always")
                        strategy = 0;
                    else if (branchstr == "Never")
                        strategy = 1;
                    else
                        return false;
                    break;
                }
                default:
                printf("Parameter Wrong!!\n");
                return false;
            }
        }
    }
    return true;
}