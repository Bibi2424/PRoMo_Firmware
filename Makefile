# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	- File automatically-generated by tool: [projectgenerator] version: [2.30.0] date: [Fri Oct 26 20:18:00 EDT 2018] 
# 	- Modify by Bibi 2018/11/04: Add flash, Use generic path for gcc and HAL, remove Src and Inc dir
# 	- Modify by Bibi 2021/05/22: Add support for multiple MCU, use wildcard to generate SOURCE list
# ------------------------------------------------

######################################
# target
######################################
TARGET = PRoMo

# MCU = STM32F411
MCU_TYPE = STM32F405

FW_VERSION_MAJOR = 0
FW_VERSION_MINOR = 2
FW_VERSION_REV 	 = 3

HW_TYPE = HW_PROMO_V0_1

######################################
# building variables
######################################
# debug build?
DEBUG = 1

#######################################
# paths
#######################################
# Build path
BUILD_DIR = _build

APP_DIR = application

BSP_DIR = bsp

LIB_DIR = lib

TOOLS_DIR = tools

# GCC toolchain path
# GCC_PATH = "E:/Program Files (x86)/GNU Tools Arm Embedded/9 2019-q4-major/bin/"
GCC_PATH = "E:/Program Files (x86)/GNU Arm Embedded Toolchain/10 2020-q4-major/bin"
# STM32 HAL librairies
STM32_LIBRAIRIES_PATH = STM32_HAL_min
STM32_LIBRAIRIES_SRC = $(STM32_LIBRAIRIES_PATH)/STM32F4xx_HAL_Driver/Src
STM32_LIBRAIRIES_INC = $(STM32_LIBRAIRIES_PATH)/STM32F4xx_HAL_Driver/Inc
# STL-Link Flasher Path
# STM32_FLASHER_EXEC := "C:/Program Files (x86)/STMicroelectronics/STM32 ST-LINK Utility/ST-LINK Utility/"
STM32_FLASHER_EXEC := "E:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/"

# If you need to select some libaries....
USED_LIBS += VL53L0X MPU6050
# USED_LIBS += test
APP_LIBS = $(foreach dir, $(USED_LIBS), $(LIB_DIR)/$(dir))

# Or just use them all
# APP_LIBS = $(sort $(dir $(wildcard $(LIB_DIR)/*/)))

SOURCE_FOLDER = $(APP_DIR) $(BSP_DIR) $(APP_LIBS)

######################################
# source
######################################
# C sources
vpath %.c $(SOURCE_FOLDER) $(STM32_LIBRAIRIES_SRC)

C_SOURCES = $(foreach dir, $(SOURCE_FOLDER), $(wildcard $(dir)/*.c))

# STM32_HAL
C_SOURCES += stm32f4xx_ll_gpio.c
C_SOURCES += stm32f4xx_ll_usart.c
C_SOURCES += stm32f4xx_ll_i2c.c
C_SOURCES += stm32f4xx_ll_tim.c
C_SOURCES += stm32f4xx_ll_dma.c
C_SOURCES += stm32f4xx_ll_rcc.c
C_SOURCES += stm32f4xx_ll_rtc.c
C_SOURCES += stm32f4xx_ll_pwr.c
C_SOURCES += stm32f4xx_ll_spi.c
C_SOURCES += stm32f4xx_ll_utils.c
C_SOURCES += stm32f4xx_ll_exti.c

# ASM sources
ifeq ($(MCU_TYPE), STM32F405) 
ASM_SOURCES =  startup_stm32f405xx.s
else ifeq ($(MCU_TYPE), STM32F411)
ASM_SOURCES =  startup_stm32f411xe.s
endif

# AS includes
AS_INCLUDES = 


INCS  = $(SOURCE_FOLDER)
INCS += $(STM32_LIBRAIRIES_INC)
INCS += $(STM32_LIBRAIRIES_PATH)/CMSIS/Device/ST/STM32F4xx/Include
INCS += $(STM32_LIBRAIRIES_PATH)/CMSIS/Include
# directories to be searched for header files
C_INCLUDES = $(addprefix -I,$(INCS))

#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
GDB = $(GCC_PATH)/$(PREFIX)gdb
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
GDB = $(PREFIX)gdb
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S
 
#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
USER_DEFS += USE_FULL_LL_DRIVER
USER_DEFS += HSE_VALUE=25000000
USER_DEFS += HSE_STARTUP_TIMEOUT=100
USER_DEFS += LSE_STARTUP_TIMEOUT=5000
USER_DEFS += LSE_VALUE=32768
USER_DEFS += EXTERNAL_CLOCK_VALUE=12288000
USER_DEFS += HSI_VALUE=16000000
USER_DEFS += LSI_VALUE=32000
USER_DEFS += VDD_VALUE=3300
USER_DEFS += PREFETCH_ENABLE=1
USER_DEFS += INSTRUCTION_CACHE_ENABLE=1
USER_DEFS += DATA_CACHE_ENABLE=1

USER_DEFS += TARGET=$(TARGET)
USER_DEFS += FW_VERSION_MAJOR=$(FW_VERSION_MAJOR)
USER_DEFS += FW_VERSION_MINOR=$(FW_VERSION_MINOR)
USER_DEFS += FW_VERSION_REV=$(FW_VERSION_REV)
USER_DEFS += HW_TYPE=$(HW_TYPE)

ifeq ($(MCU_TYPE), STM32F405)
	USER_DEFS += STM32F405xx
else ifeq ($(MCU_TYPE), STM32F411)
	USER_DEFS += STM32F411xE
endif

ifeq ($(DEBUG), 1)
	USER_DEFS += DEBUG_ENABLE=1
endif

C_DEFS = $(addprefix -D,$(USER_DEFS))


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -ffreestanding

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -ffreestanding

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
ifeq ($(MCU_TYPE), STM32F405)
	LDSCRIPT = STM32F405RGTx_FLASH.ld
else ifeq ($(MCU_TYPE), STM32F411)
	LDSCRIPT = STM32F411RETx_FLASH.ld
endif

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections


#######################################
# build the application
#######################################

# default action: build all
all: build

build: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	@echo "   [CC]" $<
	@$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	@echo "   [AS]" $<
	@$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	@echo "   [LD]" $@
	@$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	@echo "   [SIZE]"
	@$(SZ) $@
	@echo ""

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "   [CP]" $@
	@$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	@echo "   [CP]" $@
	@$(BIN) $< $@	
	
$(BUILD_DIR):
	mkdir $@		

#######################################
# clean up
#######################################
clean:
	@echo "    [RM]" $(BUILD_DIR)
	@-rm -fR $(BUILD_DIR)

#######################################
# flash
#######################################
# flash: all
flash: $(BUILD_DIR)/$(TARGET).hex | $(BUILD_DIR)
# 	@$(STM32_FLASHER_EXEC)ST-LINK_CLI.exe -c HOTPLUG -P $< 0x08000000 -Rst
	@$(STM32_FLASHER_EXEC)STM32_Programmer_CLI.exe -c port=SWD -d $< 0x08000000 -rst


reset:
# 	@$(STM32_FLASHER_EXEC)ST-LINK_CLI.exe -c -Rst
	$(STM32_FLASHER_EXEC)STM32_Programmer_CLI.exe -c port=SWD -rst

run-openocd:
	openocd -f $(TOOLS_DIR)/stlink.cfg  -f $(TOOLS_DIR)/stm32f4x.cfg

gdb:
	@echo \"$(GDB)\" $(BUILD_DIR)/PRoMo.elf -x $(TOOLS_DIR)/.gdbinit

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

.PHONY: all build clean flash reset run-openocd gdb

# *** EOF ***
