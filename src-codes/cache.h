#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
#include <cstring>
#include <stdio.h>
#include "storage.h"

 
struct CacheBlock {
  bool valid;
  bool dirty;
  int lru;
  uint64_t tag;
  char data[256];
  CacheBlock() {
    valid = false;
    dirty = false;
    lru = 0;
  }
};
class CacheConfig {
  public:
    int blocksize;
    int replace_strategy;
    int capacity;
    int associativity;
    int blocknum;
    int prefetch;
    int set_num; // Number of cache sets
    int write_through; // 0|1 for back|through
    int write_allocate; // 0|1 for no-alc|alc
    int bypass_possi;
    CacheBlock* blocks;
    CacheConfig() {}
    CacheConfig(CacheBlock* b, int blocksize, int assoc, int capacity, int writet, int writea, 
      int stra = 0, int prefetch = 0, int bypass = 0) {
      this->blocksize = blocksize;
      blocks = b;
      associativity = assoc;
      this->capacity = capacity;
      this->set_num = capacity / (assoc * blocksize);
      this->blocknum = this->set_num * assoc;
      write_allocate = writea;
      write_through = writet;
      replace_strategy = stra;
      bypass_possi = bypass;
      this->prefetch = prefetch;
    }
};

typedef
class Cache: public Storage {
 public:
  Cache() {}
  ~Cache() {}
  
  // Sets & Gets
  void SetConfig(CacheConfig cc);
  void GetConfig(CacheConfig cc);
  void SetLower(Storage *ll) { lower_ = ll; }
  Storage* GetLower() {return lower_;}
  // Main access process
  void HandleRequest(uint32_t addr, int bytes, int read,
                     char *content, int &hit, int &time);
  Storage *lower_;

 private:
  // Bypassing
  int BypassDecision();
  // Partitioning
  void PartitionAlgorithm();
  // Replacement
  int ReplaceDecision();
  void ReplaceAlgorithm();
  // Prefetching
  int PrefetchDecision();
  void PrefetchAlgorithm();
  int CheckHit(unsigned set, unsigned tag);
  int CheckFull(unsigned set);
  int FindPos(unsigned set);
  CacheConfig config_;
  DISALLOW_COPY_AND_ASSIGN(Cache);
};

#endif //CACHE_CACHE_H_ 
