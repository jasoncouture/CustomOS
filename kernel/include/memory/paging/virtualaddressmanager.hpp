#pragma once
#include <memory/paging/pagetableentry.hpp>

class VirtualAddressManager 
{
    public:
        VirtualAddressManager();
        VirtualAddressManager(PageTableEntry rootTable[512], bool freeOnDestory = false);
        void Map(void* virtualAddress, void* physicalAddress, bool writable = true);
        PageTableEntry* GetPageTableEntry(void *virtualAddress, bool create = false);
        void Map(void* physicalAddress, bool writable = true);
        void *GetPhysicalAddress(void* virtualAddress);
        void Activate();
        void *GetPageTableAddress();
        static VirtualAddressManager* GetKernelVirtualAddressManager();
    private:
        PageTableEntry* RootTable;
        bool active;
        bool FreeOnDestroy;
        static bool IsInitialized;
        static VirtualAddressManager* KernelVirtualAddressManager;

};

