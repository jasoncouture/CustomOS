#pragma once
#include "pagetableentry.hpp"

class VirtualAddressManager 
{
    public:
        VirtualAddressManager();
        VirtualAddressManager(PageTableEntry rootTable[512], bool freeOnDestory = false);
        void Map(void* virtualAddress, void* physicalAddress);
        void Activate();
    private:
        PageTableEntry* RootTable;
        bool FreeOnDestroy;

};

extern VirtualAddressManager KernelVirtualAddressManager;