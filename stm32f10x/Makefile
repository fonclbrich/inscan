PROJECT ?= libinscanHW

# path to STM32F103 standard peripheral library
STD_PERIPH_LIBS ?= STM32F10x_StdPeriph_Lib_V3.5.0/Libraries

PERIPH_SOURCES = $(STD_PERIPH_LIBS)/CMSIS/CM3/DeviceSupport/ST/STM32F10x/system_stm32f10x.c
#PERIPH_SOURCES += $(STD_PERIPH_LIBS)/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c
#PERIPH_SOURCES += $(STD_PERIPH_LIBS)/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c
#PERIPH_SOURCES += $(STD_PERIPH_LIBS)/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c
#PERIPH_SOURCES += $(STD_PERIPH_LIBS)/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c
#PERIPH_SOURCES += $(STD_PERIPH_LIBS)/STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.c

CSOURCES  = $(PERIPH_SOURCES)
CSOURCES += blinkLED.c
CSOURCES += usartdebug.c
CSOURCES += clock.c
CSOURCES += timer.c
CSOURCES += usb.c

OBJS = $(CSOURCES:.c=.o) $(ASOURCES:.s=.o)

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
AR = arm-none-eabi-ar
AS = arm-none-eabi-as

CFLAGS  = -O2 -Wall
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m3 -mthumb-interwork
CFLAGS += -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DPRINTUSBSETUP -DDEBUG_USB #-DPRINTCLKINFO
CFLAGS += -D STM32F10X_USART1_INCOMING=0x100
CFLAGS += -Wl,--gc-sections

CFLAGS += -I$(STD_PERIPH_LIBS)/CMSIS/CM3/DeviceSupport/ST/STM32F10x/
CFLAGS += -I$(STD_PERIPH_LIBS)/CMSIS/CM3/CoreSupport
CFLAGS += -I$(STD_PERIPH_LIBS)/STM32F10x_StdPeriph_Driver/inc
CFLAGS += -I../include -I..

AFLAGS =  -mcpu=cortex-m3 -mthumb

all: $(PROJECT).a
	
$(PROJECT).a: ${OBJS} startup.o
	$(AR) vrcs $@ ${OBJS}

startup.o : $(STD_PERIPH_LIBS)/CMSIS/CM3/DeviceSupport/ST/STM32F10x/startup/TrueSTUDIO/startup_stm32f10x_md.s
	$(AS) $(AFLAGS) -c $< -o $@ 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@ 
	
%.o: %.s
	$(AS) $(AFLAGS) -c $< -o $@ 
	
clean:
	rm -rf $(OBJS) *.o *.a 
