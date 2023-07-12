ATA_SOURCEBASE=drivers/ata/
ATA_BUILDBASE=build/drivers/ata

ATA_CXXFILES=atapi.cpp ide.cpp intr.cpp
ATA_CXXSOURCES=$(addprefix $(ATA_SOURCEBASE)/,$(ATA_CXXFILES))
ATA_CXXBUILD=$(addprefix $(ATA_BUILDBASE)/,$(addsuffix .o,$(ATA_CXXFILES)))

ATA_OUTPUT=$(ATA_BUILDBASE)/atadrv.dll

DRIVER_OUTS+=$(ATA_OUTPUT)
RAMFS_FILES+=$(ATA_OUTPUT)

ata: $(ATA_OUTPUT)

ata_clean:
	rm -rf $(ATA_BUILDBASE)

$(ATA_OUTPUT): $(ATA_CXXBUILD)
	$(DRIVER_LD) -shared $(ATA_CXXBUILD) -o $@ 

$(ATA_BUILDBASE)/%.d: $(ATA_SOURCEBASE)/source/%.cpp
	mkdir -p build/$(dir $(subst source/,,$^))	
	$(DRIVER_CXX) $(DRIVER_CXXFLAGS) -MMD -MT '$(patsubst $(ATA_SOURCEBASE)/source/%.cpp,$(ATA_BUILDBASE)/%.cpp.o,$<)' $< -MF $@ > /dev/null
	rm $(patsubst %.d,%.o,$(notdir $@))

$(ATA_BUILDBASE)/%.cpp.o: $(ATA_SOURCEBASE)/source/%.cpp $(ATA_BUILDBASE)/%.d
	mkdir -p build/kernel/$(dir $(subst kernel/source/,,$<))
	$(DRIVER_CXX) $(DRIVER_CXXFLAGS) -mcmodel=large -c $< -o $@
