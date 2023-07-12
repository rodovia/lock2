# A separate recipes file is used because
# I cannot bare oskrnl having 500kb of ACPICA code bloat
# when I'm only using 2 functions of it

ACPICA_OUTPUT=libacpica.a
ACPICA_SOURCE=$(KERNELSOURCE)/acpi/acpica
ACPICA_BUILD=build/acpica
ACPICA_CFLAGS=$(CFLAGS) -D_LOCK2_SOURCE -mcmodel=kernel -fPIC
ACPICA_CSOURCES=$(wildcard $(ACPICA_SOURCE)/*.c)
ACPICA_CFILES=$(notdir $(ACPICA_CSOURCES))
ACPICA_CBUILD:=$(addprefix $(ACPICA_BUILD)/,$(addsuffix .o,$(ACPICA_CFILES)))

$(ACPICA_BUILD)/%.c.o: $(ACPICA_SOURCE)/%.c
	mkdir -p $(ACPICA_BUILD)
	$(CC) $(ACPICA_CFLAGS) -c $< -o $@

$(ACPICA_BUILD)/$(ACPICA_OUTPUT): $(ACPICA_CBUILD)
	$(AR) rcs $@ $^

acpica_clean:
	rm -rf $(ACPICA_BUILD)
