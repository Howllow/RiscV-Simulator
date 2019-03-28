#include <elfio/elfio.hpp>
#include <cstdio>
#include <cmath>

#include "machine.h"
#include "Read_Elf.h"
#include "MM.h"


char* filename;
bool single_step = false;
bool print_elf = false;
bool Parser(int argc, char** argv);

int main(int argc, char** argv)
{
    if (!Parser(argc, argv)) 
        return -1;

    read_ELF(filename);

    


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
                default:
                printf("Parameter Wrong!!\n");
                return false;
            }
        }
    }
    return true;
}