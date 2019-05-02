#include "Simulator.h"
#include <fstream>
#include <cstring>
using namespace std;

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

void simulateCache(char* cachefile, CacheConfig config, ofstream &csv)
{
    Memory m;
    Cache c;
    c.SetLower(&m);
    StorageStats s;
    StorageLatency ml;
    ml.bus_latency = 0;
    ml.hit_latency = 100;
    StorageLatency cl;
    cl.bus_latency = 0;
    cl.hit_latency = 1;
    memset(&s, 0, sizeof(s));
    c.SetStats(s);
    c.SetLatency(cl);
    m.SetStats(s);
    m.SetLatency(ml);
    ifstream file(cachefile);
    if (!cachefile.is_open()) {
        printf("openfile error!\n");
        exit(-1);
    }
    char rw;
    unsigned addr;
    char tmp[256] = {0};
    int time;
    int hit;
    int totaltime = 0;
    while(file >> rw >> hex >> addr) {
        switch(type)
        {
            case 'r':
            c.HandleRequest(addr, 1, 1, tmp, hit, time);
            break;
            case 'w':
            c.HandleRequest(addr, 1, 0, tmp, hit, time);
            break;
            default:
            break;
        }
        totaltime += time;
    }
    float missrate = ((float)c.stats_.missnum) / c.stats_.access_counter;
    csv << config.capacity << "," << config.blocksize << "," << config.associativity << "," <<
    config.write_through << "," << config.write_allocate << "," << missrate << "," << totaltime 
    << endl;
}
void CacheTest(char* cachefile, int testnum) 
{
    for (unsigned int ad = 0; ad < Max_Addr; ad++) {
		if (!check_Page(ad))
			get_Page(ad);
		setB(ad, 0);
	}
    switch(testnum) 
    {
        case 1:
        ofstream csv((string)cachefile + '.csv');
        csv << "Capacity,BlockSize,Associativity,WriteThrough,WriteAllocate,MissRate,TotalTime\n";
        for (csz = 32768; csz <= 32768 * 1024; csz *= 2){
            for (int ass = 1; ass <= 32; ass *= 2) {
                for (bsz = 1; bsz <= 4096; bsz *= 2) {
                    CacheConfig* config1;
                    CacheConfig* config2;
                    CacheConfig* config3;
                    CacheConfig* config4;
                    config1 = new CacheConfig(bsz, ass, csz, 0, 0);
                    config2 = new CacheConfig(bsz, ass, csz, 1, 0);
                    config3 = new CacheConfig(bsz, ass, csz, 0, 1);
                    config4 = new CacheConfig(bsz, ass, csz, 1, 1);
                    simulateCache(cachefile, config1, csv);
                    simulateCache(cachefile, config2, csv);
                    simulateCache(cachefile, config3, csv);
                    simulateCache(cachefile, config4, csv);
                }
            }
        }
        csv.close();
        break;
        default:
        exit(-1);
    }

    exit(0);
}

/*
 * Parameter parser
 */ 
bool Parser(int argc, char** argv)
{
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]){
                case 'c':
                CacheTest(argv[i + 1], atoi(argv[i + 2]));
                break;
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