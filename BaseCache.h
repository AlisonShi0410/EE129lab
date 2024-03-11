#ifndef __BASE_CACHE_H__
#define __BASE_CACHE_H__

#include <iostream>

#define ADDR_BITS 32
typedef struct _cacheLine {
  uint32_t tag;
  uint32_t *data; // data points to data stored in 
    		  // cache line. cache line size is
    		  // multiple of 32bits.
  uint32_t lru;
  bool valid;
} cacheLine;

class BaseCache{
  private:
    //cache base parameters
  	uint32_t cacheSize; //in Bytes
  	uint32_t associativity;
  	uint32_t blockSize;  //in Bytes
    cacheLine **cacheLines;
    	
   	//cache derived parameters
    //define any additional parameters like number of
    //sets, index/tag bits or whatever is needed to
    //implement correct cache behavior.
	  //WRITE ME
	  uint32_t numSets;
	  uint32_t numIndexBits;
	  uint32_t numOffsetBits;
	  uint32_t numTagBits;

	  //cache access statistics
	  uint32_t numReads;
	  uint32_t numWrites;
	  uint32_t numReadHits;
	  uint32_t numReadMisses;
	  uint32_t numWriteHits;
	  uint32_t numWriteMisses;

  public:
    //Default constructor to set everything to '0'.
    BaseCache();        
    //Constructor to initialize cache parameters and to create the cache
    BaseCache(uint32_t _cacheSize, uint32_t _associativity, uint32_t _blockSize);
	  //Set cache base parameters
    void setCacheSize(uint32_t _cacheSize); 
    void setAssociativity(uint32_t _associativity);
    void setBlockSize(uint32_t _blockSize);
        
    //Get cache base parameters
	uint32_t getCacheSize(); 
    uint32_t getAssociativity(); 
    uint32_t getBlockSize(); 
	
	  //Get cache access statistics
	  uint32_t getReadHits(); 
	  uint32_t getReadMisses(); 
	  uint32_t getWriteHits(); 
	  uint32_t getWriteMisses(); 
	  double getReadHitRate();
	  double getReadMissRate();
	  double getWriteHitRate();
	  double getWriteMissRate();
	  double getOverallHitRate();
	  double getOverallMissRate();

	  //Initialize cache derived parameters
    //This function computes the derived parameters given
    //the base parameters.
	  void initDerivedParams();
	  //Reset cache access statistics
	  void resetStats();
	  //Create cache and clear it
	  void createCache();
	  //Reset cache
	  void clearCache();
	  //Read data
	  //return true if it was a hit, false if it was a miss
	  //data is only valid if it was a hit, input data pointer
	  //is not updated upon miss.
	  bool read(uint32_t addr, uint32_t *data);
	  //Write data
	  //Function returns write hit or miss status. 
	  bool write(uint32_t addr, uint32_t data);

	  /********ADD ANY ADDITIONAL METHODS IF REQUIRED*********/	 
	  // Update the lru count for each cache line.
	  void updateLRU(uint32_t ru_associativity, uint32_t set_index);
	  // Return the Least Recent Use associativity index.
	  uint32_t getLRUIndex(uint32_t set_index);
	  //Destructor to free all allocated memeroy.
	  ~BaseCache();
};
#endif
