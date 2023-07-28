KERNEL=build/kernel/lock2.elf
ISO_OUTPUT=lock2.iso

AS=nasm
CC=clang
CXX=clang++
LD=ld.lld

ASFLAGS=-g -F dwarf -f elf64
CFLAGS=-Os -g -mgeneral-regs-only -c -ffreestanding -fno-stack-protector -nostdlib \
	-fno-stack-check -fno-lto -fno-stack-protector -m64 \
	-march=x86-64 -mabi=sysv -mno-80387 -mno-red-zone
CXXFLAGS=$(CFLAGS) -std=c++17 -pipe -Wall -Wextra \
	-fno-exceptions -fno-rtti  -fno-threadsafe-statics -Weffc++

ifeq ($(CXX),g++)
# GCC warning about this inside limine.h.
# Nothing we can do about.
CXXFLAGS+=-Wno-missing-field-initializers
endif
ifeq ($(CXX),clang++)
CXXFLAGS+=-Wno-deprecated-register
endif

MODULES=kernel drivers kernel/source/acpi/acpica
all: iso/$(ISO_OUTPUT)

$(foreach m,$(MODULES),$(eval include $(m)/recipes.mk))
-include aux/initramfs.mk

iso/root:
	mkdir -p iso/root

iso/$(ISO_OUTPUT): iso/root ramfs $(KERNEL) FORCE
	cp -v $(KERNEL) limine/limine-bios.sys \
	limine/limine-bios-cd.bin limine/limine-uefi-cd.bin limine.cfg iso/root/
	mkdir -p iso/root/EFI/BOOT
	cp -v limine/BOOT*.EFI iso/root/EFI/BOOT
	xorriso -as mkisofs -b limine-bios-cd.bin \
			-no-emul-boot -boot-load-size 4 -boot-info-table \
			--efi-boot limine-uefi-cd.bin \
			-efi-boot-part --efi-boot-image --protective-msdos-label \
			iso/root/ -o iso/$(ISO_OUTPUT)
	./limine/limine bios-install iso/$(ISO_OUTPUT)

clean:
	rm -rf build/kernel
	mkdir -p build/kernel

# For some reason, Clang likes to dump the project root
# with .o files from the kernel.
clean_dot_o: FORCE
	

FORCE: ;

