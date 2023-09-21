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

    auto i = ide::CreateController(devcs);
    mgr->SetRole(kDHelpDriverRoleDisk);
    mgr->SetInterface(new CAtaMassStorage(i));
}

static void CheckForErrors(ide::ide_t* state)
{
    ide::channel& channel = state->Channels[state->CurrentChannel];
    port16 data = channel.IoBase + kAtaRegIoData;
    port8 cmd = channel.IoBase + kAtaRegIoStatusCommand;
    Warn("status = 0x%p", (*cmd).Read());

    static volatile uint8_t sensecmd[6] = {
        0x03, 0, /* REQUEST SENSE, Fixed format */
        0, 0,
        252, 0
    };

    (*cmd) = 0xA0;
    for (uint16_t o = 0; o < 6; o++)
    {
        (*data) = sensecmd[o];
    }

    state->WriteMutex->Lock();
    BochsDebugBreak;
}

/* Some code was taken from osdev wiki. */
void AtapiReadCdRom(ide::ide_t* state,
                    uint32_t lba,
                    uint8_t* buffer,
                    uint32_t sectors,
                    uint32_t maxByteCount)
{
    ide::channel& channel = state->Channels[state->CurrentChannel];

    state->WriteMutex->Lock();
    port16 dma = channel.BusMaster;
    port8 lbalw = channel.IoBase + kAtaRegIoSectorCylinderLow;
    port8 lbahg = channel.IoBase + kAtaRegIoSectorCylinderHigh;
    port8 cmd = channel.IoBase + kAtaRegIoStatusCommand;
    port16 data = channel.IoBase + kAtaRegIoData;
    port8 driveselect = channel.IoBase + kAtaRegIoDriveSelectHead;
    port8 feat = channel.IoBase + 1;
    port8 idecmd = channel.BusMaster;
    port8 idestatus = channel.BusMaster + 2;

    (*idecmd) = 0;
    (*driveselect) = (state->CurrentDevice << 4); /* Set current device and enable LBA */
    (*feat) = 1;

    // (*idecmd) = 0;             /* Disable DMA */
    (*idecmd) = (1 << 3) | 1; /* Set operation to read and reenable DMA */
    *lbalw = (uint16_t)maxByteCount & 0xFF;
    *lbahg = (uint16_t)maxByteCount >> 8;
    *cmd = 0xA0; /* ATAPI PACKET */

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

    state->WriteMutex->Lock(); /* Lock again so the function will halt 
                                (and only unlock when the interrupt is triggered) */
    CheckForErrors(state);
}
