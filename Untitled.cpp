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








// PRE-FETCHING ON CACHE ACCESS

isMiss_2way = false;
numIterations_2way_prefetchMiss++;
index_2way_prefetchMiss = -1;
// 2-WAY SET ASSOCIATIVITY
set_2way_prefetchMiss = (hexAddress >> 5) % 256;
tag_2way_prefetchMiss = hexAddress >> ((unsigned long)log2(256)+5);
for (int i = 0; i < 2; i++) {
  if (cache_2way_prefetchMiss[set_2way_prefetchMiss][i].tag == tag_2way_prefetchMiss && cache_2way_prefetchMiss[set_2way_prefetchMiss][i].valid) {
    hits_2way_prefetchMiss++;
    index_2way_prefetchMiss = i;
    break;
  }
  if (!cache_2way_prefetchMiss[set_2way_prefetchMiss][i].valid) {
    isMiss_2way = true;
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


isMiss_4way = false;
numIterations_4way_prefetchMiss++;
index_4way_prefetchMiss = -1;
// 4-WAY SET ASSOCIATIVITY
set_4way_prefetchMiss = (hexAddress >> 5) % 128;
tag_4way_prefetchMiss = hexAddress >> ((unsigned long)log2(128)+5);
for (int i = 0; i < 4; i++) {
  if (cache_4way_prefetchMiss[set_4way_prefetchMiss][i].tag == tag_4way_prefetchMiss && cache_4way_prefetchMiss[set_4way_prefetchMiss][i].valid) {
    hits_4way_prefetchMiss++;
    index_4way_prefetchMiss = i;
    break;
  }
  if (!cache_4way_prefetchMiss[set_4way_prefetchMiss][i].valid) {
    isMiss_4way = true;
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

isMiss_8way = false;
numIterations_8way_prefetchMiss++;
index_8way_prefetchMiss = -1;
// 8-WAY SET ASSOCIATIVITY
set_8way_prefetchMiss = (hexAddress >> 5) % 64;
tag_8way_prefetchMiss = hexAddress >> ((unsigned long)log2(64)+5);
for (int i = 0; i < 8; i++) {
  if (cache_8way_prefetchMiss[set_8way_prefetchMiss][i].tag == tag_8way_prefetchMiss && cache_8way_prefetchMiss[set_8way_prefetchMiss][i].valid) {
    hits_8way_prefetchMiss++;
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

isMiss_16way = false;
numIterations_16way_prefetchMiss++;
index_16way_prefetchMiss = -1;
// 16-WAY SET ASSOCIATIVITY
set_16way_prefetchMiss = (hexAddress >> 5) % 32;
tag_16way_prefetchMiss = hexAddress >> ((unsigned long)log2(32)+5);
for (int i = 0; i < 16; i++) {
  if (cache_16way_prefetchMiss[set_16way_prefetchMiss][i].tag == tag_16way_prefetchMiss && cache_16way_prefetchMiss[set_16way_prefetchMiss][i].valid) {
    hits_16way_prefetchMiss++;
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
