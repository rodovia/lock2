#include "atapi.h"
#include "alloc/physical.h"
#include "ata.h"
#include "dllogic/api/dhelp.h"
#include "ide.h"
#include "intr.h"

#define UNUSED(x) ((void)(x))

static ata_check_result AtaCheck();
IDHelpDriverManager* __DriverMgr;

void AtapiReadCdRom(uint32_t lba,
                    uint32_t sectors,
                    uint16_t* buffer,
                    uint16_t maxByteCount);

static void DriverAtaInterrupt() __system;

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
    mgr->GetInterruptController(&ic);
    pci->FindDeviceByClass(0x1, 0x1, &devcs);

    auto vector = ic->GenerateVector();
    ic->AssociateVector(15, vector);
    ic->AssociateVector(14, vector);
    ic->HandleInterrupt(vector, DriverAtaInterrupt);

    ide::CreateController(devcs);
    ata_check_result ch = AtaCheck(); 
    if (ch == kAtaChkNoDrive)
    {
        return;
    }
    
    AtapiReadCdRom(0, 2, nullptr, 50);
    mgr->SetNotify(Driver_Notify);
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

static void AtaSelectDrive(ata_drive drive)
{
    port8 sel = GetRegister<uint8_t>(0x1F0, kAtaRegIoDriveSelectHead);
    (*sel) = (drive == kAtaDriveMaster) ? 0xA0 : 0xB0;
}

template<class R, class C>
static R PollUntil(CPort<R>& port, C conditional)
{
    R r;
    do
    {
        r = *port;
    } while(conditional(r));

    return r;
}

static ata_check_result AtaCheck()
{
    uint16_t status;
    port8 sectorcnt = GetRegister<uint8_t>(0x1F0, kAtaRegIoSectorCountRegister);
    port8 sectorn = GetRegister<uint8_t>(0x1F0, kAtaRegIoSectorNumber);
    port8 cyllw = GetRegister<uint8_t>(0x1F0, kAtaRegIoSectorCylinderLow);
    port8 cylmd = GetRegister<uint8_t>(0x1F0, kAtaRegIoSectorCylinderHigh);
    port8 cmd = GetRegister<uint8_t>(0x1F0, kAtaRegIoStatusCommand);

    AtaSelectDrive(kAtaDriveMaster);
    *sectorcnt = 0;
    *sectorn = 0;
    *cyllw = 0;
    *cylmd = 0;

    *cmd = 0xEC; /* identify */
    status = *cmd;
    if (status == 0)
    {
        return kAtaChkNoDrive;
    }
    
    if ((uint8_t)*cyllw != 0 ||
        (uint8_t)*cylmd != 0)
    {
        return kAtaChkAtapiDrive;
    }

    PollUntil(cmd, [](uint16_t status) 
    {
        return (status & (1 << 3)) != 0 || (status & 1) != 0;
    });
    return kAtaChkPataDrive;
}

/* Some code was taken from osdev wiki. */
void AtapiReadCdRom(uint32_t lba,
                    uint32_t sectors,
                    uint16_t* buffer,
                    uint16_t maxByteCount)
{
    port16 dma = 0x1F0;
    port8 lbalw = 0x1F0 + kAtaRegIoSectorCylinderLow;
    port8 lbahg = 0x1F0 + kAtaRegIoSectorCylinderHigh;
    port8 cmd = 0x1F0 + kAtaRegIoStatusCommand;

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

    AtaSelectDrive(kAtaDriveMaster);
    *lbalw = maxByteCount & 0xFF;
    *lbahg = maxByteCount >> 8;
    *cmd = 0xA0; /* ATAPI PACKET */

    for (uint8_t i = 0; i < 6; i++)
    {
        (*dma) = command16[i];
    }
}

static void DriverAtaInterrupt()
{

}
