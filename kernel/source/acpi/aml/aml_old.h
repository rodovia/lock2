#pragma once

#include <string>
#include <type_traits>
#include <vector>
#include "acpi/tables.h"
#include <stdint.h>

namespace acpi 
{

enum aml_package_type : unsigned int
{
    kAmlPackageTypeScope,
    kAmlPackageTypeMethod,
    kAmlPackageTypeDevice,
    kAmlPackageTypePackage,
    kAmlPackageTypeComputationalIntegerData
};

enum aml_method_arg_type : unsigned int
{
    kAmlMethodArgArgRef, /* Value is a ArgX offset */
    kAmlMethodArgLocalRef, /* Value is a LocalX offset */
    kAmlMethodArgComputationalData, /* Value is an integer or a ptr to string */
    kAmlMethodArgPackage /* Value is a ptr to package */
};

struct aml_package
{
    static inline auto Type = kAmlPackageTypePackage;
    aml_package(std::string_view name, aml_package* parent, 
            uint32_t begin, uint32_t size)
        : Parent(parent),
          Length(size),
          Begin(begin)
    {
        Name = name;
        if (parent != nullptr)
        {
            aml_package* t = this;
            parent->Child.push_back(t);
        }
    }

    aml_package()
        : Parent(nullptr),
          Length(0),
          Begin(0)
    {
    }

    aml_package* FindObject(std::string_view query);

    std::vector<aml_package*> Child;
    std::string_view Name;
    aml_package* Parent;
    uint32_t Length;
    uint32_t Begin;
};

template<class _Ty>
struct aml_integer_data : public aml_package
{
    static_assert(std::is_integral_v<_Ty>);
    static inline auto Type = kAmlPackageTypeComputationalIntegerData;
    using DataType = _Ty;

    aml_integer_data(std::string_view name, _Ty data, 
                    aml_package* parent, size_t begin)
        : aml_package(name, parent, begin, sizeof(_Ty)),
          Value(data) 
    {
    };

    _Ty Value;
};

struct aml_method : public aml_package
{
    static inline auto Type = kAmlPackageTypeMethod;
    using aml_package::aml_package;

    uint8_t ArgumentCount;
    uint8_t Serialized;
    uint8_t SyncLevel;

    void Call(class CMachineLanguageParser& mlp, 
            uint8_t argc, struct aml_register* regs);
};

struct aml_device : public aml_package
{
    using aml_package::aml_package;
    static inline auto Type = kAmlPackageTypeDevice;
};

/* An register is an ArgX or LocalX variable */
struct aml_register
{
    aml_method_arg_type Type;
    uint64_t Value;
};

class CMachineLanguageParser
{
    friend struct aml_method;

public:
    CMachineLanguageParser(acpi::system_desc_header* dsdt);
    ~CMachineLanguageParser()
    {
        if (m_Root != nullptr)
            delete m_Root;
    };

    void ExecuteUntil(uint64_t end = 0);
    void SetInstructionPointer(uint64_t ptr);
    aml_package* FindSingleObject(std::string_view name);

private:
    uint32_t HandlePackageLength();
    void HandleScopeName(std::string_view& parent, std::string_view& name);
    std::string HandleStringName();
    bool Think();

    void InitializeRoot();
    void QuitInnerScope();

private:
    bool m_IsExtOp;
    aml_package* m_CurrentScope;
    aml_package* m_Root;

    uint8_t* m_Data;
    uint32_t m_CodeSize;
    uint32_t m_InstructionPointer;
    std::vector<uint32_t> m_CallStack;

    aml_register* m_ArgVars[7];
    aml_register* m_LocalVars[7];
};

}
