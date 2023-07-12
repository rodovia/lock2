DRIVER_LD=x86_64-w64-mingw32-ld
DRIVER_CXX=$(CXX) -target x86_64-pc-windows-gnu
DRIVER_CXXFLAGS=$(CXXFLAGS) -Ikernel/source -Ifreecxxlib/src -fno-pie -fno-pic

ENABLED_DRIVERS=ata

$(foreach d,$(ENABLED_DRIVERS),$(eval include drivers/$(d)/recipes.mk))

drivers: $(DRIVER_OUTS)
