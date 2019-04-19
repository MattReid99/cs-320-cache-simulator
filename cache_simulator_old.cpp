#include<iostream>
#include<fstream>
#include<sstream>
#include<string>
#include <vector>
#include <math.h>
#include <bitset>
#include <time.h>

#define L "L"
#define S "S"

#define KB 1024


using namespace std;

// retrieved from GeeksForGeeks
// k = # bits, p = position
int bitExtracted(int number, int k, int p)
{
    return (((1 << k) - 1) & (number >> (p - 1)));
}


int main(int argc, char *argv[]) {
  if (argc != 3) {
    cerr << "correct arguments:\t./predictors <input file>.txt <output file>.txt\n";
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

  // SET-ASSOCIATIVE CACHE
  // 32 BYTE LINE SIZE
  // 16 KB
  // 2-WAY ASSOCIATIVE
  // 32 BIT ADDRESSESS
  // 5 BIT OFFSET (log2(32)) (# BLOCKS)
  // SIZE OF CACHE = 512 entries
  // 512 / (32 * #ways) = # SETS (8), log2(8) = 3 bits for set
  // tag = 32 - 5 - 3 = 24 bits
  // 163384 BYTES / (32 BYTES PER BLOCK * 2 PER SET)

  unsigned long associativeHits = 0;
  int LRU[256] = {0};
  unsigned long associativeCache[256][2] = {0};
  int associativeSet; // (bits 24-26)
  unsigned long associativeTag;
  // ignore offset (last 5 bits)


  vector<vector<int> > LRU_4ways(128);
  unsigned long associativeCache_4way[128][4] = {0};
  int associativeSet_4way;
  unsigned long associativeTag_4way;
  unsigned long associativeHits_4way = 0;
  for (int i=0; i<128; i++) {
    for (int j=0; j<4; j++) {
          LRU_4ways[i].push_back(j);
    }
  }


  // 0, 1 - SET 1   2, 3  - SET 2   4,5 - SET 3 6,7 - SET 4 8,9 - S


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


    // 4-WAY SET ASSOCIATIVITY
    associativeSet_4way = (hexAddress >> 5) % 128;
    associativeTag_4way = hexAddress >> (unsigned long)(log2(128)+5);
    int found_4way = -1;
    // first check if tag is in set at 0-3
    for (int i=0; i<4; i++) {
      if (associativeCache_4way[associativeSet_4way][i] == associativeTag_4way) {
        associativeHits_4way++;
        found_4way = i;
        break;
      }
    }
    // if not, check if any values in set are == 0, and insert there
    if (found_4way == -1) {
      for (int i=0; i<4; i++) {
        if (associativeCache_4way[associativeSet_4way][i] == 0) {
          associativeCache_4way[associativeSet_4way][i] = associativeTag_4way;
          associativeHits_4way++;
          found_4way = i;
          break;
          }
      }
    }

    // if value found, set new value to be MOST recently used
    if (found_4way != -1) {
      // LRU_4ways[associativeSet_4way].erase(0);
      for (int i=0; i<4; i++) if (LRU_4ways[associativeSet_4way][i] == found_4way) LRU_4ways[associativeSet_4way].erase(LRU_4ways[associativeSet_4way].begin()+i);
      LRU_4ways[associativeSet_4way].push_back(found_4way);
    }
    //if not found, remove LEAST recently used, set = to tag
    else {
      int oldIndex = LRU_4ways[associativeSet_4way][0];
      associativeCache_4way[associativeSet_4way][oldIndex] = associativeTag_4way;
      LRU_4ways[associativeSet_4way].erase(LRU_4ways[associativeSet_4way].begin()+0);
      LRU_4ways[associativeSet_4way].push_back(oldIndex);
    }



  }


  for (int i=0; i<3; i++) outputFile << directHits[i] << "," << total << "; ";
  outputFile << directHits[3] << "," << total << ";" << endl;
  outputFile << associativeHits << "," << total << "; " << associativeHits_4way << "," << total << "; " << endl;

  outputFile.close();
  return 0;
}
