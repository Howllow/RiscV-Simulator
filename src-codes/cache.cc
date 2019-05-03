#include "cache.h"
#include "math.h"
#include <cstdlib>
#include <ctime>

void Cache::SetConfig(CacheConfig cc) {
  config_ = cc;
}
void Cache::HandleRequest(uint32_t addr, int bytes, int read,
                          char* content, int &hit, int &time) {
  hit = 0;
  time = 0;
  time += latency_.bus_latency + latency_.hit_latency;
  stats_.access_time += time;
  stats_.access_counter += 1;
  unsigned b_bits = (int)log2(config_.blocksize);
  unsigned s_bits = (int)log2(config_.set_num);
  unsigned t_bits = 32 - b_bits - s_bits;
  unsigned tag = (addr >> (b_bits + s_bits)) & ((1 << t_bits) - 1);
  unsigned set = (addr >> b_bits) & ((1 << s_bits) - 1);
  unsigned off = addr & ((1 << (int)log2(config_.blocksize)) - 1);
  int hit_pos = -1;
  int replace_pos = -1; 

  //get data or store data
  hit_pos = CheckHit(tag, set);
  //miss
  if (hit_pos == -1) {
    stats_.miss_num++;
    if (BypassDecision) {
      Storage* m = this;
      //find memory
      while(m->lower_ != NULL) {
        m = m->lower_;
      }
      time += latency_.bus_latency + 25;
      //read bypassing the cache
      if (read) {
        m->HandleRequest(addr, bytes, 1, content, hit, time);
      }
      else {
        m->HandleRequest(addr, bytes, 0, content, hit, time);
      }
      return;
    }
    replace_pos = CheckFull(set);
    if (replace_pos == -1) {
      //get replace pos
      replace_pos = FindPos(set);
    }
  }
  //hit
  else {
    if (!read) {
      memcpy(config_.blocks[hit_pos].data + off, content, bytes);
      //write through
      if (config_.write_through) {
        int lhit, ltime;
        lower_->HandleRequest(addr, bytes, 0, content, lhit, ltime);
        stats_.access_time += latency_.bus_latency;
        time += latency_.bus_latency + ltime;
        return;
      }
      //write back
      else {
        config_.blocks[hit_pos].dirty = true;
      }
    }
    else {
      memcpy(content, config_.blocks[hit_pos].data + off, bytes);
      return;
    }
  }
  // Prefetch?
  if (PrefetchDecision()) {
    unsigned startaddr = addr - off;
    for (int i = 1; i <= config_.prefetch; i++) {
      unsigned blockaddr = startaddr + i * config_.blocksize;
      char tmp[256];
      int tmphit;
      int tmptime;
      int tmp_hitpos = -1;
      int tmp_placepos = -1;
      unsigned btag = (blockaddr >> (b_bits + s_bits)) & ((1 << t_bits) - 1);
      unsigned bset = (blockaddr >> b_bits) & ((1 << s_bits) - 1);
      tmp_hitpos = CheckHit(bset, btag);
      if (tmp_hitpos != -1)
        continue;
      else {
        tmp_placepos = CheckFull(bset);
        if (tmp_placepos == -1)
          tmp_placepos = FindPos(bset);
      }
      lower_->HandleRequest(blockaddr, config_.blocksize, 1, tmp, tmphit, tmptime);
      CacheBlock &rblock = config_.blocks[tmp_placepos];
      if (rblock.valid && rblock.dirty) {
        unsigned rep_addr = (rblock.tag << (32 - t_bits)) + (bset << b_bits);
        lower_->HandleRequest(rep_addr, config_.blocksize, 0, rblock.data, tmphit, tmptime);
      }
      rblock.valid = true;
      rblock.lru = 0;
      rblock.dirty = false;
      rblock.tag = btag;
      memcpy(rblock.data, tmp, config_.blocksize);
    }
    
  }
    int lower_hit, lower_time;
    if (read) {
      // Fetch from lower layer, already miss
      lower_->HandleRequest(addr - off, config_.blocksize, 1, content,
                            lower_hit, lower_time);
      hit = 0;
      time += latency_.bus_latency + lower_time;
      stats_.access_time += latency_.bus_latency;
      CacheBlock &lblock = config_.blocks[replace_pos]; //victim
      //dirty block
      if (lblock.valid && lblock.dirty) {
        unsigned rep_addr = (lblock.tag << (32 - t_bits)) + (set << b_bits);
        lower_->HandleRequest(rep_addr, config_.blocksize, 0, lblock.data, lower_hit, lower_time);
        stats_.access_time += latency_.bus_latency;
        time += latency_.bus_latency + lower_time;
      }
      lblock.valid = true;
      lblock.lru = 0;
      lblock.dirty = false;
      lblock.tag = tag;
      //place the block in this cache
      memcpy(lblock.data, content, config_.blocksize);
      //get the data into content
      memcpy(content, lblock.data + off, bytes);
      return;
    }
    //case write, already miss
    else {
      // write allocate, need to read from lower cache
      if (config_.write_allocate) {
          char ldata[256];
          lower_->HandleRequest(addr - off, config_.blocksize, 1, ldata,
                            lower_hit, lower_time);
          time += latency_.bus_latency + lower_time;
          stats_.access_time += latency_.bus_latency;
          CacheBlock &lblock = config_.blocks[replace_pos]; //victim
          //dirty block
          if (lblock.valid && lblock.dirty) {
            unsigned rep_addr = (lblock.tag << (32 - t_bits)) + (set << b_bits);
            lower_->HandleRequest(rep_addr, config_.blocksize, 0, lblock.data, lower_hit, lower_time);
            stats_.access_time += latency_.bus_latency;
            time += latency_.bus_latency + lower_time;
          }
        lblock.valid = true;
        lblock.lru = 0;
        lblock.dirty = true;
        lblock.tag = tag;
        //place the block from lower level
        memcpy(lblock.data, ldata, config_.blocksize);
        //write content into block
        memcpy(lblock.data + off, content, bytes);
      }
      // no write allocate, write to storage directly
      else {
        lower_->HandleRequest(addr, bytes, 0, content, lower_hit, lower_time);
        time += latency_.bus_latency + lower_time;
        stats_.access_time += latency_.bus_latency;
      }
    }
}

int Cache::BypassDecision() {
  srand((unsigned)time(NULL));
  unsigned r = rand() % 100;
  if (r < config_.bypass_possi)  
    return true;
  else return false;
}

void Cache::PartitionAlgorithm() {
}

int Cache::ReplaceDecision() {
  return true;
  //return FALSE;
}

void Cache::ReplaceAlgorithm(){
}

int Cache::PrefetchDecision() {
  return config_.prefetch;
}

void Cache::PrefetchAlgorithm() {
}

int Cache::CheckHit(unsigned set, unsigned tag)
{
  int hit_pos = -1;
  for (int i = 0; i < config_.associativity; i++) {
      int id = set * config_.associativity + i;
      if (config_.blocks[id].valid && tag == config_.blocks[id].tag) {
        hit_pos = id;
        config_.blocks[id].lru = 0;
      }
      else if (config_.blocks[id].valid && tag !=  config_.blocks[id].tag)
        config_.blocks[id].lru++;
  }
  return hit_pos;
}

int Cache::CheckFull(unsigned set)
{
  int replace_pos = -1;
  for (int i = 0; i < config_.associativity; i++) {
        int id = set * config_.associativity + i;
        if (!config_.blocks[id].valid) {
          replace_pos = id;
          break;
      }
  }
  return replace_pos;
}

int Cache::FindPos(unsigned set)
{
    int replace_pos;
    stats_.replace_num ++;
    // if FIFO
    if (config_.replace_strategy) {
      for (int i = 0; i < config_.associativity - 1; i++) {
        int id = set * config_.associativity + i;
        config_.blocks[id] = config_.blocks[id + 1];
      }
      replace_pos = (set + 1) * config_.associativity - 1;
    }
    // if LRU
    else {
      int max = -1;
      for (int i = 0; i < config_.associativity; i++) {
        int id = set * config_.associativity + i;
        if (config_.blocks[id].lru > max) {
          max = config_.blocks[id].lru;
          replace_pos = id;
        }
      }
    }
  return replace_pos;
}

