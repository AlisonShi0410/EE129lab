#include <iostream>
#include "BaseCache.h"
#include <cmath>
using namespace std;

// WRITE ME
// Default constructor to set everything to '0'
BaseCache::BaseCache()
{   
    cacheSize = 0;
    associativity = 0;
    blockSize = 0;
    cacheLines = nullptr;
    initDerivedParams();
    resetStats();
}

// WRITE ME
// Constructor to initialize cache parameters, create the cache and clears it
BaseCache::BaseCache(uint32_t _cacheSize, uint32_t _associativity, uint32_t _blockSize)
{   
    cacheSize = _cacheSize;
    associativity = _associativity;
    blockSize = _blockSize;
    initDerivedParams();
    clearCache();
    createCache();
    resetStats();

}

// WRITE ME
// Set cache base parameters
void BaseCache::setCacheSize(uint32_t _cacheSize) {this->cacheSize = _cacheSize;}
void BaseCache::setAssociativity(uint32_t _associativity) {this->associativity = _associativity;}
void BaseCache::setBlockSize(uint32_t _blockSize) {this->blockSize = _blockSize;}

// WRITE ME
// Get cache base parameters
uint32_t BaseCache::getCacheSize() {return this->cacheSize;}
uint32_t BaseCache::getAssociativity() {return this->associativity;}
uint32_t BaseCache::getBlockSize() {return this->blockSize;}

// WRITE ME
// Get cache access statistics
uint32_t BaseCache::getReadHits() {return this->numReadHits;}
uint32_t BaseCache::getReadMisses() {return this->numReadMisses;}
uint32_t BaseCache::getWriteHits() {return this->numWriteHits;}
uint32_t BaseCache::getWriteMisses() {return this->numWriteMisses;}
double BaseCache::getReadHitRate() {return numReadHits * 100 / numReads;}
double BaseCache::getReadMissRate() {return numReadMisses* 100 / numReads;}
double BaseCache::getWriteHitRate() {return numWriteHits * 100 / numWrites;}
double BaseCache::getWriteMissRate() {return numWriteMisses * 100 / numWrites;}
double BaseCache::getOverallHitRate() {return (numWriteHits + numReadHits) * 100 /(numWrites + numReads);}
double BaseCache::getOverallMissRate() {return (numWriteMisses + numReadMisses) * 100 /(numWrites + numReads);}

// WRITE ME
// Initialize cache derived parameters
void BaseCache::initDerivedParams()
{   
    numSets = cacheSize / (blockSize * associativity);
    numIndexBits = static_cast<uint32_t>(log2(cacheSize / (blockSize * associativity)));
    numOffsetBits = static_cast<uint32_t>(log2(blockSize));
    numTagBits = ADDR_BITS - numIndexBits - numOffsetBits;

}

// WRITE ME
// Reset cache access statistics
void BaseCache::resetStats()
{   
    numReads = 0;
    numWrites = 0;
    numReadHits = 0;
    numWriteHits = 0;
    numWriteMisses = 0;
}

// WRITE ME
// Create cache and clear it
void BaseCache::createCache()
{
    cacheLines = new cacheLine *[numSets];
    // Allocate for each set
    for (uint32_t i = 0; i < numSets; i++)
    {
        // Allocate for each line
        cacheLines[i] = new cacheLine[associativity];
        for (uint32_t j = 0; j < associativity; j++)
        {
            cacheLines[i][j].data = new uint32_t[blockSize / sizeof(uint32_t)]();
            cacheLines[i][j].valid = false;
            cacheLines[i][j].lru = 0;
        }
    }
}

// WRITE ME
// Reset cache
void BaseCache::clearCache()
{
    if (cacheLines != nullptr) {
        for (uint32_t i = 0; i < numSets; i ++) {
            for (uint32_t j = 0; j < associativity; j ++) {
                if (cacheLines[i][j].data != nullptr) {
                    delete[] cacheLines[i][j].data;
                    cacheLines[i][j].data = nullptr;
                }
            }
            delete[] cacheLines[i];
        }
        delete[] cacheLines;
        cacheLines = nullptr;
    }

}

// WRITE ME
// Read data
// return true if it was a hit, false if it was a miss
// data is only valid if it was a hit, input data pointer
// is not updated upon miss. Make sure to update LRU stack
// bits. You can choose to separate the LRU bits update into
//  a separate function that can be used from both read() and write().
bool BaseCache::read(uint32_t addr, uint32_t *data)
{
    uint32_t cur_tag = addr >> (numIndexBits + numOffsetBits);
    uint32_t cur_set = (addr >> numOffsetBits) & (numSets - 1);
    uint32_t cur_offset = (addr & (blockSize - 1)) / sizeof(uint32_t);
    numReads++;

    for (uint32_t i = 0; i < associativity; i ++) {
        // Hit: update LRU
        if (cacheLines[cur_set][i].tag == cur_tag) {
            cacheLines[cur_set][i].valid = true;
            *data = cacheLines[cur_set][i].data[cur_offset];
            updateLRU(cur_set, i);
            numReadHits++;
            return true;
        }
    }

    // Miss: replace and update LRU
    uint32_t rpl_index = getLRUIndex(cur_set);
    cacheLines[cur_set][rpl_index].tag = cur_tag;
    cacheLines[cur_set][rpl_index].valid = true;
    updateLRU(cur_set, rpl_index);
    numReadMisses++;
    return false;
}
// WRITE ME
// Write data
// Function returns write hit or miss status.
bool BaseCache::write(uint32_t addr, uint32_t data)
{
    uint32_t cur_tag = addr >> (numIndexBits + numOffsetBits);
    uint32_t cur_set = (addr >> numOffsetBits) & (numSets - 1);
    uint32_t cur_offset = (addr & (blockSize - 1)) / sizeof(uint32_t);
    numWrites++;

    for (uint32_t i = 0; i < associativity; i ++) {
        // Hit: update LRU
        if (cacheLines[cur_set][i].tag == cur_tag) {
            cacheLines[cur_set][i].valid = true;
            cacheLines[cur_set][i].data[cur_offset] = data;
            updateLRU(cur_set, i);
            numWriteHits++;
            return true;
        }
    }

    // Miss: replace and update LRU
    uint32_t rpl_index = getLRUIndex(cur_set);
    cacheLines[cur_set][rpl_index].tag = cur_tag;
    cacheLines[cur_set][rpl_index].data[cur_offset] = data;
    cacheLines[cur_set][rpl_index].valid = true;
    updateLRU(cur_set, rpl_index);
    numWriteMisses++;
    return false;
}


// Update LRU count for each line
void BaseCache::updateLRU(uint32_t set_index, uint32_t way_index)
{
    cacheLines[set_index][way_index].lru = 0;
    for (uint32_t i = 0; i < associativity; i ++) {
        if (cacheLines[set_index][i].valid && i != way_index) {
            cacheLines[set_index][i].lru ++;
        }
    }
}

// Return the LRU associativity for replacement
uint32_t BaseCache::getLRUIndex(uint32_t set_index)
{
    uint32_t LRUindex = 0;
    uint32_t maxLRU = 0;
    for (uint32_t i = 0; i < associativity; i ++) {
        if (cacheLines[set_index][i].valid && cacheLines[set_index][i].lru > maxLRU) {
            maxLRU = cacheLines[set_index][i].lru;
            LRUindex = i;
        }
    }
    return LRUindex;
}

// WRITE ME
// Destructor to free all allocated memeroy.
BaseCache::~BaseCache()
{
    clearCache();
}
                                                        