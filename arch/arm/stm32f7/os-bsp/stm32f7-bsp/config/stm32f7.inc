include $(RTEMS_ROOT)/make/custom/default.cfg

RTEMS_CPU = arm
CPU_CFLAGS = -mthumb -mcpu=cortex-m7 -mfpu=fpv5-d16 -mfloat-abi=hard
CFLAGS_OPTIMIZE_V = -O0 -g
CFLAGS_OPTIMIZE_V += -ffunction-sections -fdata-sections

LDFLAGS = -Wl,--gc-sections