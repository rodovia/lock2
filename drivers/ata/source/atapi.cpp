#include "atapi.h"
#include "ata.h"
#include "dllogic/api/dhelp.h"
#include "ide.h"
#include "intr.h"

#define Warn(M, ...) do {  \
    IDHelpTerminal* __term; \
    __DriverMgr->GetTerminal(&__term); \
    __term->WriteFormat(M __VA_OPT__(,) __VA_ARGS__); } \
    while(0)
#define UNUSED(x) ((void)(x))

IDHelpDriverManager* __DriverMgr;

void AtapiReadCdRom(ide::ide_t* state,
                    uint32_t lba,
                    uint32_t sectors,
                    uint16_t* buffer,
                    uint16_t maxByteCount);

__system extern "C" 
void Driver_Init(IDHelpDriverManager* mgr)
{
    __DriverMgr = mgr;
    IDHelpTerminal* term;
    IDHelpPci* pci;
    IDHelpPciDevice* devcs;
    IDHelpInterruptController* ic;
    mgr->GetTerminal(&term);
    mgr->GetPci(&pci);
    pci->FindDeviceByClass(0x1, 0x1, &devcs);

    auto i = ide::CreateController(devcs);
    AtapiReadCdRom(i, 180, 2, nullptr, 1024);
    mgr->SetRole(kDHelpDriverRoleDisk);
}

uint64_t Driver_Notify(uint32_t message,
                    uint64_t lparam, uint64_t wparam,
                    uint64_t flags) 
{
    UNUSED(message); UNUSED(lparam);
    UNUSED(wparam); UNUSED(flags);
    return 0;
}

/* Some code was taken from osdev wiki. */
void AtapiReadCdRom(ide::ide_t* state,
                    uint32_t lba,
                    uint32_t sectors,
                    uint16_t* buffer,
                    uint16_t maxByteCount)
{
    ide::channel& channel = state->Channels[state->CurrentChannel];
    port16 dma = channel.BusMaster;
    port8 lbalw = channel.IoBase + kAtaRegIoSectorCylinderLow;
    port8 lbahg = channel.IoBase + kAtaRegIoSectorCylinderHigh;
    port8 cmd = channel.IoBase + kAtaRegIoStatusCommand;
    port16 data = channel.IoBase + kAtaRegIoData;
    port8 driveselect = channel.IoBase + kAtaRegIoDriveSelectHead;
    port8 feat = channel.IoBase + 1;
    port8 idecmd = channel.BusMaster;
    port8 idestatus = channel.BusMaster + 2;

    (*idestatus) = 4; /* To clear bit 4 of DMA status */
    (*feat) = 1;

    (*idecmd) = 0;             /* Disable DMA */
    (*idecmd) = (1 << 3) | 1; /* Set operation to read and reenable DMA */
    (*driveselect) = (state->CurrentChannel << 4) | (1 << 5) | (1 << 7) | (1 << 6);
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

    *lbalw = maxByteCount & 0xFF;
    *lbahg = maxByteCount >> 8;
    *cmd = 0xA0; /* ATAPI PACKET */

    for (uint8_t i = 0; i < 6; i++)
    {
        (*data) = command16[i];
    }
}
