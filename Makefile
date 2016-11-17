CC=arm-none-eabi-gcc
COMMON_FLAGS = -mthumb -mcpu=cortex-m0plus -Os
CDEFINES = -D__SAMD21G18A__
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
-Wno-overflow -Wno-shadow
CFLAGS = $(COMMON_FLAGS) \
-x c -c -pipe -nostdlib \
--param max-inline-insns-single=500 \
-fno-strict-aliasing -fdata-sections -ffunction-sections -mlong-calls \
$(WFLAGS) $(CDEFINES)

LDFLAGS= $(COMMON_FLAGS) \
-Wall -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--unresolved-symbols=report-all -Wl,--warn-common \
-Wl,--warn-section-align -Wl,--warn-unresolved-symbols \
-save-temps  -T./asf/sam0/utils/linker_scripts/samd21/gcc/samd21j18a_flash.ld \
--specs=nano.specs --specs=nosys.specs 
BUILD_PATH=build
INCLUDES = -I./drivers -I./utils -I./utils/preprocessor -I./utils/interrupt  
INCLUDES += -I./asf/sam0/utils/cmsis/samd21/include -I./asf/thirdparty/CMSIS/Include -I./asf/sam0/utils/cmsis/samd21/source
INCLUDES += -I./asf/common
SOURCES = main.c sam_ba_monitor.c asf/sam0/utils/cmsis/samd21/source/gcc/startup_samd21.c usart_sam_ba.c drivers/cdc_enumerate.c drivers/uart_driver.c utils/interrupt/interrupt_sam_nvic.c 
OBJECTS = $(addprefix $(BUILD_PATH)/, $(SOURCES:.c=.o))

NAME=samd21_sam_ba
EXECUTABLE=$(BUILD_PATH)/$(NAME).bin

all: $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) -L$(BUILD_PATH) $(LDFLAGS) -Wl,-Map,$(BUILD_PATH)/$(NAME).map -o $(BUILD_PATH)/$(NAME).elf $(OBJECTS)
	arm-none-eabi-objcopy -O binary $(BUILD_PATH)/$(NAME).elf $@
	@arm-none-eabi-size $(BUILD_PATH)/$(NAME).elf

$(BUILD_PATH)/%.o: %.c
	-@mkdir -p $(@D)
	@echo "$<"
	@$(CC) $(CFLAGS) $(BLD_EXTA_FLAGS) $(INCLUDES) $< -o $@
	
clean:
	rm -rf $(EXECUTABLE) $(BUILD_PATH)



