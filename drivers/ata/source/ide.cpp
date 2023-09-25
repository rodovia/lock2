#include "ide.h"
#include "alloc/physical.h"
#include "ata.h"
#include "dllogic/api/dhelp.h"

#define ATA_PRIMARY_IO_PORT 0x1F0
#define ATA_PRIMARY_CTL_PORT 0x3F0
#define ATA_SECONDARY_IO_PORT 0x170
#define ATA_SECONDARY_CTL_PORT 0x376

#define ATA_MASTER_CHANNEL 0
#define ATA_SLAVE_CHANNEL 1

#define Warn(M, ...) do {  \
    IDHelpTerminal* __term; \
    __DriverMgr->GetTerminal(&__term); \
    __term->WriteFormat(M __VA_OPT__(,) __VA_ARGS__); } \
    while(0)

extern IDHelpDriverManager* __DriverMgr;

static void DriverAtaInterrupt(void*) __system;

/* Required by Clang/GCC */
extern "C"
void* memcpy(void* to, const void* from, size_t size)
{
    uint8_t* toc = reinterpret_cast<uint8_t*>(to);
    const uint8_t* froc = reinterpret_cast<const uint8_t*>(from);

    for (int i = 0; i < size; i++)
    {
        toc[i] = froc[i];
    }

    return toc;
}

extern "C"
void* memset(void* dest, int value, size_t size)
{
    uint8_t* toc = reinterpret_cast<uint8_t*>(dest);
    for (size_t i = 0; i < size; i++)
    {
        toc[i] = value;
    }

    return dest;
}

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

void ide::ide_t::IdeReadBuffer(int channel, int reg, 
                               uint32_t* buffer, uint32_t length)
{
    port32 ideb(Channels[channel].BusMaster);
    if (reg < 0x08)
    {
        port32 iob(Channels[channel].IoBase + reg);
        (*iob).ReadRepeated(buffer, length);
    }
    else if (reg < 0x0C)
    {
        port32 iob(Channels[channel].IoBase + reg - 0x06);
        (*iob).ReadRepeated(buffer, length);
    }
    else if (reg < 0x0E)
    {
        port32 ctlb(Channels[channel].CtlBase + reg);
        (*ctlb).ReadRepeated(buffer, length);
    }
    else if (reg < 0x16)
    {
        port32 iob(Channels[channel].BusMaster  + reg - 0x0E);
        (*ideb).ReadRepeated(buffer, length);
    }
}

ide::ide_t* ide::CreateController(IDHelpPciDevice* device)
{
    IDHelpTerminal* term;
    IDHelpThreadScheduler* thr;
    IDHelpMemoryAllocator* alloc;
    __DriverMgr->GetTerminal(&term);
    __DriverMgr->GetAllocator(&alloc);
    __DriverMgr->GetThreadManager(&thr);

    auto idspace = (uint32_t*)alloc->Allocate(512);
    auto state = (ide_t*)alloc->Allocate(sizeof(ide_t));
    memset(state, 0, sizeof(ide_t));
    thr->CreateMutex(&state->WriteMutex);

    uint32_t bar[5];
    int ptr = 0;
    uint16_t* reint;
    for (int i = 0x10; i != 0x28; i += 0x4)
    {
        bar[ptr] = device->ReadConfigurationSpace(i);
        ptr++;
    }

    SetupInterrupts(state);

    state->Channels[ATA_MASTER_CHANNEL].IoBase =  (bar[0] & 0xFFFFFFFC) + ATA_PRIMARY_IO_PORT * (!bar[0]);
    state->Channels[ATA_MASTER_CHANNEL].CtlBase = (bar[1] & 0xFFFFFFFC) + ATA_PRIMARY_CTL_PORT * (!bar[1]);
    state->Channels[ATA_SLAVE_CHANNEL].IoBase =  (bar[2] & 0xFFFFFFFC) + ATA_SECONDARY_CTL_PORT * (!bar[2]);
    state->Channels[ATA_SLAVE_CHANNEL].CtlBase = (bar[3] & 0xFFFFFFFC) + ATA_SECONDARY_CTL_PORT * (!bar[3]);
    state->Channels[ATA_MASTER_CHANNEL].BusMaster = (bar[4] & 0xFFFFFFFC);
    state->Channels[ATA_SLAVE_CHANNEL].BusMaster = (bar[4] & 0xFFFFFFFC) + 8;

    /* Disable interrupts */
    state->IdeWrite(ATA_MASTER_CHANNEL, ATA_REG_CONTROL, 0b10);
    state->IdeWrite(ATA_SLAVE_CHANNEL, ATA_REG_CONTROL, 0b10);

    int t, c, count = 0;
    for (t = 0; t < 2; t++)
    {
        for (c = 0; c < 2; c++)
        {
            auto type = kAtaChkAtaDrive;
            state->Devices[count].Reserved = 0;

            state->IdeWrite(t, ATA_REG_HDDEVSEL, 0xA0 | (c << 4));
            thr->HaltExecution(1);
            state->IdeWrite(t, ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
            thr->HaltExecution(1);

            auto status = state->IdeRead(t, ATA_REG_STATUS);
            if (status == 0)
            {
                count++;
                continue;
            }

            while (true)
            {
                status = state->IdeRead(t, ATA_REG_STATUS);
                if (status & ATA_SR_ERR)
                {
                    break;
                }

                if (!(status & ATA_SR_BSY) && (status & ATA_SR_DRQ))
                {
                    break;
                }
            }

            if (status & ATA_SR_ERR)
            {
                /* Likely an ATAPI device */
                uint8_t ch = state->IdeRead(t, ATA_REG_LBA1);
                uint8_t cyl = state->IdeRead(t, ATA_REG_LBA2);
                if ((ch == 0x14 && cyl == 0xEB)
                 || (ch == 0x69 && cyl == 0x96))
                {
                    type = kAtaChkAtapiDrive;
                }
                else 
                {
                    count++;
                    continue;
                }
                state->IdeWrite(t, ATA_REG_COMMAND, ATA_CMD_IDENTIFY_PACKET);
            }

            state->IdeReadBuffer(t, ATA_REG_DATA, idspace, 128);
            reint = reinterpret_cast<uint16_t*>(idspace);
            
            state->Devices[count].Model[40] = 0;
            state->Devices[count].Reserved = 1;
            state->Devices[count].Capabilities = reint[ATA_IDENT_CAPABILITIES];
            state->Devices[count].CommandSets = reint[ATA_IDENT_COMMANDSETS];
            state->Devices[count].Signature = reint[ATA_IDENT_DEVICETYPE];
            state->Devices[count].Channel = c;
            state->Devices[count].Drive = t;
            state->Devices[count].Type = type;
            state->Devices[count].Size = reint[ATA_IDENT_MAX_LBA];

            if (state->Devices[count].CommandSets & (1 << 10))
            {
                state->Devices[count].Size = reint[ATA_IDENT_MAX_LBA_EXT];
            }
            for (int i = 0; i < 40; i += 2)
            {
                state->Devices[count].Model[i + 1] = reint[ATA_IDENT_MODEL + i + 1];
                state->Devices[count].Model[i] = reint[ATA_IDENT_MODEL + i];
            }

            count++;
        }
    }

    /* TODO: This Allocate must be AlignedAlloc to 64 boundary */
    state->PhysicalRegions = (direct_memory*)alloc->Allocate(sizeof(direct_memory));
    state->PhysicalRegions->Address = (uint64_t)alloc->Allocate(1024);
    state->PhysicalRegions->LastEntry = (1 << 15);
    state->PhysicalRegions->TransferSize = 1024;

    memset(reinterpret_cast<void*>(state->PhysicalRegions->Address), 0, 1024);

    port32 bm = state->Channels[ATA_MASTER_CHANNEL].BusMaster + 0x4;
    (*bm) = (uint64_t)state->PhysicalRegions;

    /* Re-enable interrupts */
    state->IdeWrite(ATA_MASTER_CHANNEL, ATA_REG_CONTROL, 0);
    state->IdeWrite(ATA_SLAVE_CHANNEL, ATA_REG_CONTROL, 0);

    alloc->Free(idspace);

    return state;
}

void ide::ide_t::SelectDevice(int channel, int slavemaster)
{
    auto& ch = Channels[channel];

    port8 sel = ch.IoBase + 6;
    (*sel) = (slavemaster << 4);
    port8 alts = ch.CtlBase;

    /* INB 15 times to wait the drive to select the bus */
    for (int i = 0; i < 15; i++)
    {
        ((void)(*alts).Read());
    }
}

void ide::SetupInterrupts(ide_t* state)
{
    IDHelpPci* pci;
    IDHelpPciDevice* device;
    IDHelpInterruptController* ic;
    IDHelpTerminal* t;
    __DriverMgr->GetPci(&pci);
    __DriverMgr->GetInterruptController(&ic);
    __DriverMgr->GetTerminal(&t);

    pci->FindDeviceByClass(0x1, 0x1, &device);
    uint8_t pif = device->ReadConfigurationSpace(0x8) & 0xFF00;
    if (!(pif & 1))
    {
        int v = ic->GenerateVector();
        ic->AssociateVector(14, v);
        ic->HandleInterrupt(v, DriverAtaInterrupt, state);
    }

    if (!(pif & (1 << 2)))
    {
        int v = ic->GenerateVector();
        ic->AssociateVector(15, v);
        ic->HandleInterrupt(v, DriverAtaInterrupt, state);
    }
}

void DriverAtaInterrupt(void* ct)
{
    auto state = reinterpret_cast<ide::ide_t*>(ct);
    state->WriteMutex->Release();
}
