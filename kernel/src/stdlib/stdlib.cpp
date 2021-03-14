extern "C"
{
    void VERIFY_NOT_REACHED()
    {
        // Trigger a page fault in the kernel?
        // Fire a special interrupt?
        *((unsigned long long *)0xFFFFFFFFFFFFFFFF) = 1;
        while (true)
            asm("hlt");
    }

    void __cxa_pure_virtual()
    {
        VERIFY_NOT_REACHED();
    }

    void __stack_chk_fail()
    {
        VERIFY_NOT_REACHED();
    }

    void __stack_chk_fail_local()
    {
        __stack_chk_fail();
    }

    int __cxa_atexit(void (*)(void *), void *, void *)
    {
        VERIFY_NOT_REACHED();
        return 0;
    }
}