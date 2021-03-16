#include <interrupts/apic.hpp>

InputOutputPort* PIC1CommandPort = NULL;
InputOutputPort* PIC1DataPort    = NULL;

InputOutputPort* PIC2CommandPort = NULL;
InputOutputPort* PIC2DataPort    = NULL;

InputOutputPort* KeyboardPort    = NULL;

void InitPorts() 
{
    PIC1CommandPort = new InputOutputPort(PIC1_COMMAND);
    PIC1DataPort = new InputOutputPort(PIC1_DATA);
    PIC2CommandPort = new InputOutputPort(PIC2_COMMAND);
    PIC2DataPort = new InputOutputPort(PIC2_DATA);
    KeyboardPort = new InputOutputPort(0x60);
}


void InitApic() 
{
    uint8_t a1, a2;

    a1 = PIC1DataPort->Read();
    a2 = PIC2DataPort->Read();

    PIC1CommandPort->Write(ICW1_INIT | ICW1_ICW4);
    PIC2CommandPort->Write(ICW1_INIT | ICW1_ICW4);
    // Remap Pic Master to 0x20 IRQ offset.
    PIC1DataPort->Write(0x20);
    // Remap Pic Slave to 0x28 IRQ offset.
    PIC2DataPort->Write(0x28);

    PIC1DataPort->Write(4);
    PIC2DataPort->Write(2);

    PIC1DataPort->Write(ICW4_8086);
    PIC2DataPort->Write(ICW4_8086);

    PIC1DataPort->Write(a1);
    PIC2DataPort->Write(a2);
}


void EndPicInterruptPrimary()
{
    PIC1CommandPort->Write(PIC_EOI, false);
}

void EndPicInterruptSecondary()
{
    PIC2CommandPort->Write(PIC_EOI, false);
    PIC1CommandPort->Write(PIC_EOI, false);
}