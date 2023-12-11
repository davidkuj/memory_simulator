#include "DRAM.h"
#include "DRAM_Cache.h"
#include "Performance.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// read/write a word of memory.
// This read/write employs a 2-Way Set Associative Write Deferred cache
// Each cache line is 32 bytes (see DRAM.h)
// Total cache size is specified below
#define CACHE_SIZE 256


struct line{
    //check if dirty or clean
    bool validFlag;
    bool dirtyFlag;
    //line itself
    CacheLine data;
    //address
    Address ad;
    //split the 16 bit line into different elements to access
    unsigned int tag;
    unsigned int setIndex;
    unsigned int offset;
    //keep track of age
    int age;
};

int totalAge = 0;

struct line cache[4][2];

void initCache()
{
    for(int i = 0; i<4;++i)
    {
        for(int j = 0; j<2; ++j)
        {
            memset(cache[i][j].data, 0, 32);
            cache[i][j].setIndex = i;
            cache[i][j].dirtyFlag = false;
            cache[i][j].validFlag = false;
        }
    }
}
int readWithCache(Address addr)
{
    totalAge +=1;
    int val;
    unsigned int t = addr >> 7;
    unsigned int s = (addr>>5)&0x3;
    unsigned int o = addr & 0x1F;
    Address a = addr & 0xFFE0;
    for(int j = 0; j<2; ++j)
    {
        if(cache[s][j].validFlag && cache[s][j].tag == t)
        {
            perfCacheHit(a, s, j);
            cache[s][j].age += 1;
            memcpy(&val, &cache[s][j].data[o], 4);
            cache[s][j].age = totalAge;
            return val;
        }
    }
    int lessIndex = 0;
    if(cache[s][0].age<cache[s][1].age)
    {
        lessIndex = 1;
    }
    
    if(cache[s][lessIndex].dirtyFlag == true)
    {
        writeDramCacheLine((cache[s][0].tag << 7 | cache[s][0].setIndex << 5), cache[s][lessIndex].data);
        cache[s][lessIndex].dirtyFlag = false;
    }
    readDramCacheLine(a, cache[s][lessIndex].data);
    perfCacheMiss(a, s, lessIndex, cache[s][lessIndex].validFlag);
    memcpy(&val, &cache[s][lessIndex].data[o], 4);
    cache[s][lessIndex].validFlag = true;
    cache[s][lessIndex].tag = t;
    cache[s][lessIndex].setIndex = s;
    cache[s][lessIndex].age = totalAge;
    return val;
        
}
void writeWithCache(Address addr, int value)
{
    totalAge +=1;
    unsigned int t = addr >> 7;
    unsigned int s = (addr>>5)&0x3;
    unsigned int o = addr & 0x1F;
    Address a = addr & 0xFFE0; 
    for(int j = 0; j<2; ++j)
    {   
        if(cache[s][j].validFlag && t  == cache[s][j].tag)
        {   
            perfCacheHit(a, s, j);
            memcpy(&cache[s][j].data[o], &value, 4);
            cache[s][j].validFlag = true;
            cache[s][j].dirtyFlag = true;
            cache[s][j].age = totalAge;
            cache[s][j].setIndex = s;
            return;
        }   
    }
    int oldIndex = 0;
    if(cache[s][0].age<cache[s][1].age)
    {   
        oldIndex = 1;
    }
    if(cache[s][oldIndex].dirtyFlag == true)
    {   
        writeDramCacheLine((cache[s][0].tag << 7 | cache[s][0].setIndex << 5), cache[s][oldIndex].data);
        cache[s][oldIndex].dirtyFlag = false;
    }
    readDramCacheLine(a, cache[s][oldIndex].data);
    perfCacheMiss(a, s, oldIndex, cache[s][oldIndex].validFlag);
    memcpy(&cache[s][oldIndex].data[o], &value,  4);
    cache[s][oldIndex].validFlag = true;
    cache[s][oldIndex].dirtyFlag = true;
    cache[s][oldIndex].tag = t; 
    cache[s][oldIndex].setIndex = s;
    cache[s][oldIndex].age = totalAge;

}
void flushCache()
{
    //how to iterate through the entire cache?
    for(int i = 0; i<4; ++i)
    {
        for(int j = 0; j<2; j++)
        {
            if(cache[i][j].dirtyFlag == true)
            {
                writeDramCacheLine((cache[i][j].tag << 7 | i << 5), cache[i][j].data);
                cache[i][j].validFlag = true;
                cache[i][j].dirtyFlag = false;
            }
        }
    }
    perfCacheFlush();
}


