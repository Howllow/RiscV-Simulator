#include "cache.h"
#include "math.h"

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
  //printf("%d, %d, %d\n", config_.blocksize, config_.set_num, config_.associativity);
  //printf("%d, %d, %d\n", b_bits, s_bits, t_bits);
  unsigned tag = (addr >> (b_bits + s_bits)) & ((1 << t_bits) - 1);
  unsigned set = (addr >> b_bits) & ((1 << s_bits) - 1);
  unsigned off = addr & ((1 << (int)log2(config_.blocksize)) - 1);
  //printf("0x%lx, 0x%lx, 0x%lx, 0x%lx\n", addr, tag, set, off);
  int hit_pos = -1;
  int replace_pos = -1; 

  //get data or store data
  if (!BypassDecision()) {
    bool valid;
    for (int i = 0; i < config_.associativity; i++) {
      int id = set * config_.associativity + i;
      valid = config_.blocks[id].valid;
      unsigned findtag = config_.blocks[id].tag;
      if (valid && tag == findtag) {
        hit = 1;
        hit_pos = id;
        config_.blocks[id].lru = 0;
      }
      else if (valid && tag != findtag)
        config_.blocks[id].lru++;
    }

    //miss
    if (!hit) {
      //printf("miss!\n");
      stats_.miss_num++;
      int max = -1;
      bool full = true;
      for (int i = 0; i < config_.associativity; i++) {
        int id = set * config_.associativity + i;
        if (config_.blocks[id].valid) {
          if (config_.blocks[id].lru > max) {
            max = config_.blocks[id].lru;
            replace_pos = id;
          }
        }
        else {
          replace_pos = id;
          full = false;
          break;
        }
      }
      if (full)
        stats_.replace_num ++;
    }
    //hit
    else {
      //printf("hit!\n");
      if (!read) {
        //printf("read!\n");
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
        //printf("write!\n");
        //printf("hitpos:%d\n", hit_pos);
        memcpy(content, config_.blocks[hit_pos].data + off, bytes);
        //printf("write successful!\n");
        return;
      }
    }


  }
  // Prefetch?
  if (PrefetchDecision()) {
    PrefetchAlgorithm();
  } else {
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
        lower_->HandleRequest(addr - off, bytes, 0, content, lower_hit, lower_time);
        time += latency_.bus_latency + lower_time;
        stats_.access_time += latency_.bus_latency;
      }
    }
  }

}

int Cache::BypassDecision() {
  return false;
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
  return false;
}

void Cache::PrefetchAlgorithm() {
}

