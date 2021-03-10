#include "apic.hpp"

InputOutputPort PIC1CommandPort = InputOutputPort(PIC1_COMMAND);
InputOutputPort PIC1DataPort = InputOutputPort(PIC1_DATA);

InputOutputPort PIC2CommandPort = InputOutputPort(PIC2_COMMAND);
InputOutputPort PIC2DataPort = InputOutputPort(PIC2_DATA);


void Wait() {
    WaitPort.Write(0);
}

void InitApic() 
{
    uint8_t a1, a2;

    a1 = PIC1DataPort.Read();
    Wait();
    a2 = PIC2DataPort.Read();
    Wait();

    PIC1CommandPort.Write(ICW1_INIT | ICW1_ICW4);
    Wait();
    PIC2CommandPort.Write(ICW1_INIT | ICW1_ICW4);
    Wait();
    // Remap Pic Master to 0x20 IRQ offset.
    PIC1DataPort.Write(0x20);
    Wait();
    // Remap Pic Slave to 0x28 IRQ offset.
    PIC2DataPort.Write(0x28);
    Wait();

    PIC1DataPort.Write(4);
    Wait();
    PIC2DataPort.Write(2);
    Wait();

    PIC1DataPort.Write(ICW4_8086);
    Wait();
    PIC2DataPort.Write(ICW4_8086);
    Wait();

    PIC1DataPort.Write(a1);
    Wait();
    PIC2DataPort.Write(a2);
    Wait();
}


void EndPicInterruptPrimary()
{
    PIC1CommandPort.Write(PIC_EOI);
}

void EndPicInterruptSecondary()
{
    PIC2CommandPort.Write(PIC_EOI);
    PIC1CommandPort.Write(PIC_EOI);
}