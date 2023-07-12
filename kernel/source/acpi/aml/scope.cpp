#include "aml.h"
#include <algorithm>

void acpi::aml_method::Call(acpi::CMachineLanguageParser& mlp, 
                            uint8_t argc, aml_register* argv)
{
    if (argc > ArgumentCount)
    {
        Warn("aml_method::Call: Excess arguments (%i), truncating to fit %i\n", 
                argc, ArgumentCount);
        argc = ArgumentCount;
    }

    memcpy(mlp.m_ArgVars, argv, argc);
    mlp.SetInstructionPointer((uint64_t)Begin);
    mlp.ExecuteUntil(Length);
}

acpi::aml_package* 
acpi::aml_package::FindObject(std::string_view query)
{
    auto pred = [query](aml_package* const& p)
    {
        return p->Name.compare(query);
    };
    return *std::find_if(Child.begin(), Child.end(), pred);
}
