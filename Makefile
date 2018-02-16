# path to STM32F103 standard peripheral library
STD_PERIPH_LIBS ?= stm32f10x/STM32F10x_StdPeriph_Lib_V3.5.0
ARCH = stm32f10x

# list of source files
SOURCES  = inscan.c usb.c # scsi.c usbconsts.c

STARTUP = $(ARCH)/startup.o
LINKUP = $(ARCH)/linkup.ld

LDFLAGS = $(STARTUP) -T$(LINKUP) -linscanHW -L$(ARCH)
# name for output binary files
PROJECT ?= inscan.0.8

# compiler, objcopy (should be in PATH)
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

# path to st-flash (or should be specified in PATH)
ST_FLASH ?= st-flash

# specify compiler flags
CFLAGS  = -O2 -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3 -mthumb-interwork
CFLAGS += -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DDEBUG_USB
CFLAGS += -Wl,--gc-sections
CFLAGS += -I$(ARCH) -Iinclude
#CFLAGS += -I$(STD_PERIPH_LIBS)/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/
#CFLAGS += -I$(STD_PERIPH_LIBS)/Libraries/CMSIS/CM3/CoreSupport
#CFLAGS += -I$(STD_PERIPH_LIBS)/Libraries/STM32F10x_StdPeriph_Driver/inc

OBJS = $(SOURCES:.c=.o)

all: $(PROJECT).elf

# compile
$(PROJECT).elf: $(SOURCES) libinscanHW
	$(CC) $(CFLAGS) $(SOURCES) -o $@ $(LDFLAGS)
	$(OBJCOPY) -O ihex $(PROJECT).elf $(PROJECT).hex
	$(OBJCOPY) -O binary $(PROJECT).elf $(PROJECT).bin

.PHONY: libinscanHW
libinscanHW:
	make -C stm32f10x
	
.PHONY: clean

clean:
	rm -rf *.o *.elf *.hex *.bin
	make -C stm32f10x clean
