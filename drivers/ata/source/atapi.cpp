#include "mass_storage.h"
#include "alloc/physical.h"
#include "ata.h"
#include "dllogic/api/dhelp.h"
#include "ide.h"
#include <vector> 

#define Warn(M, ...) do {  \
    IDHelpTerminal* __term; \
    __DriverMgr->GetTerminal(&__term); \
    __term->WriteFormat(M __VA_OPT__(,) __VA_ARGS__); } \
    while(0)
#define UNUSED(x) ((void)(x))

IDHelpDriverManager* __DriverMgr;

__system extern "C" 
void Driver_Init(IDHelpDriverManager* mgr)
{
    __DriverMgr = mgr;
    IDHelpTerminal* term;
    IDHelpPci* pci;
    IDHelpPciDevice* devcs;
    mgr->GetTerminal(&term);
    mgr->GetPci(&pci);
    pci->FindDeviceByClass(0x1, 0x1, &devcs);
    if (devcs == nullptr)
    {
        return;
    }

    auto i = ide::CreateController(devcs);
    mgr->SetRole(kDHelpDriverRoleDisk);
    mgr->SetInterface(new CAtaMassStorage(i));
}


int AtaWaitStatus(ide::channel& channel, int st, int timeout = 10)
{
    uint8_t status;
    IDHelpThreadScheduler* sched;
    __DriverMgr->GetThreadManager(&sched);
    port8 ns = channel.CtlBase;

    while (timeout)
    {
        status = *ns;
        if (!(status & ATA_SR_BSY))
        {
            if ((status & (ATA_SR_DF | ATA_SR_ERR)))
            {
                return 1;
            }
            return status & st;
        }

        sched->HaltExecution(1);
        timeout--;
    }

    return -1;
}

/* Some code was taken from osdev wiki. */
void AtapiReadCdRom(ide::ide_t* state,
                    uint32_t lba,
                    uint8_t* buffer,
                    uint32_t sectors,
                    uint32_t maxByteCount)
{
    state->WriteMutex->Lock();
    ide::channel& channel = state->Channels[state->CurrentChannel];

    port8 lbalw = channel.IoBase + kAtaRegIoSectorCylinderLow;
    port8 lbahg = channel.IoBase + kAtaRegIoSectorCylinderHigh;
    port8 cmd = channel.IoBase + kAtaRegIoStatusCommand;
    port16 data = channel.IoBase + kAtaRegIoData;
    port8 feat = channel.IoBase + 1;
    port8 idecmd = channel.BusMaster;

    (*idecmd) = 0;
    state->SelectDevice(state->CurrentChannel, state->CurrentDevice);

    (*feat) = 0;
    *lbalw = maxByteCount & 0xFF;
    *lbahg = maxByteCount >> 8;
    
    *cmd = 0xA0; /* ATAPI PACKET */
    AtaWaitStatus(channel, ATA_SR_DRQ);

    volatile uint8_t command[12] = {0xA8, 0,
	                    static_cast<uint8_t>((lba >> 0x18) & 0xFF), 
                        static_cast<uint8_t>((lba >> 0x10) & 0xFF), 
                        static_cast<uint8_t>((lba >> 0x08) & 0xFF),
	                    static_cast<uint8_t>((lba >> 0x00) & 0xFF),
                        static_cast<uint8_t>((sectors >> 0x18) & 0xFF), 
                        static_cast<uint8_t>((sectors >> 0x10) & 0xFF), 
                        static_cast<uint8_t>((sectors >> 0x08) & 0xFF), 
                        static_cast<uint8_t>((sectors >> 0x00) & 0xFF),
	                    0, 0};
    volatile uint16_t* command16 = reinterpret_cast<volatile uint16_t*>(command);

    for (uint8_t i = 0; i < 6; i++)
    {
        (*data) = command16[i];
    }

    state->WriteMutex->Lock();

    uint8_t high = *lbahg;
    uint8_t low = *lbalw;
    BochsDebugBreak;
    (*data).ReadRepeated(reinterpret_cast<uint16_t*>(buffer), 
                                ((high << 8) | low) / 2);
}