#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include <vector>
#include <math.h>
#include <bitset>


#define L "L"
#define S "S"

#define KB 1024

#define LEFT false
#define RIGHT true

// SET EXTRA CREDIT TO 1 TO RUN WITH EXTRA_CREDIT
#define EXTRA_CREDIT 0

using namespace std;

struct cacheLine {
  bool valid = false;
  unsigned long tag = 0;
  unsigned long time = 0;
};

struct cacheLine_EC {
  bool valid = false;
  unsigned long tag = 0;
  unsigned long time = 0;
  int numUses = 0;

};


class Node {
public:
  Node* parent = NULL;
  Node* left = NULL;
  Node* right = NULL;
  bool direction = LEFT;
  int value = -1;
};

// Binary tree
Node* head = NULL;
vector<Node*> cache_hotCold;


Node* fillTree(int level, Node* parent) {
  Node* node = new Node();
  node->parent = parent;

  if (level == 0) {
    cache_hotCold.push_back(node);
    return node;
  }

  node->left = fillTree(level - 1, node);
  node->right = fillTree(level - 1, node);

  return node;
}

// on cache hits, oscilate bits
void flipBits(Node * curr, Node * parent) {
  while (parent != NULL) {
    if (parent->left == curr && parent->direction == LEFT) parent->direction = RIGHT;
    else if (parent->right == curr && parent->direction == RIGHT) parent->direction = LEFT;

    curr = parent;
    parent = parent->parent;
  }
}

// called on cache misses, returns victim, and subsequently flips bits
void evictVictim(unsigned long tag) {
  Node* curr = head;
  Node* next = NULL;
  Node* victim = NULL;

  while (curr) {
    victim = curr;
    next = (curr->direction) ? curr->right : curr->left;
    curr->direction = (curr->direction) ? false : true;
    curr = next;
  }
  victim->value = tag;
}




vector<bool> heap(512, false);

// OLD IMPLEMEENTATION
// returns current victim and toggles bits - first evict 0, then 256, ... called on cache misses
// int pseudoLRU(int level, int index) {
//   int exponent = 9 - level;
//   if (level == 9) {
//     if (heap[index]) {
//       // toggle bit
//       heap[index] = false;
//       return 1;
//     }
//     else {
//       // toggle bit
//       heap[index] = true;
//       return 0;
//     }
//   }
//   if (heap[index]) {
//     // toggle bit
//     heap[index] = false;
//     return pow(2,exponent) + pseudoLRU(level+1, 2 * index + 2);
//   }
//   else {
//     // toggle bit
//     heap[index] = true;
//     return pseudoLRU(level+1, 2 * index + 1);
//   }
// }

// toggle bits at most recently used index ... called on cache hits
// void pseudoLRU(int index) {
//   int current = 0;
//   while (current <= index) {
//     if (heap[current]) {
//       heap[current] = false;
//       current = 2 * current + 2;
//     }
//     else {
//       heap[current] = true;
//       current = 2 * current + 1;
//     }
//   }
// }


int getLeastFreqUsed(int set, struct cacheLine_EC * cache, int size) {
  int leastIndex = 0;
  for (int i = 1; i < size; i++) {
    if (cache[i].numUses < cache[leastIndex].numUses) leastIndex = i;
    else {
      if (cache[i].numUses == cache[leastIndex].numUses) {
        if (cache[i].time < cache[leastIndex].time) leastIndex = i;
      }
    }
  }
  return leastIndex;
}

int getLRU(int set, struct cacheLine * cache, int size) {
  int index = 0;
  double least = 9999999; //lowest time value
  for (int i = 0; i < size; i++) {
    if (cache[i].time == 0) return index;
    if (cache[i].time < least) {
      least = cache[i].time;
      index = i;
    }
  }
  return index;
}

int getMRU(int set, struct cacheLine * cache, int size) {
  int index = 0;
  double max = -1; //lowest time value
  for (int i = 0; i < size; i++) {
    if (cache[i].time == 0) return index;
    if (cache[i].time > max) {
      max = cache[i].time;
      index = i;
    }
  }
  return index;
}



int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "correct arguments:\t./memory trace: <input file>.txt output file: <output file>.txt\n";
    return 1;
  }
  remove(argv[2]);
  ifstream infile(argv[1]);
  ofstream outputFile(argv[2], ios::trunc);


  unsigned long long hexAddress;
  string type, line;
  unsigned long total = 0;
  unsigned long tag;

  // DIRECT MAPPED CACHES
  // 32 BYTE LINE SIZE
  // 1, 4, 8, 16 KB
  int * directIndices = new int[4];
  int directCacheSizes[] = {32, 128, 512, 1024};
  unsigned long ** directCaches = new unsigned long*[4];
  unsigned long directHits[4] = {0,0,0,0};
  for (int i=0; i<4; i++) directCaches[i] = new unsigned long[directCacheSizes[i]];


  // 2-WAY SET ASSOCIATIVE
  unsigned long associativeHits = 0;
  int LRU[256] = {0};
  unsigned long associativeCache[256][2] = {0};
  int associativeSet; // (bits 24-26)
  unsigned long associativeTag;


  // 2-WAY SET ASSOCIATIVE NO ALLOCATION
  unsigned long associativeHits_noAlloc = 0;
  int LRU_noAlloc[256] = {0};
  unsigned long assocCache_noAlloc[256][2] = {0};
  int associativeSet_noAlloc; // (bits 24-26)
  unsigned long associativeTag_noAlloc;
  // ignore offset (last 5 bits)



  unsigned long numIterations = 1;


  // 4-WAY SET ASSOCIATIVE
  struct cacheLine cache_4way[128][4];
  int set_4way;
  unsigned long tag_4way;
  int index_4way = -1;
  unsigned long hits_4way = 0;

  // 8-WAY SET ASSOCIATIVE
  struct cacheLine cache_8way[64][8];
  int set_8way;
  unsigned long tag_8way;
  int index_8way = -1;
  unsigned long hits_8way = 0;

  // 16-WAY SET ASSOCIATIVE
  struct cacheLine cache_16way[32][16];
  int set_16way;
  unsigned long tag_16way;
  int index_16way = -1;
  unsigned long hits_16way = 0;

  // FULLY ASSOCIATIVE CACHE
  struct cacheLine cache_fullAssoc[512];
  unsigned long tag_fullAssoc;
  int index_fullAssoc = -1;
  unsigned long hits_fullAssoc = 0;

  //FULLY ASSOCIATIVE CACHE HOT/COLD BITS
  //struct cacheLine cache_hotCold[512];
  unsigned long tag_hotCold;
  int index_hotCold = -1;
  unsigned long hits_hotCold = 0;


  // N- WAY SET ASSOCIATIVE NO ALLOCATION
  // 4-WAY SET ASSOCIATIVE
  struct cacheLine cache_4way_noAlloc[128][4];
  int set_4way_noAlloc;
  unsigned long tag_4way_noAlloc;
  int index_4way_noAlloc = -1;
  unsigned long hits_4way_noAlloc = 0;

  // 8-WAY SET ASSOCIATIVE
  struct cacheLine cache_8way_noAlloc[64][8];
  int set_8way_noAlloc;
  unsigned long tag_8way_noAlloc;
  int index_8way_noAlloc = -1;
  unsigned long hits_8way_noAlloc = 0;

  // 16-WAY SET ASSOCIATIVE
  struct cacheLine cache_16way_noAlloc[32][16];
  int set_16way_noAlloc;
  unsigned long tag_16way_noAlloc;
  int index_16way_noAlloc = -1;
  unsigned long hits_16way_noAlloc = 0;



  // PRE-FETCHING ON CACHE ACCESS
  // 2-WAY
  struct cacheLine cache_2way_prefetch[256][2];
  int set_2way_prefetch;
  unsigned long tag_2way_prefetch;
  int index_2way_prefetch= -1;
  unsigned long hits_2way_prefetch = 0;
  unsigned int numIterations_2way_prefetch = 1;


  // 4-WAY
  struct cacheLine cache_4way_prefetch[128][4];
  int set_4way_prefetch;
  unsigned long tag_4way_prefetch;
  int index_4way_prefetch= -1;
  unsigned long hits_4way_prefetch = 0;
  unsigned int numIterations_4way_prefetch = 1;

  // 8-WAY
  struct cacheLine cache_8way_prefetch[64][8];
  int set_8way_prefetch;
  unsigned long tag_8way_prefetch;
  int index_8way_prefetch = -1;
  unsigned long hits_8way_prefetch = 0;
  unsigned int numIterations_8way_prefetch = 1;
  // 16-WAY
  struct cacheLine cache_16way_prefetch[32][16];
  int set_16way_prefetch;
  unsigned long tag_16way_prefetch;
  int index_16way_prefetch = -1;
  unsigned long hits_16way_prefetch = 0;
  unsigned int numIterations_16way_prefetch = 1;


  // PRE-FETCHING ON CACHE MISS
  // 2-WAY
  struct cacheLine cache_2way_prefetchMiss[256][2];
  int set_2way_prefetchMiss;
  unsigned long tag_2way_prefetchMiss;
  int index_2way_prefetchMiss = -1;
  unsigned long hits_2way_prefetchMiss = 0;
  unsigned int numIterations_2way_prefetchMiss = 1;
  bool isMiss_2way = false;

  // 4-WAY
  struct cacheLine cache_4way_prefetchMiss[128][4];
  int set_4way_prefetchMiss;
  unsigned long tag_4way_prefetchMiss;
  int index_4way_prefetchMiss= -1;
  unsigned long hits_4way_prefetchMiss = 0;
  unsigned int numIterations_4way_prefetchMiss = 1;
  bool isMiss_4way = false;
  // 8-WAY
  struct cacheLine cache_8way_prefetchMiss[64][8];
  int set_8way_prefetchMiss;
  unsigned long tag_8way_prefetchMiss;
  int index_8way_prefetchMiss = -1;
  unsigned long hits_8way_prefetchMiss = 0;
  unsigned int numIterations_8way_prefetchMiss = 1;
  bool isMiss_8way = false;
  // 16-WAY
  struct cacheLine cache_16way_prefetchMiss[32][16];
  int set_16way_prefetchMiss;
  unsigned long tag_16way_prefetchMiss;
  int index_16way_prefetchMiss = -1;
  unsigned long hits_16way_prefetchMiss = 0;
  unsigned int numIterations_16way_prefetchMiss = 1;
  bool isMiss_16way = false;

  // FULLY ASSOCIATIVE CACHE MOST RECENTLY USED EXTRA CREDIT
  struct cacheLine cache_extraCredit[512];
  unsigned long tag_extraCredit;
  int index_extraCredit = -1;
  unsigned long hits_extraCredit = 0;

  // FULLY ASSOCIATIVE CACHE LEAST FREQUENTLY USED EXTRA CREDIT
  struct cacheLine_EC cache_EC_LFU[512];
  unsigned long tag_EC_LFU;
  int index_EC_LFU = -1;
  unsigned long hits_EC_LFU = 0;


  head = fillTree(9, NULL);

  while(getline(infile, line)) {
    total++;
    stringstream s(line);
    s >> type >> std::hex >> hexAddress;
    //bitset<32> b(hexAddress);
    //string address = b.to_string();


    // set direct mapped cache indices
    for (int i=0; i<4; i++) {
      directIndices[i] = (hexAddress >> 5) % directCacheSizes[i];
      unsigned long tag = hexAddress >> (unsigned long)(log2(directCacheSizes[i]) + 5);
      if (directCaches[i][directIndices[i]] == tag) directHits[i]++;
      else directCaches[i][directIndices[i]] = tag;
    }

    // 2-WAY SET ASSOCIATIVITY
    associativeSet = (hexAddress >> 5) % 256;
    associativeTag = hexAddress >>((unsigned long)log2(256)+5);
    if (associativeTag == associativeCache[associativeSet][0] || associativeTag == associativeCache[associativeSet][1]) {
      associativeHits++;
      if (associativeTag == associativeCache[associativeSet][0]) LRU[associativeSet] = 1;
      else LRU[associativeSet] = 0;
    }
    else {
      associativeCache[associativeSet][LRU[associativeSet]] = associativeTag;
      LRU[associativeSet] += 1;
      if (LRU[associativeSet] > 1) LRU[associativeSet] = 0;
    }



    numIterations++;
    index_4way = -1;
    // 4-WAY SET ASSOCIATIVITY
    set_4way = (hexAddress >> 5) % 128;
    tag_4way = hexAddress >> ((unsigned long)log2(128)+5);
    for (int i = 0; i < 4; i++) {
      if (cache_4way[set_4way][i].tag == tag_4way && cache_4way[set_4way][i].valid) {
        hits_4way++;
        index_4way = i;
        break;
      }
      if (!cache_4way[set_4way][i].valid) {
        index_4way = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_4way == -1) {
      index_4way = getLRU(set_4way, cache_4way[set_4way], 4);
    }
    cache_4way[set_4way][index_4way].valid = true;
    cache_4way[set_4way][index_4way].tag = tag_4way;
    cache_4way[set_4way][index_4way].time = numIterations;


    // 8-WAY SET ASSOCIATIVITY
    index_8way = -1;
    set_8way = (hexAddress >> 5) % 64;
    tag_8way = hexAddress >> (unsigned long)(log2(64)+5);
    for (int i = 0; i < 8; i++) {
      if (cache_8way[set_8way][i].tag == tag_8way && cache_8way[set_8way][i].valid) {
        hits_8way++;
        index_8way = i;
        break;
      }
      if (!cache_8way[set_8way][i].valid) {
        index_8way = i;
        break;
      }
    }
    if (index_8way == -1) index_8way = getLRU(set_8way, cache_8way[set_8way], 8);
    cache_8way[set_8way][index_8way].valid = true;
    cache_8way[set_8way][index_8way].tag = tag_8way;
    cache_8way[set_8way][index_8way].time = numIterations;



    // 16-WAY SET ASSOCIATIVITY
    index_16way = -1;
    set_16way = (hexAddress >> 5) % 32;
    tag_16way = hexAddress >> (unsigned long)(log2(32)+5);
    for (int i = 0; i < 16; i++) {
      if (cache_16way[set_16way][i].tag == tag_16way && cache_16way[set_16way][i].valid) {
        hits_16way++;
        index_16way = i;
        break;
      }
      if (!cache_16way[set_16way][i].valid) {
        index_16way = i;
        break;
      }
    }
    if (index_16way == -1) index_16way = getLRU(set_16way, cache_16way[set_16way], 16);
    cache_16way[set_16way][index_16way].valid = true;
    cache_16way[set_16way][index_16way].tag = tag_16way;
    cache_16way[set_16way][index_16way].time = numIterations;

    // Fully-Associative cache
    tag_fullAssoc = hexAddress >> 5;//((unsigned long)5);
    index_fullAssoc = -1;
    for (int i = 0; i < 512; i++) {
      if (cache_fullAssoc[i].tag == tag_fullAssoc && cache_fullAssoc[i].valid) {
        hits_fullAssoc++;
        index_fullAssoc = i;
        break;
      }
      if (!cache_fullAssoc[i].valid) {
        index_fullAssoc = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_fullAssoc == -1) {
      index_fullAssoc = getLRU(0, cache_fullAssoc, 512);
    }
    cache_fullAssoc[index_fullAssoc].valid = true;
    cache_fullAssoc[index_fullAssoc].tag = tag_fullAssoc;
    cache_fullAssoc[index_fullAssoc].time = numIterations;


    // Fully-Associative HOT/COLD BIT cache
    // index_hotCold = -1;
    // tag_hotCold = hexAddress >> 5;//((unsigned long)5);
    // for (int i=0; i < 512; i++) {
    //   if (cache_hotCold[i].tag == tag_hotCold) {
    //     hits_hotCold++;
    //     index_hotCold = i;
    //     break;
    //   }
    // }
    // if (index_hotCold == -1) index_hotCold = pseudoLRU(0,0);
    // else pseudoLRU(index_hotCold);
    // cache_hotCold[index_hotCold].tag = tag_hotCold;

    index_hotCold = -1;
    tag_hotCold = hexAddress >> 5;

    for (int i = 0; i < 512; i++) {
      // record cache hit
      if (cache_hotCold[i]->value == tag_hotCold) {
        hits_hotCold++;

        index_hotCold = i;
        flipBits(cache_hotCold[i], cache_hotCold[i]->parent);
        break;
      }
    }
    // evict victim if tag isn't found, place value into cache
    if (index_hotCold == -1) evictVictim(tag_hotCold);



    // Set-Associative Cache with no Allocation on a Write Miss
    associativeSet_noAlloc = (hexAddress >> 5) % 256;
    associativeTag_noAlloc = hexAddress >>((unsigned long)log2(256)+5);
    if (associativeTag_noAlloc == assocCache_noAlloc[associativeSet_noAlloc][0] || associativeTag_noAlloc == assocCache_noAlloc[associativeSet_noAlloc][1]) {
      associativeHits_noAlloc++;
      if (associativeTag_noAlloc == assocCache_noAlloc[associativeSet_noAlloc][0]) LRU_noAlloc[associativeSet_noAlloc] = 1;
      else LRU_noAlloc[associativeSet_noAlloc] = 0;
    }
    else {
      if (type == L) {
        assocCache_noAlloc[associativeSet_noAlloc][LRU_noAlloc[associativeSet_noAlloc]] = associativeTag_noAlloc;
        LRU_noAlloc[associativeSet_noAlloc] += 1;
        if (LRU_noAlloc[associativeSet_noAlloc] > 1) LRU_noAlloc[associativeSet_noAlloc] = 0;
      }
    }


    // 4-WAY
    set_4way_noAlloc = (hexAddress >> 5) % 128;
    tag_4way_noAlloc = hexAddress >> ((unsigned long)log2(128)+5);
    index_4way_noAlloc = -1;
    for (int i = 0; i < 4; i++) {
      if (cache_4way_noAlloc[set_4way_noAlloc][i].tag == tag_4way_noAlloc && cache_4way_noAlloc[set_4way_noAlloc][i].valid) {
        hits_4way_noAlloc++;
        index_4way_noAlloc = i;
        break;
      }
      if (!cache_4way_noAlloc[set_4way_noAlloc][i].valid && type == L) {
        index_4way_noAlloc = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (type == L || index_4way_noAlloc != -1) {
      if (index_4way_noAlloc == -1) {
        index_4way_noAlloc = getLRU(set_4way_noAlloc, cache_4way_noAlloc[set_4way_noAlloc], 4);
      }
      cache_4way_noAlloc[set_4way_noAlloc][index_4way_noAlloc].valid = true;
      cache_4way_noAlloc[set_4way_noAlloc][index_4way_noAlloc].tag = tag_4way_noAlloc;
      cache_4way_noAlloc[set_4way_noAlloc][index_4way_noAlloc].time = numIterations;
    }

    // 8-WAY
    set_8way_noAlloc = (hexAddress >> 5) % 64;
    tag_8way_noAlloc = hexAddress >> ((unsigned long)log2(64)+5);
    index_8way_noAlloc = -1;
    for (int i = 0; i < 8; i++) {
      if (cache_8way_noAlloc[set_8way_noAlloc][i].tag == tag_8way_noAlloc && cache_8way_noAlloc[set_8way_noAlloc][i].valid) {
        hits_8way_noAlloc++;
        index_8way_noAlloc = i;
        break;
      }
      if (!cache_8way_noAlloc[set_8way_noAlloc][i].valid && type == L) {
        index_8way_noAlloc = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (type == L || index_8way_noAlloc != -1) {
      if (index_8way_noAlloc == -1) {
        index_8way_noAlloc = getLRU(set_8way_noAlloc, cache_8way_noAlloc[set_8way_noAlloc], 8);
      }
      cache_8way_noAlloc[set_8way_noAlloc][index_8way_noAlloc].valid = true;
      cache_8way_noAlloc[set_8way_noAlloc][index_8way_noAlloc].tag = tag_8way_noAlloc;
      cache_8way_noAlloc[set_8way_noAlloc][index_8way_noAlloc].time = numIterations;
    }

    // 16-WAY
    set_16way_noAlloc = (hexAddress >> 5) % 32;
    tag_16way_noAlloc = hexAddress >> ((unsigned long)log2(32)+5);
    index_16way_noAlloc = -1;
    for (int i = 0; i < 16; i++) {
      if (cache_16way_noAlloc[set_16way_noAlloc][i].tag == tag_16way_noAlloc && cache_16way_noAlloc[set_16way_noAlloc][i].valid) {
        hits_16way_noAlloc++;
        index_16way_noAlloc = i;
        break;
      }
      if (!cache_16way_noAlloc[set_16way_noAlloc][i].valid && type == L) {
        index_16way_noAlloc = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (type == L || index_16way_noAlloc != -1) {
      if (index_16way_noAlloc == -1) {
        index_16way_noAlloc = getLRU(set_16way_noAlloc, cache_16way_noAlloc[set_16way_noAlloc], 16);
      }
      cache_16way_noAlloc[set_16way_noAlloc][index_16way_noAlloc].valid = true;
      cache_16way_noAlloc[set_16way_noAlloc][index_16way_noAlloc].tag = tag_16way_noAlloc;
      cache_16way_noAlloc[set_16way_noAlloc][index_16way_noAlloc].time = numIterations;
    }


    // PRE-FETCHING ON CACHE ACCESS

    numIterations_2way_prefetch++;
    index_2way_prefetch = -1;
    // 2-WAY SET ASSOCIATIVITY
    set_2way_prefetch = (hexAddress >> 5) % 256;
    tag_2way_prefetch = hexAddress >> ((unsigned long)log2(256)+5);
    for (int i = 0; i < 2; i++) {
      if (cache_2way_prefetch[set_2way_prefetch][i].tag == tag_2way_prefetch && cache_2way_prefetch[set_2way_prefetch][i].valid) {
        hits_2way_prefetch++;
        index_2way_prefetch = i;
        break;
      }
      if (!cache_2way_prefetch[set_2way_prefetch][i].valid) {
        index_2way_prefetch = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_2way_prefetch == -1) {
      index_2way_prefetch = getLRU(set_2way_prefetch, cache_2way_prefetch[set_2way_prefetch], 2);
    }
    cache_2way_prefetch[set_2way_prefetch][index_2way_prefetch].valid = true;
    cache_2way_prefetch[set_2way_prefetch][index_2way_prefetch].tag = tag_2way_prefetch;
    cache_2way_prefetch[set_2way_prefetch][index_2way_prefetch].time = numIterations_2way_prefetch;



    numIterations_2way_prefetch++;
    index_2way_prefetch = -1;
    // 2-WAY SET ASSOCIATIVITY
    set_2way_prefetch = ((hexAddress + 32) >> 5) % 256;
    tag_2way_prefetch = (hexAddress + 32) >> ((unsigned long)log2(256)+5);
    for (int i = 0; i < 2; i++) {
      if (cache_2way_prefetch[set_2way_prefetch][i].tag == tag_2way_prefetch && cache_2way_prefetch[set_2way_prefetch][i].valid) {
        index_2way_prefetch = i;
        break;
      }
      if (!cache_2way_prefetch[set_2way_prefetch][i].valid) {
        index_2way_prefetch = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_2way_prefetch == -1) {
      index_2way_prefetch = getLRU(set_2way_prefetch, cache_2way_prefetch[set_2way_prefetch], 2);
    }
    cache_2way_prefetch[set_2way_prefetch][index_2way_prefetch].valid = true;
    cache_2way_prefetch[set_2way_prefetch][index_2way_prefetch].tag = tag_2way_prefetch;
    cache_2way_prefetch[set_2way_prefetch][index_2way_prefetch].time = numIterations_2way_prefetch;



    numIterations_4way_prefetch++;
    index_4way_prefetch = -1;
    // 4-WAY SET ASSOCIATIVITY
    set_4way_prefetch = (hexAddress >> 5) % 128;
    tag_4way_prefetch = hexAddress >> ((unsigned long)log2(128)+5);
    for (int i = 0; i < 4; i++) {
      if (cache_4way_prefetch[set_4way_prefetch][i].tag == tag_4way_prefetch && cache_4way_prefetch[set_4way_prefetch][i].valid) {
        hits_4way_prefetch++;
        index_4way_prefetch = i;
        break;
      }
      if (!cache_4way_prefetch[set_4way_prefetch][i].valid) {
        index_4way_prefetch = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_4way_prefetch == -1) {
      index_4way_prefetch = getLRU(set_4way_prefetch, cache_4way_prefetch[set_4way_prefetch], 4);
    }
    cache_4way_prefetch[set_4way_prefetch][index_4way_prefetch].valid = true;
    cache_4way_prefetch[set_4way_prefetch][index_4way_prefetch].tag = tag_4way_prefetch;
    cache_4way_prefetch[set_4way_prefetch][index_4way_prefetch].time = numIterations_4way_prefetch;



    numIterations_4way_prefetch++;
    index_4way_prefetch = -1;
    // 4-WAY SET ASSOCIATIVITY
    set_4way_prefetch = ((hexAddress + 32) >> 5) % 128;
    tag_4way_prefetch = (hexAddress + 32) >> ((unsigned long)log2(128)+5);
    for (int i = 0; i < 4; i++) {
      if (cache_4way_prefetch[set_4way_prefetch][i].tag == tag_4way_prefetch && cache_4way_prefetch[set_4way_prefetch][i].valid) {
        index_4way_prefetch = i;
        break;
      }
      if (!cache_4way_prefetch[set_4way_prefetch][i].valid) {
        index_4way_prefetch = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_4way_prefetch == -1) {
      index_4way_prefetch = getLRU(set_4way_prefetch, cache_4way_prefetch[set_4way_prefetch], 4);
    }
    cache_4way_prefetch[set_4way_prefetch][index_4way_prefetch].valid = true;
    cache_4way_prefetch[set_4way_prefetch][index_4way_prefetch].tag = tag_4way_prefetch;
    cache_4way_prefetch[set_4way_prefetch][index_4way_prefetch].time = numIterations_4way_prefetch;


    numIterations_8way_prefetch++;
    index_8way_prefetch = -1;
    // 8-WAY SET ASSOCIATIVITY
    set_8way_prefetch = (hexAddress >> 5) % 64;
    tag_8way_prefetch = hexAddress >> ((unsigned long)log2(64)+5);
    for (int i = 0; i < 8; i++) {
      if (cache_8way_prefetch[set_8way_prefetch][i].tag == tag_8way_prefetch && cache_8way_prefetch[set_8way_prefetch][i].valid) {
        hits_8way_prefetch++;
        index_8way_prefetch = i;
        break;
      }
      if (!cache_8way_prefetch[set_8way_prefetch][i].valid) {
        index_8way_prefetch = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_8way_prefetch == -1) {
      index_8way_prefetch = getLRU(set_8way_prefetch, cache_8way_prefetch[set_8way_prefetch], 8);
    }
    cache_8way_prefetch[set_8way_prefetch][index_8way_prefetch].valid = true;
    cache_8way_prefetch[set_8way_prefetch][index_8way_prefetch].tag = tag_8way_prefetch;
    cache_8way_prefetch[set_8way_prefetch][index_8way_prefetch].time = numIterations_8way_prefetch;



    numIterations_8way_prefetch++;
    index_8way_prefetch = -1;
    // 8-WAY SET ASSOCIATIVITY
    set_8way_prefetch = ((hexAddress + 32) >> 5) % 64;
    tag_8way_prefetch = (hexAddress + 32) >> ((unsigned long)log2(64)+5);
    for (int i = 0; i < 8; i++) {
      if (cache_8way_prefetch[set_8way_prefetch][i].tag == tag_8way_prefetch && cache_8way_prefetch[set_8way_prefetch][i].valid) {
        index_8way_prefetch = i;
        break;
      }
      if (!cache_8way_prefetch[set_8way_prefetch][i].valid) {
        index_8way_prefetch = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_8way_prefetch == -1) {
      index_8way_prefetch = getLRU(set_8way_prefetch, cache_8way_prefetch[set_8way_prefetch], 8);
    }
    cache_8way_prefetch[set_8way_prefetch][index_8way_prefetch].valid = true;
    cache_8way_prefetch[set_8way_prefetch][index_8way_prefetch].tag = tag_8way_prefetch;
    cache_8way_prefetch[set_8way_prefetch][index_8way_prefetch].time = numIterations_8way_prefetch;


    numIterations_16way_prefetch++;
    index_16way_prefetch = -1;
    // 16-WAY SET ASSOCIATIVITY
    set_16way_prefetch = (hexAddress >> 5) % 32;
    tag_16way_prefetch = hexAddress >> ((unsigned long)log2(32)+5);
    for (int i = 0; i < 16; i++) {
      if (cache_16way_prefetch[set_16way_prefetch][i].tag == tag_16way_prefetch && cache_16way_prefetch[set_16way_prefetch][i].valid) {
        hits_16way_prefetch++;
        index_16way_prefetch = i;
        break;
      }
      if (!cache_16way_prefetch[set_16way_prefetch][i].valid) {
        index_16way_prefetch = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_16way_prefetch == -1) {
      index_16way_prefetch = getLRU(set_16way_prefetch, cache_16way_prefetch[set_16way_prefetch], 16);
    }
    cache_16way_prefetch[set_16way_prefetch][index_16way_prefetch].valid = true;
    cache_16way_prefetch[set_16way_prefetch][index_16way_prefetch].tag = tag_16way_prefetch;
    cache_16way_prefetch[set_16way_prefetch][index_16way_prefetch].time = numIterations_16way_prefetch;



    numIterations_16way_prefetch++;
    index_16way_prefetch = -1;
    // 16-WAY SET ASSOCIATIVITY
    set_16way_prefetch = ((hexAddress + 32) >> 5) % 32;
    tag_16way_prefetch = (hexAddress + 32) >> ((unsigned long)log2(32)+5);
    for (int i = 0; i < 16; i++) {
      if (cache_16way_prefetch[set_16way_prefetch][i].tag == tag_16way_prefetch && cache_16way_prefetch[set_16way_prefetch][i].valid) {
        index_16way_prefetch = i;
        break;
      }
      if (!cache_16way_prefetch[set_16way_prefetch][i].valid) {
        index_16way_prefetch = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_16way_prefetch == -1) {
      index_16way_prefetch = getLRU(set_16way_prefetch, cache_16way_prefetch[set_16way_prefetch], 16);
    }
    cache_16way_prefetch[set_16way_prefetch][index_16way_prefetch].valid = true;
    cache_16way_prefetch[set_16way_prefetch][index_16way_prefetch].tag = tag_16way_prefetch;
    cache_16way_prefetch[set_16way_prefetch][index_16way_prefetch].time = numIterations_16way_prefetch;



    // PRE-FETCHING ON CACHE MISS

    isMiss_2way = true;
    numIterations_2way_prefetchMiss++;
    index_2way_prefetchMiss = -1;
    // 2-WAY SET ASSOCIATIVITY
    set_2way_prefetchMiss = (hexAddress >> 5) % 256;
    tag_2way_prefetchMiss = hexAddress >> ((unsigned long)log2(256)+5);
    for (int i = 0; i < 2; i++) {
      if (cache_2way_prefetchMiss[set_2way_prefetchMiss][i].tag == tag_2way_prefetchMiss && cache_2way_prefetchMiss[set_2way_prefetchMiss][i].valid) {
        hits_2way_prefetchMiss++;
        isMiss_2way = false;
        index_2way_prefetchMiss = i;
        break;
      }
      if (!cache_2way_prefetchMiss[set_2way_prefetchMiss][i].valid) {
        index_2way_prefetchMiss = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_2way_prefetchMiss == -1) {
      index_2way_prefetchMiss = getLRU(set_2way_prefetchMiss, cache_2way_prefetchMiss[set_2way_prefetchMiss], 2);
    }
    cache_2way_prefetchMiss[set_2way_prefetchMiss][index_2way_prefetchMiss].valid = true;
    cache_2way_prefetchMiss[set_2way_prefetchMiss][index_2way_prefetchMiss].tag = tag_2way_prefetchMiss;
    cache_2way_prefetchMiss[set_2way_prefetchMiss][index_2way_prefetchMiss].time = numIterations_2way_prefetchMiss;


    if (isMiss_2way) {

      numIterations_2way_prefetchMiss++;
      index_2way_prefetchMiss = -1;
      // 2-WAY SET ASSOCIATIVITY
      set_2way_prefetchMiss = ((hexAddress + 32) >> 5) % 256;
      tag_2way_prefetchMiss = (hexAddress + 32) >> ((unsigned long)log2(256)+5);
      for (int i = 0; i < 2; i++) {
        if (cache_2way_prefetchMiss[set_2way_prefetchMiss][i].tag == tag_2way_prefetchMiss && cache_2way_prefetchMiss[set_2way_prefetchMiss][i].valid) {
          index_2way_prefetchMiss = i;
          break;
        }
        if (!cache_2way_prefetchMiss[set_2way_prefetchMiss][i].valid) {
          index_2way_prefetchMiss = i;
          break;
        }
      }
      // if not found in cache, replace current value using LRU policy
      if (index_2way_prefetchMiss == -1) {
        index_2way_prefetchMiss = getLRU(set_2way_prefetchMiss, cache_2way_prefetchMiss[set_2way_prefetchMiss], 2);
      }
      cache_2way_prefetchMiss[set_2way_prefetchMiss][index_2way_prefetchMiss].valid = true;
      cache_2way_prefetchMiss[set_2way_prefetchMiss][index_2way_prefetchMiss].tag = tag_2way_prefetchMiss;
      cache_2way_prefetchMiss[set_2way_prefetchMiss][index_2way_prefetchMiss].time = numIterations_2way_prefetchMiss;

    }


    isMiss_4way = true;
    numIterations_4way_prefetchMiss++;
    index_4way_prefetchMiss = -1;
    // 4-WAY SET ASSOCIATIVITY
    set_4way_prefetchMiss = (hexAddress >> 5) % 128;
    tag_4way_prefetchMiss = hexAddress >> ((unsigned long)log2(128)+5);
    for (int i = 0; i < 4; i++) {
      if (cache_4way_prefetchMiss[set_4way_prefetchMiss][i].tag == tag_4way_prefetchMiss && cache_4way_prefetchMiss[set_4way_prefetchMiss][i].valid) {
        hits_4way_prefetchMiss++;
        isMiss_4way = false;
        index_4way_prefetchMiss = i;
        break;
      }
      if (!cache_4way_prefetchMiss[set_4way_prefetchMiss][i].valid) {
        index_4way_prefetchMiss = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_4way_prefetchMiss == -1) {
      index_4way_prefetchMiss = getLRU(set_4way_prefetchMiss, cache_4way_prefetchMiss[set_4way_prefetchMiss], 4);
    }
    cache_4way_prefetchMiss[set_4way_prefetchMiss][index_4way_prefetchMiss].valid = true;
    cache_4way_prefetchMiss[set_4way_prefetchMiss][index_4way_prefetchMiss].tag = tag_4way_prefetchMiss;
    cache_4way_prefetchMiss[set_4way_prefetchMiss][index_4way_prefetchMiss].time = numIterations_4way_prefetchMiss;


    if (isMiss_4way) {

      numIterations_4way_prefetchMiss++;
      index_4way_prefetchMiss = -1;
      // 4-WAY SET ASSOCIATIVITY
      set_4way_prefetchMiss = ((hexAddress + 32) >> 5) % 128;
      tag_4way_prefetchMiss = (hexAddress + 32) >> ((unsigned long)log2(128)+5);
      for (int i = 0; i < 4; i++) {
        if (cache_4way_prefetchMiss[set_4way_prefetchMiss][i].tag == tag_4way_prefetchMiss && cache_4way_prefetchMiss[set_4way_prefetchMiss][i].valid) {
          index_4way_prefetchMiss = i;
          break;
        }
        if (!cache_4way_prefetchMiss[set_4way_prefetchMiss][i].valid) {
          index_4way_prefetchMiss = i;
          break;
        }
      }
      // if not found in cache, replace current value using LRU policy
      if (index_4way_prefetchMiss == -1) {
        index_4way_prefetchMiss = getLRU(set_4way_prefetchMiss, cache_4way_prefetchMiss[set_4way_prefetchMiss], 4);
      }
      cache_4way_prefetchMiss[set_4way_prefetchMiss][index_4way_prefetchMiss].valid = true;
      cache_4way_prefetchMiss[set_4way_prefetchMiss][index_4way_prefetchMiss].tag = tag_4way_prefetchMiss;
      cache_4way_prefetchMiss[set_4way_prefetchMiss][index_4way_prefetchMiss].time = numIterations_4way_prefetchMiss;

    }

    isMiss_8way = true;
    numIterations_8way_prefetchMiss++;
    index_8way_prefetchMiss = -1;
    // 8-WAY SET ASSOCIATIVITY
    set_8way_prefetchMiss = (hexAddress >> 5) % 64;
    tag_8way_prefetchMiss = hexAddress >> ((unsigned long)log2(64)+5);
    for (int i = 0; i < 8; i++) {
      if (cache_8way_prefetchMiss[set_8way_prefetchMiss][i].tag == tag_8way_prefetchMiss && cache_8way_prefetchMiss[set_8way_prefetchMiss][i].valid) {
        hits_8way_prefetchMiss++;
        isMiss_8way = false;
        index_8way_prefetchMiss = i;
        break;
      }
      if (!cache_8way_prefetchMiss[set_8way_prefetchMiss][i].valid) {
        index_8way_prefetchMiss = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_8way_prefetchMiss == -1) {
      index_8way_prefetchMiss = getLRU(set_8way_prefetchMiss, cache_8way_prefetchMiss[set_8way_prefetchMiss], 8);
    }
    cache_8way_prefetchMiss[set_8way_prefetchMiss][index_8way_prefetchMiss].valid = true;
    cache_8way_prefetchMiss[set_8way_prefetchMiss][index_8way_prefetchMiss].tag = tag_8way_prefetchMiss;
    cache_8way_prefetchMiss[set_8way_prefetchMiss][index_8way_prefetchMiss].time = numIterations_8way_prefetchMiss;


    if (isMiss_8way) {

      numIterations_8way_prefetchMiss++;
      index_8way_prefetchMiss = -1;
      // 8-WAY SET ASSOCIATIVITY
      set_8way_prefetchMiss = ((hexAddress + 32) >> 5) % 64;
      tag_8way_prefetchMiss = (hexAddress + 32) >> ((unsigned long)log2(64)+5);
      for (int i = 0; i < 8; i++) {
        if (cache_8way_prefetchMiss[set_8way_prefetchMiss][i].tag == tag_8way_prefetchMiss && cache_8way_prefetchMiss[set_8way_prefetchMiss][i].valid) {
          index_8way_prefetchMiss = i;
          break;
        }
        if (!cache_8way_prefetchMiss[set_8way_prefetchMiss][i].valid) {
          index_8way_prefetchMiss = i;
          break;
        }
      }
      // if not found in cache, replace current value using LRU policy
      if (index_8way_prefetchMiss == -1) {
        index_8way_prefetchMiss = getLRU(set_8way_prefetchMiss, cache_8way_prefetchMiss[set_8way_prefetchMiss], 8);
      }
      cache_8way_prefetchMiss[set_8way_prefetchMiss][index_8way_prefetchMiss].valid = true;
      cache_8way_prefetchMiss[set_8way_prefetchMiss][index_8way_prefetchMiss].tag = tag_8way_prefetchMiss;
      cache_8way_prefetchMiss[set_8way_prefetchMiss][index_8way_prefetchMiss].time = numIterations_8way_prefetchMiss;

    }

    isMiss_16way = true;
    numIterations_16way_prefetchMiss++;
    index_16way_prefetchMiss = -1;
    // 16-WAY SET ASSOCIATIVITY
    set_16way_prefetchMiss = (hexAddress >> 5) % 32;
    tag_16way_prefetchMiss = hexAddress >> ((unsigned long)log2(32)+5);
    for (int i = 0; i < 16; i++) {
      if (cache_16way_prefetchMiss[set_16way_prefetchMiss][i].tag == tag_16way_prefetchMiss && cache_16way_prefetchMiss[set_16way_prefetchMiss][i].valid) {
        hits_16way_prefetchMiss++;
        isMiss_16way = false;
        index_16way_prefetchMiss = i;
        break;
      }
      if (!cache_16way_prefetchMiss[set_16way_prefetchMiss][i].valid) {
        index_16way_prefetchMiss = i;
        break;
      }
    }
    // if not found in cache, replace current value using LRU policy
    if (index_16way_prefetchMiss == -1) {
      index_16way_prefetchMiss = getLRU(set_16way_prefetchMiss, cache_16way_prefetchMiss[set_16way_prefetchMiss], 16);
    }
    cache_16way_prefetchMiss[set_16way_prefetchMiss][index_16way_prefetchMiss].valid = true;
    cache_16way_prefetchMiss[set_16way_prefetchMiss][index_16way_prefetchMiss].tag = tag_16way_prefetchMiss;
    cache_16way_prefetchMiss[set_16way_prefetchMiss][index_16way_prefetchMiss].time = numIterations_16way_prefetchMiss;


    if (isMiss_16way) {

      numIterations_16way_prefetchMiss++;
      index_16way_prefetchMiss = -1;
      // 16-WAY SET ASSOCIATIVITY
      set_16way_prefetchMiss = ((hexAddress + 32) >> 5) % 32;
      tag_16way_prefetchMiss = (hexAddress + 32) >> ((unsigned long)log2(32)+5);
      for (int i = 0; i < 16; i++) {
        if (cache_16way_prefetchMiss[set_16way_prefetchMiss][i].tag == tag_16way_prefetchMiss && cache_16way_prefetchMiss[set_16way_prefetchMiss][i].valid) {
          index_16way_prefetchMiss = i;
          break;
        }
        if (!cache_16way_prefetchMiss[set_16way_prefetchMiss][i].valid) {
          index_16way_prefetchMiss = i;
          break;
        }
      }
      // if not found in cache, replace current value using LRU policy
      if (index_16way_prefetchMiss == -1) {
        index_16way_prefetchMiss = getLRU(set_16way_prefetchMiss, cache_16way_prefetchMiss[set_16way_prefetchMiss], 16);
      }
      cache_16way_prefetchMiss[set_16way_prefetchMiss][index_16way_prefetchMiss].valid = true;
      cache_16way_prefetchMiss[set_16way_prefetchMiss][index_16way_prefetchMiss].tag = tag_16way_prefetchMiss;
      cache_16way_prefetchMiss[set_16way_prefetchMiss][index_16way_prefetchMiss].time = numIterations_16way_prefetchMiss;

    }


    // EXTRA CREDIT CACHE MOST RECENTLY USED
    // tag_extraCredit = hexAddress >> 5;//((unsigned long)5);
    // index_extraCredit = -1;
    // for (int i = 0; i < 512; i++) {
    //   if (cache_extraCredit[i].tag == tag_extraCredit && cache_extraCredit[i].valid) {
    //     hits_extraCredit++;
    //     index_extraCredit = i;
    //     break;
    //   }
    //   if (!cache_extraCredit[i].valid) {
    //     index_extraCredit = i;
    //     break;
    //   }
    // }
    // // if not found in cache, replace current value using LRU policy
    // if (index_extraCredit == -1) {
    //   index_extraCredit = getMRU(0, cache_extraCredit, 512);
    // }
    // cache_extraCredit[index_extraCredit].valid = true;
    // cache_extraCredit[index_extraCredit].tag = tag_extraCredit;
    // cache_extraCredit[index_extraCredit].time = numIterations;



    if (EXTRA_CREDIT == 1) {
      // EXTRA CREDIT CACHE LEAST FREQUENTLY USED
      tag_EC_LFU = hexAddress >> 5;//((unsigned long)5);
      index_EC_LFU = -1;
      for (int i = 0; i < 512; i++) {
        if (cache_EC_LFU[i].tag == tag_EC_LFU && cache_EC_LFU[i].valid) {
          hits_EC_LFU++;
          index_EC_LFU = i;
          cache_EC_LFU[index_EC_LFU].numUses = cache_EC_LFU[index_EC_LFU].numUses + 1;
          break;
        }
        if (!cache_EC_LFU[i].valid) {
          index_EC_LFU = i;
          break;
        }
      }
      // if not found in cache, replace current value using LRU policy
      if (index_EC_LFU == -1) {
        index_EC_LFU = getLeastFreqUsed(0, cache_EC_LFU, 512);
      }
      cache_EC_LFU[index_EC_LFU].valid = true;
      cache_EC_LFU[index_EC_LFU].tag = tag_EC_LFU;
      cache_EC_LFU[index_EC_LFU].time = numIterations;
      cache_EC_LFU[index_EC_LFU].numUses = cache_EC_LFU[index_EC_LFU].numUses + 1;;
    }

  }


  for (int i=0; i<3; i++) outputFile << directHits[i] << "," << total << "; ";
  outputFile << directHits[3] << "," << total << ";" << endl;
  outputFile << associativeHits << "," << total << "; " << hits_4way << "," << total << "; " << hits_8way << "," << total << "; " << hits_16way << "," << total << ";" << endl;
  outputFile << hits_fullAssoc << "," << total << ";" << endl;
  outputFile << hits_hotCold << "," << total << ";" << endl;
  outputFile << associativeHits_noAlloc << "," << total << "; " << hits_4way_noAlloc << "," << total << "; " << hits_8way_noAlloc << "," << total << "; " << hits_16way_noAlloc << "," << total << ";"<< endl;
  outputFile << hits_2way_prefetch << "," << total << "; " << hits_4way_prefetch << "," << total << "; " << hits_8way_prefetch << "," << total << "; " << hits_16way_prefetch << "," << total << ";" << endl;
  outputFile << hits_2way_prefetchMiss << "," << total << "; " << hits_4way_prefetchMiss << "," << total << "; " << hits_8way_prefetchMiss << "," << total << "; " << hits_16way_prefetchMiss << "," << total << ";" << endl;
  if (EXTRA_CREDIT == 1) outputFile << hits_EC_LFU << "," << total << "; <---- Extra Credit" << endl;
  outputFile.close();
  return 0;
}
