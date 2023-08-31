# SUFFIXES+=.d
KERNEL_OUTPUT_FILE=build/kernel/lock2.elf

KERNELSOURCE=kernel/source
KERNELBUILD=build/kernel
KERNEL_EXTRAFLAGS=-I$(KERNELSOURCE)/klibc -I$(KERNELSOURCE) -Ifreecxxlib/src -mcmodel=kernel \
				 -D_LOCK2_SOURCE -fno-pie -fno-pic
KERNEL_CPPFLAGS=$(CFLAGS) $(KERNEL_EXTRAFLAGS)
KERNEL_CXXFLAGS=$(CXXFLAGS) $(KERNEL_EXTRAFLAGS)

LDFLAGS=-nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T kernel/linker.ld

ASFILES=arch/i386/cpu/sgms.s arch/i386/entry/crti.s arch/i386/cpu/idt.s \
		arch/i386/memcpy.s arch/i386/spinlock.s arch/i386/task_switch.s  \
		arch/i386/timer/tsc.s
ASSOURCES=$(addprefix kernel/source/,$(ASFILES))
ASBUILD=$(addprefix $(KERNELBUILD)/,$(addsuffix .o,$(ASFILES)))

CXXFILES=start_thunk.cpp terminal.cpp requests.cpp klibc/string.cpp    \
		arch/i386/cpu/gdt.cpp toshibatxl1.cpp arch/i386/cpu/idt.cpp     \
		arch/i386/cpu/idt_handlers.cpp alloc/physical.cpp 			     \
		arch/i386/paging/paging.cpp acpi/tables.cpp 				      \
		alloc/new.cpp arch/i386/apic.cpp arch/i386/timer/apic_timer.cpp    \
		dllogic/pe.cpp dllogic/load.cpp 							        \
		dllogic/ustar.cpp arch/i386/debug/stackframe.cpp  					 \
		arch/i386/debug/elf.cpp dllogic/api/dhelp.cpp pci/pci.cpp			  \
		klibc/abort.cpp 			   										   \
		acpi/acpi_handlers.cpp scheduler/scheduler.cpp scheduler/thread.cpp     \
		scheduler/semaphore.cpp arch/i386/timer/hpet.cpp 						 \
		arch/i386/thread_finish.cpp
ifeq ($(CXX),clang++)
CXXFILES+=arch/i386/itaniumabi_runtime/icxxabi.cpp
endif
CXXSOURCES=$(addprefix $(KERNELSOURCE)/,$(CXXFILES))
CXXBUILD=$(addprefix $(KERNELBUILD)/,$(addsuffix .o,$(CXXFILES)))
CXXHEADERS=$(patsubst %.cpp.o,%.d,$(CXXBUILD))
KERNEL_ALLBUILD=$(CXXBUILD) $(ASBUILD) $(CPPBUILD)

$(KERNEL_OUTPUT_FILE): $(ENABLED_DRIVERS) $(KERNEL_ALLBUILD) build/acpica/libacpica.a kernel/linker.ld
	$(LD) $(LDFLAGS) -L$(ACPICA_BUILD) $(ASBUILD) $(CXXBUILD) -lacpica -o $@

build/kernel/%.d: kernel/source/%.cpp
	mkdir -p build/$(dir $(subst source/,,$^))	
	$(CXX) $(KERNEL_CXXFLAGS) -MMD -MT '$(patsubst kernel/source/%.cpp,build/kernel/%.cpp.o,$<)' $< -MF $@ > /dev/null
	rm $(patsubst %.d,%.o,$(notdir $@))

build/kernel/%.cpp.o: kernel/source/%.cpp build/kernel/%.d
	mkdir -p build/kernel/$(dir $(subst kernel/source/,,$<))
	$(CXX) $(KERNEL_CXXFLAGS) -c $< -o $@

build/kernel/%.c.o: kernel/source/%.c
	mkdir -p build/kernel/$(dir $(subst kernel/source/,,$<))
	$(CC) $(KERNEL_CPPFLAGS) -c $< -o $@

build/kernel/%.s.o: kernel/source/%.s
	mkdir -p build/$(dir $(subst source/,,$^))
	$(AS) $(ASFLAGS) $^ -o $@

acpica:
	make -C $(KERNELSOURCE)/arch/i386/acpi/acpica 

-include $(CXXHEADERS)

create_build:
	mkdir -p build/kernel
