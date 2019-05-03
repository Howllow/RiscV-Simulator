#include "Simulator.h"
#include <fstream>
#include <cstring>
#include <ctime>
using namespace std;

char* filename = NULL;
bool single_step = false;
bool printinfo = false;
bool usecache = false;
int strategy = 0;
Cache l2;
Cache l1;
Cache llc;
CacheBlock blocks[100];
CacheBlock l1blocks[512];
CacheBlock l2blocks[4096];
CacheBlock llcblocks[131072];
Memory m;
bool Parser(int argc, char** argv);
Simulator* simulator;
ELFIO::elfio* elf_reader;
unsigned char** mem[1024]; //virtual memory of the simulator(first level pt entry)
int main(int argc, char** argv)
{
    srand((unsigned)time(0));
    for (int i = 0; i < 1024; i++)
        mem[i] = NULL;
    if (!Parser(argc, argv)) 
        return -1;
    simulator = new Simulator(usecache, single_step, strategy, printinfo);
    StorageStats stats;
    memset(&stats, 0, sizeof(stats));
    m.SetStats(stats);
    l1.SetStats(stats);
    l2.SetStats(stats);
    llc.SetStats(stats);
    StorageLatency l1la;
    StorageLatency l2la;
    StorageLatency llcla;
    StorageLatency mla;
    l1la.bus_latency = 0;
    l1la.hit_latency = 1;
    l2la.bus_latency = 0;
    l2la.hit_latency = 8;
    llcla.bus_latency = 0;
    llcla.hit_latency = 20;
    mla.bus_latency = 0;
    mla.hit_latency = 25;
    CacheConfig l1config = CacheConfig(l1blocks, 64, 8, 32768, 0, 0);
    CacheConfig l2config = CacheConfig(l2blocks, 64, 8, 32768 * 8, 0, 0);
    CacheConfig llcconfig = CacheConfig(llcblocks, 64, 8, 8 * 1024 * 1024, 0, 0);
    l1.SetConfig(l1config);
    l2.SetConfig(l2config);
    llc.SetConfig(llcconfig);
    l1.SetLatency(l1la);
    l2.SetLatency(l2la);
    llc.SetLatency(llcla);
    m.SetLatency(mla);
    l1.SetLower(&l2);
    l2.SetLower(&llc);
    llc.SetLower(&m);
    elf_reader = new ELFIO::elfio();
    read_ELF(filename, elf_reader);
    simulator->PC = elf_reader->get_entry();
    simulator->Run();

    return 0;
}

void simulateCache(char* cachefile, int bsz, int ass, int csz, int wt, int wa, ofstream &csv)
{
    Memory m;
    Cache c;
    c.SetLower(&m);
    CacheConfig config = CacheConfig(blocks, bsz, ass, csz, wt, wa);
    c.SetConfig(config);
    StorageStats s;
    StorageLatency ml;
    CacheBlock* todelete = config.blocks;
    ml.bus_latency = 0;
    ml.hit_latency = 20;
    StorageLatency cl;
    cl.bus_latency = 0;
    cl.hit_latency = 1;
    memset(&s, 0, sizeof(s));
    c.SetStats(s);
    c.SetLatency(cl);
    m.SetStats(s);
    m.SetLatency(ml);
    ifstream file(cachefile);
    if (!file.is_open()) {
        printf("openfile error!\n");
        exit(-1);
    }
    char rw;
    unsigned addr;
    char tmp[256] = {0};
    int time;
    int hit;
    long long totaltime = 0;
    while(file >> rw >> hex >> addr) {
        switch(rw)
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

    memset(blocks, 0, sizeof(blocks));
    float missrate = ((float)c.stats_.miss_num) / c.stats_.access_counter;
    csv << csz << "," << bsz << "," << ass << "," <<
    wt << "," << wa << "," << missrate << "," << totaltime 
    << endl;
    return;
}

void OptimizeCache(char* cachefile)
{
    Memory m;
    StorageLatency l1la;
    StorageLatency l2la;
    StorageLatency mla;
    l1la.bus_latency = 0;
    l2la.bus_latency = 12;
    l1la.hit_latency = 3;
    l2la.hit_latency = 4;
    mla.hit_latency = 75;
    mla.bus_latency = 0;
    l1.SetLatency(l1la);
    l2.SetLatency(l2la);
    m.SetLatency(mla);
    l1.SetLower(&l2);
    l2.SetLower(&m);
    char rw;
    unsigned addr;
    int time;
    int hit;
    int totalcycles = 0;
    char tmp[256] = {0};
    for (int prel1 = 0; prel1 < 5; prel1++)
        for (int prel2 = 0; prel2 < 5; prel2++) {
            CacheConfig l1c = CacheConfig(l1blocks, 64, 8, 32 * 1024, 0, 1, 0, prel1, 0);
            CacheConfig l2c = CacheConfig(l2blocks, 64, 8, 256 * 1024, 0, 1, 0, prel2, 0);
            StorageStats s;
            memset(&s, 0, sizeof(s));
            l1.SetStats(s);
            l2.SetStats(s);
            l1.SetConfig(l1c);
            l2.SetConfig(l2c);
            ifstream file(cachefile);
            if (!file.is_open()) {
                printf("openfile error!\n");
                exit(-1);
            }
            while(file >> rw >> hex >> addr) {
                switch(rw)
                {
                    case 'r':
                    l1.HandleRequest(addr, 1, 1, tmp, hit, time);
                    break;
                    case 'w':
                    l1.HandleRequest(addr, 1, 0, tmp, hit, time);
                    break;
                    default:
                    break;
                }
                totalcycles += time;
            }
        float missrate1 = ((float)l1.stats_.miss_num) / l1.stats_.access_counter;
        float missrate2 = ((float)l2.stats_.miss_num) / l2.stats_.access_counter;
        float amat = 7.5 * (1 - missrate1) + 9.5 * missrate1 * (1 - missrate2) + 59.5 * missrate1 * missrate2;
        cout << "------L1 prefetch:" << prel1 << " Blocks, L2 prefetch:" << prel2 <<" Blocks------"
        << endl;
        cout << "L1 missrate:" << missrate1 << endl;
        cout << "L2 missrate:" << missrate2 << endl;
        cout << "AMAT:"<< amat << endl;
        cout << "Total Cycles:" << totalcycles << endl;
        memset(l1blocks, 0, sizeof(l1blocks));
        memset(l2blocks, 0, sizeof(l2blocks));
        totalcycles = 0;
    }
}
void CacheTest(char* cachefile, int testnum) 
{
    for (unsigned int ad = 0; ad < 0x200000; ad++) {
		if (!check_Page(ad))
			get_Page(ad);
		setB(ad, 0);
	}
    ofstream csv((string)cachefile + ".small.csv");
    switch(testnum) 
    {
        case 1:
        csv << "Capacity,BlockSize,Associativity,WriteThrough,WriteAllocate,MissRate,TotalTime\n";
        for (int csz = 32768; csz <= 32768 * 1024; csz *= 2){
            for (int ass = 1; ass <= 32; ass *= 2) {
                for (int bsz = 1; bsz <= 256; bsz *= 2) {
                    int blocknum = csz / bsz;
                    if (blocknum % ass)
                        continue;
                    simulateCache(cachefile, bsz, ass, csz, 0, 0, csv);
                    simulateCache(cachefile, bsz, ass, csz, 0, 1, csv);
                    simulateCache(cachefile, bsz, ass, csz, 1, 0, csv);
                    simulateCache(cachefile, bsz, ass, csz, 1, 1, csv);
                }
            }
        }
        csv.close();
        break;
        case 2:
        OptimizeCache(cachefile);
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
                case 'u':
                usecache = true;
                break;
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