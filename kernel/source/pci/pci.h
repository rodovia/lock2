#pragma once

#include "dllogic/api/dhelp.h"
#include <stdint.h>
#include <vector>

namespace pci
{

struct pci_device : public IDHelpPciDevice
{
    constexpr pci_device(uint8_t bus, uint8_t slot, uint8_t function)
        : Bus(bus),
          Slot(slot),
          Function(function)
    {
    }

    constexpr pci_device()
        : Bus(0xFF),
          Slot(0xFF),
          Function(0xFF)
    {
    }

    bool IsValid() const override
    {
        return Bus != 0xFF && Slot != 0xFF &&
               Function != 0xFF;
    }

    ~pci_device() override
    {
    }

    uint16_t GetVendor() const override;
    uint8_t GetHeaderType() const;
    uint8_t GetDevice() const override;
    uint16_t GetClassSubClass() const override;
    uint8_t GetSecondaryBus() const;
    uint32_t ReadConfigurationSpace(uint8_t offset) const override;
    void WriteConfigurationSpace(uint8_t offset, uint32_t nvalue) override;

    uint8_t Bus, Slot, Function;
};

class CPci : public IDHelpPci
{
public:
    static CPci& GetInstance()
    {
        static CPci p;
        return p;
    }

    void DiscoverDevices();
    void EnumerateDevices(IDHelpPciDevice** outDevices, size_t* length) override;
    void FindDeviceExact(uint8_t bus, uint8_t device, IDHelpPciDevice** dev) override;
    void FindDeviceByClass(uint8_t klass, uint8_t subclass, IDHelpPciDevice** outDevices) override;
private:
    CPci()
    {
        this->DiscoverDevices();
    }; 
    void DiscoverBus(uint8_t bus);
    void DiscoverDevice(uint8_t bus, uint8_t device);
    void DiscoverFunction(uint8_t bus, uint8_t device, uint8_t function);

    std::vector<pci_device> m_Devices;
};

}
