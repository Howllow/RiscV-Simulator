#include "Simulator.h"


char* filename = NULL;
bool single_step = false;
bool printinfo = false;
int strategy = 0;
bool Parser(int argc, char** argv);
Simulator* simulator;
ELFIO::elfio* elf_reader;
unsigned char** mem[1024]; //virtual memory of the simulator(first level pt entry)
int main(int argc, char** argv)
{
    for (int i = 0; i < 1024; i++)
        mem[i] = NULL;
    if (!Parser(argc, argv)) 
        return -1;
    simulator = new Simulator(single_step, strategy, printinfo);
    elf_reader = new ELFIO::elfio();
    read_ELF(filename, elf_reader);
    simulator->PC = elf_reader->get_entry();
    simulator->Run();

    return 0;
}

/*
 * Parameter parser
 */ 
bool Parser(int argc, char** argv)
{
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]){
                case 'p':
                printinfo = true;
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
        else if (filename == NULL)
            filename = argv[i];
    }
    return true;
}