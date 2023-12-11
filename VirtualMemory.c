#include "DRAM.h"
#include "DRAM_Cache.h"
#include "Performance.h"
#include "VirtualMemory.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// control of the virtual memory.  
// When disabled the memory addresses are interpreted as physical addresses - no translation occurs
#define PAGE_TABLE_ENTRIES 64
#define BYTES_PER_PTE 4        // each page table entry is 32-bits.

struct TLBEntry
{
    int virtualPageNumber;
    int physPageNumber;
    bool valid; 
};

struct TLBEntry TLB[2];

int RR = 0;

bool vmEnabled = false;

Address PageTableAddress;


int searchTLB(Address addr, int *ppn)
{
    for(int i = 0; i<2; ++i)
    {   
        if(TLB[i].virtualPageNumber == (addr>>10) && TLB[i].valid==true)
        {   
            *ppn = TLB[i].physPageNumber;
            perfTlbHit(*ppn); //should i be using the physical or virtual page number?
            return i;
        }   
    }    
    *ppn = readWithCache(PageTableAddress + ((addr >> 10) * BYTES_PER_PTE)) & 0x3F; //how am i supposed to extract the ppn using readWithCache()?
    /*What we have to use:
     - Virtual Page Number
     - Offset
     - Page Table Address
     - PAGE_TABLE_ENTRIES 64
     - BYTES_PER_PTE 4
    */
    perfTlbMiss(*ppn);
    TLB[RR].virtualPageNumber = (addr>>10);
    TLB[RR].physPageNumber = *ppn;
    TLB[RR].valid = true;
    int index = RR;
    if(RR ==0)
    {   
        RR = 1;
    }   
    else
    {
        RR = 0;
    }
    return index;
}

int vmRead(Address addr)
{
    if(vmEnabled)
    {
        perfStartAddressTranslation(addr);
        int physicalPageNumber = 0;
        searchTLB(addr, &physicalPageNumber);
        int p = (physicalPageNumber<<10) | (addr & 0x3FF);
        perfEndAddressTranslation(p);
        return readWithCache(p);
    }
    else
    {
        return readWithCache(addr);
    }
}

void vmWrite(Address addr, int value)
{
    if(vmEnabled)
    {
        perfStartAddressTranslation(addr);
        int physicalPageNumber = 0;
        searchTLB(addr, &physicalPageNumber);
        int p = (physicalPageNumber<<10) | (addr & 0x3FF);
        perfEndAddressTranslation(p);
        writeWithCache(p, value);
    }
    else
    {
        writeWithCache(addr, value);
    }

}

void vmDisable()
{
    vmEnabled = false;
}

void vmEnable(Address pageTable)
{
    PageTableAddress = pageTable;
    vmEnabled = true;
}
