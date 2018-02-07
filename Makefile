BOARD=zero
-include Makefile.user
CC=arm-none-eabi-gcc
COMMON_FLAGS = -mthumb -mcpu=cortex-m0plus -Os -g
WFLAGS = \
-Wall -Wstrict-prototypes \
-Werror-implicit-function-declaration -Wpointer-arith -std=gnu99 \
-ffunction-sections -fdata-sections -Wchar-subscripts -Wcomment -Wformat=2 \
-Wimplicit-int -Wmain -Wparentheses -Wsequence-point -Wreturn-type -Wswitch \
-Wtrigraphs -Wunused -Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wundef \
-Wbad-function-cast -Wwrite-strings -Waggregate-return \
-Wformat -Wmissing-format-attribute \
-Wno-deprecated-declarations -Wpacked -Wredundant-decls -Wnested-externs \
-Wlong-long -Wunreachable-code -Wcast-align \
-Wno-missing-braces -Wno-overflow -Wno-shadow -Wno-attributes -Wno-packed -Wno-pointer-sign
CFLAGS = $(COMMON_FLAGS) \
-x c -c -pipe -nostdlib \
--param max-inline-insns-single=500 \
-fno-strict-aliasing -fdata-sections -ffunction-sections \
$(WFLAGS)

LDFLAGS= $(COMMON_FLAGS) \
-Wall -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--unresolved-symbols=report-all -Wl,--warn-common \
-Wl,--warn-section-align -Wl,--warn-unresolved-symbols \
-save-temps -nostartfiles \
--specs=nano.specs --specs=nosys.specs 
BUILD_PATH=build/$(BOARD)
INCLUDES = -I./inc -I./inc/preprocessor
INCLUDES += -I./asf/sam0/utils/cmsis/samd21/include -I./asf/thirdparty/CMSIS/Include -I./asf/sam0/utils/cmsis/samd21/source
INCLUDES += -I./asf/common -I./asf/common/utils -I./asf/sam0/utils/header_files -I./asf/sam0/utils -I./asf/common/utils/interrupt
INCLUDES += -I./asf/sam0/drivers/system/interrupt -I./asf/sam0/drivers/system/interrupt/system_interrupt_samd21
INCLUDES += -I./boards/$(BOARD)

COMMON_SRC = \
	src/flash.c \
	src/init.c \
	src/startup_samd21.c \
	src/usart_sam_ba.c \
	src/utils.c

SOURCES = $(COMMON_SRC) \
	src/cdc_enumerate.c \
	src/fat.c \
	src/main.c \
	src/msc.c \
	src/sam_ba_monitor.c \
	src/uart_driver.c \
	src/hid.c \

SELF_SOURCES = $(COMMON_SRC) \
	src/selfmain.c

OBJECTS = $(patsubst src/%.c,$(BUILD_PATH)/%.o,$(SOURCES))
SELF_OBJECTS = $(patsubst src/%.c,$(BUILD_PATH)/%.o,$(SELF_SOURCES)) $(BUILD_PATH)/selfdata.o

NAME=bootloader
EXECUTABLE=$(BUILD_PATH)/$(NAME).bin
SELF_EXECUTABLE=$(BUILD_PATH)/update-$(NAME).uf2

all: dirs $(EXECUTABLE) $(SELF_EXECUTABLE)

r: run
b: burn
l: logs

burn: all
	node scripts/dbgtool.js fuses
	node scripts/dbgtool.js $(BUILD_PATH)/$(NAME).bin

run: burn wait logs

wait:
	sleep 5

logs:
	node scripts/dbgtool.js $(BUILD_PATH)/$(NAME).map

selflogs:
	node scripts/dbgtool.js $(BUILD_PATH)/update-$(NAME).map

dirs:
	@echo "Building $(BOARD)"
	-@mkdir -p $(BUILD_PATH)

$(EXECUTABLE): $(OBJECTS) 
	$(CC) -L$(BUILD_PATH) $(LDFLAGS) \
		 -T./asf/sam0/utils/linker_scripts/samd21/gcc/samd21j18a_flash.ld \
		 -Wl,-Map,$(BUILD_PATH)/$(NAME).map -o $(BUILD_PATH)/$(NAME).elf $(OBJECTS)
	arm-none-eabi-objcopy -O binary $(BUILD_PATH)/$(NAME).elf $@
	@echo
	-@arm-none-eabi-size $(BUILD_PATH)/$(NAME).elf | awk '{ s=$$1+$$2; print } END { print ""; print "Space left: " (8192-s) }'
	@echo


$(SELF_EXECUTABLE): $(SELF_OBJECTS)
	$(CC) -L$(BUILD_PATH) $(LDFLAGS) \
		 -T./scripts/samd21j18a_self.ld \
		 -Wl,-Map,$(BUILD_PATH)/update-$(NAME).map -o $(BUILD_PATH)/update-$(NAME).elf $(SELF_OBJECTS)
	arm-none-eabi-objcopy -O binary $(BUILD_PATH)/update-$(NAME).elf $(BUILD_PATH)/update-$(NAME).bin
	node scripts/bin2uf2.js $(BUILD_PATH)/update-$(NAME).bin $@

$(BUILD_PATH)/%.o: src/%.c $(wildcard inc/*.h boards/*/*.h)
	@echo "$<"
	@$(CC) $(CFLAGS) $(BLD_EXTA_FLAGS) $(INCLUDES) $< -o $@

$(BUILD_PATH)/%.o: $(BUILD_PATH)/%.c
	@$(CC) $(CFLAGS) $(BLD_EXTA_FLAGS) $(INCLUDES) $< -o $@

$(BUILD_PATH)/selfdata.c: $(EXECUTABLE) scripts/gendata.js src/sketch.cpp
	node scripts/gendata.js $(BUILD_PATH) $(NAME).bin

clean:
	rm -rf build

gdb:
	arm-none-eabi-gdb $(BUILD_PATH)/$(NAME).elf

tui:
	arm-none-eabi-gdb -tui $(BUILD_PATH)/$(NAME).elf

%.asmdump: %.o
	arm-none-eabi-objdump -d $< > $@

applet0: $(BUILD_PATH)/flash.asmdump
	node scripts/genapplet.js $< flash_write

applet1: $(BUILD_PATH)/utils.asmdump
	node scripts/genapplet.js $< resetIntoApp

drop-board: all
	@echo "*** Copy files for $(BOARD)"
	mkdir -p build/drop
	rm -rf build/drop/$(BOARD)
	mkdir -p build/drop/$(BOARD)
	cp $(SELF_EXECUTABLE) build/drop/$(BOARD)/
	cp $(EXECUTABLE) build/drop/$(BOARD)/
	cp $(BUILD_PATH)/update-bootloader.ino build/drop/$(BOARD)/
	cp boards/$(BOARD)/board_config.h build/drop/$(BOARD)/

drop-pkg:
	mv build/drop build/uf2-samd21-$(VERSION)
	cp bin-README.md build/uf2-samd21-$(VERSION)/README.md
	cd build; 7z a uf2-samd21-$(VERSION).zip uf2-samd21-$(VERSION)
	rm -rf build/uf2-samd21-$(VERSION)

tag:
	"$(MAKE)" VERSION=`awk '/define UF2_VERSION_BASE/ { gsub(/"v?/, ""); print $$3 }' inc/uf2.h` do-tag

do-tag:
	git add inc/uf2.h
	git diff --exit-code
	git commit -m "v$(VERSION)"
	git tag "v$(VERSION)"
	git push
	git push --tag
	"$(MAKE)" drop

all-boards:
	for f in `cd boards; ls` ; do "$(MAKE)" BOARD=$$f drop-board ; done

drop: all-boards
	"$(MAKE)" VERSION=`awk '/define UF2_VERSION_BASE/ { gsub(/"v?/, ""); print $$3 }' inc/uf2.h` drop-pkg

