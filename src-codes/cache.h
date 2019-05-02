#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdint.h>
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
}
class CacheConfig {
  public:
    int blocksize;
    int capacity;
    int associativity;
    int blocknum;
    int set_num; // Number of cache sets
    int write_through; // 0|1 for back|through
    int write_allocate; // 0|1 for no-alc|alc
    CacheBlock* blocks;
    CacheConfig() {blocks = new CacheBlock[50000];}
    CacheConfig(int blocksize, int assoc, int capacity, int writet, int writea) {
      this->blocksize = blocksize;
      associativity = assoc;
      this->capacity = capacity;
      this->set_num = capacity / (assoc * blocksize);
      this->blocknum = this->set_num * assoc;
      write_allocate = writea;
      write_through = writet;
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
  // Main access process
  void HandleRequest(uint64_t addr, int bytes, int read,
                     char *content, int &hit, int &time);

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

  CacheConfig config_;
  Storage *lower_;
  DISALLOW_COPY_AND_ASSIGN(Cache);
};

#endif //CACHE_CACHE_H_ 