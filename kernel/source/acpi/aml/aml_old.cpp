#include "aml.h"
#include "acpi/tables.h"
#include "alloc/physical.h"
#include "terminal.h"

#include <string>
#include <string_view>

static void SplitString(std::vector<std::string_view>& ref, 
                        std::string str, char separator)
{
    int oldIndex = 0;
    std::string data;
    for (uint64_t i = 0; i < str.length(); i++)
    {
        char const& c = str[i];

        if (c == separator)
        {
            data = str.substr(oldIndex);
            ref.push_back({data.data(), i});
            oldIndex = i;
        }
    }

    if (ref.empty())
    {
        ref.push_back(std::string_view(str.data()));
    }
}

acpi::CMachineLanguageParser::CMachineLanguageParser(acpi::system_desc_header* dsdt)
    : m_CurrentScope(nullptr),
    m_Root(nullptr),
    m_Data(reinterpret_cast<uint8_t*>(PaAdd(dsdt, sizeof(acpi::system_desc_header)))),
    m_InstructionPointer(0)
{
    if (dsdt == nullptr)
    {
        Warn("CMachineLanguageParser: DSDT is null !\n");
        return;
    }

    m_CodeSize = dsdt->Length - sizeof(acpi::system_desc_header);
}

void acpi::CMachineLanguageParser::SetInstructionPointer(uint64_t ptr)
{
    m_InstructionPointer = ptr;
}

void acpi::CMachineLanguageParser::ExecuteUntil(uint64_t end)
{
    bool error;
    this->InitializeRoot();

    if (end == 0)
    {
        end = m_CodeSize;
    }

    end += m_InstructionPointer;
    do 
    {
        if (m_InstructionPointer >= end)
        {
            break;
        }

        error = this->Think();
        m_InstructionPointer++;
    } while (!error);
}

bool acpi::CMachineLanguageParser::Think()
{
    uint8_t instruction = m_Data[m_InstructionPointer];
    this->QuitInnerScope();

    if (instruction == 0x5B)
    {
        m_IsExtOp = true;
        return false;
    }

    switch (instruction)
    {
    case 0x10: /* SCOPE */
    {
        std::string_view parent, name;
        uint32_t length = this->HandlePackageLength();
        this->HandleScopeName(parent, name);
        if (parent.length() != 0)
        {
            aml_package* object = this->FindSingleObject(parent);
            if (!object)
            {
                Error("Unknown compound %s!\n", parent);
                return true;
            }

            m_CurrentScope = new aml_package(name, object, m_InstructionPointer, length);
            break;
        }

        m_CurrentScope = new aml_package(name, m_CurrentScope, m_InstructionPointer, length);
        break;
    }
    case 0x14: /* Method */
    {
        std::string_view null, name;
        uint32_t length = this->HandlePackageLength();
        this->HandleScopeName(null, name);
        uint8_t flags = m_Data[m_InstructionPointer];

        aml_method* meth = new aml_method(name, m_CurrentScope, m_InstructionPointer + 1, length);
        meth->ArgumentCount = flags & 3;
        meth->Serialized = flags & (1 << 3);
        meth->SyncLevel = flags & (15 << 4);
        m_InstructionPointer += length - (name.length());
        break;
    }
    case 0x82:
    {
        if (m_IsExtOp) /* Device */
        {
            uint32_t length = this->HandlePackageLength();
            std::string_view null, name;
            this->HandleScopeName(null, name);
            m_CurrentScope = new aml_device(name, m_CurrentScope, 0, length);
            break;
        }
    }
    case 0x08: /* Name */
    {
        
        break;
    };
    case 0xA4: /* Return */
        
        break;
    };

    if (instruction != 0x5B)
    {
        m_IsExtOp = false;
    }

    return false;
}

void acpi::CMachineLanguageParser::InitializeRoot()
{
    if (m_Root != nullptr)
    {
        return;
    }

    m_Root = new aml_package("ROOT", nullptr, 0, 0);
    new aml_package("_GPE_", m_Root, 0, 0);
    new aml_package("_SB_", m_Root, 0, 0);
    new aml_package("_SI_", m_Root, 0, 0);
    m_CurrentScope = m_Root;
}

uint32_t acpi::CMachineLanguageParser::HandlePackageLength()
{
    uint8_t instruction = m_Data[++m_InstructionPointer];
    uint8_t fwBytes = (instruction & (3 << 6)) >> 6;
    uint32_t final = instruction & 15;
    if (fwBytes == 0)
    {
        return instruction;
    }

    for (int i = 0; i < fwBytes; i++)
    {
        instruction = m_Data[++m_InstructionPointer];
        final |= (instruction << (i == 1 ? 4 : 8*i));
    }

    return final;
}

void acpi::CMachineLanguageParser::HandleScopeName(std::string_view& parent, std::string_view& name)
{
    uint8_t instruction = m_Data[++m_InstructionPointer];
    char* buffer = (char*)&m_Data[m_InstructionPointer];

    if (instruction == 0x2E) /* Dual name */
    {
        buffer++;
        m_InstructionPointer += 8;
        parent = std::string_view(buffer, 4);
        name = std::string_view(buffer + 4, 4);
        return;
    }

    m_InstructionPointer += 4;
    name = std::string_view(buffer, 4);
}

acpi::aml_package* 
acpi::CMachineLanguageParser::FindSingleObject(std::string_view object)
{
    aml_package* package = m_CurrentScope;
    auto pred = [object](aml_package* const& p) -> bool
    {
        return object.compare(p->Name);
    };

    auto iter = std::find_if(package->Child.begin(), package->Child.end(), pred);
    package = *iter;

    return package;
}

void acpi::CMachineLanguageParser::QuitInnerScope()
{
    if (m_CurrentScope->Parent == nullptr)
    {
        return;
    }

    if (m_Data + m_InstructionPointer >= 
        (m_Data + m_CurrentScope->Begin) + m_CurrentScope->Length)
    {
        m_CurrentScope = m_CurrentScope->Parent;
    }
}
