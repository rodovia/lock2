#include "pci.h"
#include "acpi/acpica.h"
#include "acpi/acpica/aclocal.h"
#include "acpi/acpica/acpixf.h"
#include "acpi/acpica/acrestyp.h"
#include "acpi/acpica/actypes.h"
#include "arch/i386/port.h"
#include "dllogic/api/dhelp.h"
#include "internals/iterator.h"
#include "terminal.h"

#include <algorithm>

#define PCI_MULTIFUNCTION (1 << 7)

void pci::CPci::DiscoverDevices()
{
    pci_device root = pci_device(0, 0, 0);
    if ((root.GetHeaderType() & PCI_MULTIFUNCTION) == 0)
    {
        this->DiscoverBus(0);
        return;
    }
    
    uint8_t function;
    for (function = 1; function < 8; function++)
    {
        pci_device devc(0, 0, function);
        if (devc.GetVendor() == 0xFFFF)
        {
            return;
        }
    }

}

void pci::CPci::DiscoverBus(uint8_t bus)
{
    Warn("bus %i\n", bus);
    uint8_t device;
    for (device = 0; device < 32; device++)
    {
        this->DiscoverDevice(bus, device);
    }
}

void pci::CPci::DiscoverDevice(uint8_t bus, uint8_t device)
{
    uint8_t function = 0;
    pci_device devc(bus, device, function);
    
    if (devc.GetVendor() == 0xFFFF) 
    {
        return;
    }

    if ((devc.GetHeaderType() & PCI_MULTIFUNCTION) != 0)
    {
        for (function = 1; function < 7; function++)
        {
            this->DiscoverFunction(bus, device, function);
        }
    }
}

void pci::CPci::DiscoverFunction(uint8_t bus, uint8_t device, 
                                uint8_t function)
{
    pci_device dev(bus, device, function);
    uint16_t cls = dev.GetClassSubClass();
    
    if (dev.GetVendor() != 0xFFFF)
    {
        m_Devices.push_back(dev);
    }

    if ((cls >> 7) == 0x6 &&
        (cls & 0xFF) == 0x4)
    {
        uint8_t secbus = dev.GetSecondaryBus();
        this->DiscoverBus(secbus);
    }
}

void pci::CPci::EnumerateDevices(IDHelpPciDevice **outDevices, size_t *length)
{
    (*outDevices) = m_Devices.data();
    (*length) = m_Devices.size();
}

void
pci::CPci::FindDeviceByClass(uint8_t klass, uint8_t subclass, IDHelpPciDevice** outDevice)
{
    auto pred = [klass, subclass](const pci_device& d)
    {
        uint16_t clss = d.GetClassSubClass();
        return clss == ((klass << 8) | subclass);
    };

    auto iter = std::find_if(m_Devices.begin(), m_Devices.end(), pred);
    if (iter == m_Devices.end())
    {
        return;
    }
    (*outDevice) = iter + 0;
}

void pci::CPci::FindDeviceExact(uint8_t bus, uint8_t device, IDHelpPciDevice **dev)
{
    if (dev == nullptr)
    {
        return;
    }

    auto pred = [bus, device](const pci_device& d)
    {
        return d.Bus == bus && d.GetDevice() && device;
    };
    
    /* Hack to get the internal pointer of iterator. */
    (*dev) = std::find_if(m_Devices.begin(), m_Devices.end(), pred) + 0;
}

uint32_t pci::pci_device::ReadConfigurationSpace(uint8_t offset) const
{
    port32 address = 0xCF8;
    port32 data = 0xCFC;
    uint32_t lbus = static_cast<uint32_t>(Bus);
    uint32_t lslot = static_cast<uint32_t>(Slot);
    uint32_t lfunc = static_cast<uint32_t>(Function);

    uint32_t ad = (1 << 31) | (lbus << 16) | (lslot << 11) | 
                (lfunc << 8) | (offset & 0xFC);
    *address = ad;
    return (uint32_t)*data;
}

void pci::pci_device::WriteConfigurationSpace(uint8_t offset, uint32_t nvalue)
{
    port32 address = 0xCF8;
    port32 data = 0xCFC;
    uint32_t lbus = static_cast<uint32_t>(Bus);
    uint32_t lslot = static_cast<uint32_t>(Slot);
    uint32_t lfunc = static_cast<uint32_t>(Function);

    uint32_t ad = (1 << 31) | (lbus << 16) | (lslot << 11) | 
                (lfunc << 8) | (offset & 0xFC);
    *address = ad;
    *data = nvalue;
}

uint16_t pci::pci_device::GetVendor() const
{
    uint32_t vendor = this->ReadConfigurationSpace(0);
    return (vendor & 0xFFFF);
}

uint8_t pci::pci_device::GetDevice() const
{
    uint32_t device = this->ReadConfigurationSpace(0);
    return (uint16_t)(device >> 15);
}

uint8_t pci::pci_device::GetHeaderType() const
{
    uint32_t hd = this->ReadConfigurationSpace(0xC);
    return (hd >> 16);
}

uint16_t pci::pci_device::GetClassSubClass() const
{
    uint32_t cls = this->ReadConfigurationSpace(0x8);
    return (cls >> 16);
}

uint8_t pci::pci_device::GetSecondaryBus() const
{
    uint16_t cls = this->GetClassSubClass();
    if ((cls >> 7) != 0x6 && (cls & 0xFF) != 0x4)
    {
        return 0xFF;
    }

    uint32_t number = this->ReadConfigurationSpace(0x18);
    return number >> 7;
}

/*^^^^^ CPci and pci_device ^^^^^*/

static unsigned int WalkDeviceTree(ACPI_HANDLE object, 
                                   UINT32, 
                                   void*, ACPI_HANDLE* ret)
{
    /* Already matches what we want: A PCI root bus */
    (*ret) = object;

    /* AE_TERMINATE is not defined :( */
    return AE_TIME;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwritable-strings"

void pci::RouteAcpiInterrupt(uint16_t device)
{
    uint32_t count = 0;
    ACPI_HANDLE devc;
    ACPI_BUFFER output = { .Length = ACPI_ALLOCATE_BUFFER, .Pointer = nullptr };
    auto routing = reinterpret_cast<ACPI_PCI_ROUTING_TABLE*>(output.Pointer);

    AcpiGetDevices("PNP0A03", WalkDeviceTree, &count, &devc);
    AcpiGetIrqRoutingTable(devc, &output);
    while (routing->Length != 0)
    {
        
        routing++;
    }
}

#pragma GCC diagnostic pop
