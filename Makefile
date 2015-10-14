

WORKSPACE_PATH ?= ../
LINKIT_ASSIST_SDK_PATH ?= $(WORKSPACE_PATH)LINKIT_ASSIST_SDK/
GCC_BIN ?= $(LINKIT_ASSIST_SDK_PATH)tools/gcc-arm-none-eabi-4_9-2014q4-20141203-win32/bin/

SYS_OBJECTS += $(WORKSPACE_PATH)common/gccmain.o
INCLUDE_PATHS += -I. -I$(LINKIT_ASSIST_SDK_PATH)include
LIBRARY_PATHS += -L$(LINKIT_ASSIST_SDK_PATH)lib
LIBRARIES += $(LINKIT_ASSIST_SDK_PATH)lib/LINKIT10/armgcc/percommon.a -lm
LINKER_SCRIPT = $(LINKIT_ASSIST_SDK_PATH)lib/LINKIT10/armgcc/scat.ld

###############################################################################
AS      = $(GCC_BIN)arm-none-eabi-as
CC      = $(GCC_BIN)arm-none-eabi-gcc
CPP     = $(GCC_BIN)arm-none-eabi-g++
LD      = $(GCC_BIN)arm-none-eabi-gcc
OBJCOPY = $(GCC_BIN)arm-none-eabi-objcopy
OBJDUMP = $(GCC_BIN)arm-none-eabi-objdump
SIZE    = $(GCC_BIN)arm-none-eabi-size
PACK    = $(WORKSPACE_PATH)tools/PackTag
PUSH    = $(LINKIT_ASSIST_SDK_PATH)tools/PushTool

CPU = -mcpu=arm7tdmi-s -mthumb -mlittle-endian
CC_FLAGS = $(CPU) -c -fvisibility=hidden -fpic -O2
CC_SYMBOLS = -D__HDK_LINKIT_ASSIST_2502__ -D__COMPILER_GCC__

LD_FLAGS = $(CPU) -O2 -Wl,--gc-sections --specs=nosys.specs -fpic -pie -Wl,-Map=$(PROJECT).map  -Wl,--entry=gcc_entry -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-unresolved-symbols
LD_SYS_LIBS =


all: $(PROJECT).elf size

clean:
	rm -f $(PROJECT).vxp $(PROJECT).bin $(PROJECT).elf $(PROJECT).hex $(PROJECT).map $(PROJECT).lst $(OBJECTS)

.s.o:
	$(AS) $(CPU) -o $@ $<

.c.o:
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

.cpp.o:
	$(CPP) $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu++98 $(INCLUDE_PATHS) -o $@ $<


$(PROJECT).elf: $(OBJECTS) $(SYS_OBJECTS)
	$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ -Wl,--start-group $^ $(LIBRARIES) $(LD_SYS_LIBS) -Wl,--end-group

$(PROJECT).bin: $(PROJECT).elf
	@$(OBJCOPY) -O binary $< $@

$(PROJECT).hex: $(PROJECT).elf
	@$(OBJCOPY) -O ihex $< $@

$(PROJECT).vxp: $(PROJECT).elf
	@$(OBJCOPY) --strip-debug $<
	@$(PACK) $< $@

$(PROJECT).lst: $(PROJECT).elf
	@$(OBJDUMP) -Sdh $< > $@

lst: $(PROJECT).lst

size: $(PROJECT).elf
	$(SIZE) $(PROJECT).elf

flash: $(PROJECT).vxp
	$(PUSH) -v -v -v -v -t arduino -clear -port $(PORT) -app $(PROJECT).vxp
