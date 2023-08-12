#include "ide.h"
#include "dllogic/api/dhelp.h"
#include <vector>

#define ATA_PRIMARY_IO_PORT 0x1F0
#define ATA_PRIMARY_CTL_PORT 0x3F0
#define ATA_SECONDARY_IO_PORT 0x170
#define ATA_SECONDARY_CTL_PORT 0x376

#define ATA_MASTER_CHANNEL 0
#define ATA_SLAVE_CHANNEL 1

extern IDHelpDriverManager* __DriverMgr;

void ide::ide_t::IdeWrite(int channel, int reg, unsigned char data)
{
    port8 ctlb(Channels[channel].CtlBase + reg - 0x0A);
    port8 ideb(Channels[channel].BusMaster + reg - 0x0E);

    if (reg < 0x08)
    {
        port8 iob(Channels[channel].IoBase + reg);
        (*iob) = data;
    }
    else if (reg < 0x0C)
    {
        port8 iob(Channels[channel].IoBase + reg - 0x06);
        (*iob) = data;
    }
    else if (reg < 0x0E)
    {
        (*ctlb) = data;
    }
    else if (reg < 0x16)
    {
        (*ideb) = data;
    }
}

uint8_t ide::ide_t::IdeRead(int channel, int reg)
{
    uint8_t result = -1;
    port8 ctlb(Channels[channel].CtlBase);
    port8 ideb(Channels[channel].BusMaster);

    if (reg < 0x08)
    {
        port8 iob(Channels[channel].IoBase + reg);
        result = (*iob);
    }
    else if (reg < 0x0C)
    {
        port8 iob(Channels[channel].IoBase + reg - 0x06);
        result = (*iob);
    }
    else if (reg < 0x0E)
    {
        result = (*ctlb);
    }
    else if (reg < 0x16)
    {
        result = (*ideb);
    }

    return result;
}

void ide::CreateController(IDHelpPciDevice* device)
{
    ide::ide_t state;
    uint32_t bar[5];
    int ptr = 0;
    for (int i = 0x10; i != 0x28; i += 0x4)
    {
        bar[ptr] = device->ReadConfigurationSpace(i);
        ptr++;
    }
    
    state.Channels[ATA_MASTER_CHANNEL].IoBase =  (bar[0] & 0xFFFFFFFC) + ATA_PRIMARY_IO_PORT * (!bar[0]);
    state.Channels[ATA_MASTER_CHANNEL].CtlBase = (bar[1] & 0xFFFFFFFC) + ATA_PRIMARY_CTL_PORT * (!bar[1]);
    state.Channels[ATA_SLAVE_CHANNEL].IoBase =  (bar[2] & 0xFFFFFFFC) + ATA_SECONDARY_CTL_PORT * (!bar[2]);
    state.Channels[ATA_SLAVE_CHANNEL].CtlBase = (bar[3] & 0xFFFFFFFC) + ATA_SECONDARY_CTL_PORT * (!bar[3]);
    state.Channels[ATA_MASTER_CHANNEL].BusMaster = (bar[4] & 0xFFFFFFFC);
    state.Channels[ATA_SLAVE_CHANNEL].BusMaster = (bar[4] & 0xFFFFFFFC) + 8;

    /* Disable interrupts */
    state.IdeWrite(ATA_MASTER_CHANNEL, ATA_REG_CONTROL, 0b10);
    state.IdeWrite(ATA_SLAVE_CHANNEL, ATA_REG_CONTROL, 0b10);

    int t, c, count = 0;
    for (t = 0; t < 2; t++)
    {
        for (c = 0; c < 2; c++)
        {
            state.Devices[count].Reserved = 0;

            state.IdeWrite(t, ATA_REG_HDDEVSEL, 0xA0 | (c << 4));
            /* Sleep for 1 (10) milisseconds */
            

            count++;
        }
    }
}
