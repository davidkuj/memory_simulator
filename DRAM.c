#include "DRAM.h"
#include "Performance.h"
#include <string.h>

char dram[48 *1024];


// read/write a word of memory
int readDram(Address addr)
{
    int result;
    memcpy(&result, &dram[addr], 4);
    perfDramRead(addr, result);
    return result;        
}
void writeDram(Address addr, int value)
{
    memcpy(&dram[addr], &value, 4);
    perfDramWrite(addr, value); 
}

// read/write a cache line
void readDramCacheLine(Address addr, CacheLine line)
{
    memcpy(line, &dram[addr], CACHE_LINE_SIZE);
    perfDramCacheLineRead(addr, line);
}
void writeDramCacheLine(Address addr, CacheLine line)
{
    memcpy(&dram[addr], line, CACHE_LINE_SIZE);
    perfDramCacheLineWrite(addr, line);
}

