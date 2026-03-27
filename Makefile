CC      = arm-none-eabi-gcc
LD      = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
QEMU    = qemu-system-arm
LIBGCC  = $(shell $(CC) -print-libgcc-file-name)

OUT = build

BEAGLE_DIR = os/beagle
QEMU_DIR   = os/qemu
P1_DIR     = user/P1
P2_DIR     = user/P2
LIB_DIR    = lib

CFLAGS_BEAGLE = -mcpu=cortex-a8 -marm -nostdlib -nostartfiles -ffreestanding -O0 -g -Wall -I$(LIB_DIR) -I$(BEAGLE_DIR)
CFLAGS_QEMU   = -mcpu=arm926ej-s -marm -nostdlib -nostartfiles -ffreestanding -O0 -g -Wall -I$(LIB_DIR) -I$(QEMU_DIR)

.PHONY: all beagle qemu qemu-build qemu-run clean

all: beagle

clean:
	rm -rf $(OUT)
	@echo "[OK] Limpio"

$(OUT):
	mkdir -p $(OUT)

# =========================================================
# BEAGLE
# =========================================================

BEAGLE_OS_OBJS = \
	$(OUT)/beagle_root.o \
	$(OUT)/beagle_uart.o \
	$(OUT)/beagle_io.o \
	$(OUT)/beagle_watchdog.o \
	$(OUT)/beagle_timer.o \
	$(OUT)/beagle_pcb.o \
	$(OUT)/beagle_scheduler.o \
	$(OUT)/beagle_main.o

beagle: $(OUT)/os_beagle.bin $(OUT)/p1.bin $(OUT)/p2.bin
	@echo "[OK] Build Beagle completado"

$(OUT)/beagle_root.o: $(BEAGLE_DIR)/root.s | $(OUT)
	$(CC) $(CFLAGS_BEAGLE) -c $< -o $@

$(OUT)/beagle_uart.o: $(BEAGLE_DIR)/uart.c $(BEAGLE_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_BEAGLE) -c $< -o $@

$(OUT)/beagle_io.o: $(BEAGLE_DIR)/io.c $(BEAGLE_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_BEAGLE) -c $< -o $@

$(OUT)/beagle_watchdog.o: $(BEAGLE_DIR)/watchdog.c $(BEAGLE_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_BEAGLE) -c $< -o $@

$(OUT)/beagle_timer.o: $(BEAGLE_DIR)/timer.c $(BEAGLE_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_BEAGLE) -c $< -o $@

$(OUT)/beagle_pcb.o: $(BEAGLE_DIR)/pcb.c $(BEAGLE_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_BEAGLE) -c $< -o $@

$(OUT)/beagle_scheduler.o: $(BEAGLE_DIR)/scheduler.c $(BEAGLE_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_BEAGLE) -c $< -o $@

$(OUT)/beagle_main.o: $(BEAGLE_DIR)/main.c $(BEAGLE_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_BEAGLE) -c $< -o $@

$(OUT)/os_beagle.elf: $(BEAGLE_OS_OBJS) $(BEAGLE_DIR)/os.ld
	$(LD) -T $(BEAGLE_DIR)/os.ld $(BEAGLE_OS_OBJS) -o $@

$(OUT)/os_beagle.bin: $(OUT)/os_beagle.elf
	$(OBJCOPY) -O binary $< $@

# -------- P1 / P2 para Beagle --------

$(OUT)/stdio_user.o: $(LIB_DIR)/stdio.c $(LIB_DIR)/stdio.h | $(OUT)
	$(CC) $(CFLAGS_BEAGLE) -c $< -o $@

$(OUT)/p1_main.o: $(P1_DIR)/main.c $(LIB_DIR)/stdio.h | $(OUT)
	$(CC) $(CFLAGS_BEAGLE) -c $< -o $@

$(OUT)/p2_main.o: $(P2_DIR)/main.c $(LIB_DIR)/stdio.h | $(OUT)
	$(CC) $(CFLAGS_BEAGLE) -c $< -o $@

$(OUT)/p1.elf: $(OUT)/p1_main.o $(OUT)/stdio_user.o $(OUT)/os_beagle.elf $(P1_DIR)/p1.ld
	$(LD) -T $(P1_DIR)/p1.ld \
		$(OUT)/p1_main.o $(OUT)/stdio_user.o \
		-R $(OUT)/os_beagle.elf \
		$(LIBGCC) \
		-o $@

$(OUT)/p1.bin: $(OUT)/p1.elf
	$(OBJCOPY) -O binary $< $@

$(OUT)/p2.elf: $(OUT)/p2_main.o $(OUT)/stdio_user.o $(OUT)/os_beagle.elf $(P2_DIR)/p2.ld
	$(LD) -T $(P2_DIR)/p2.ld \
		$(OUT)/p2_main.o $(OUT)/stdio_user.o \
		-R $(OUT)/os_beagle.elf \
		$(LIBGCC) \
		-o $@

$(OUT)/p2.bin: $(OUT)/p2.elf
	$(OBJCOPY) -O binary $< $@

# =========================================================
# QEMU
# =========================================================

QEMU_OS_OBJS = \
	$(OUT)/qemu_root.o \
	$(OUT)/qemu_uart.o \
	$(OUT)/qemu_io.o \
	$(OUT)/qemu_watchdog.o \
	$(OUT)/qemu_timer.o \
	$(OUT)/qemu_pcb.o \
	$(OUT)/qemu_scheduler.o \
	$(OUT)/qemu_main.o \
	$(OUT)/stdio_qemu.o \
	$(OUT)/qemu_p1.o \
	$(OUT)/qemu_p2.o

qemu-build: $(OUT)/os_qemu.elf
	@echo "[OK] Build QEMU completado"

qemu-run: $(OUT)/os_qemu.elf
	$(QEMU) -M versatilepb -m 128M -nographic -kernel $(OUT)/os_qemu.elf

qemu: qemu-run

$(OUT)/qemu_root.o: $(QEMU_DIR)/root.s | $(OUT)
	$(CC) $(CFLAGS_QEMU) -c $< -o $@

$(OUT)/qemu_uart.o: $(QEMU_DIR)/uart.c $(QEMU_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_QEMU) -c $< -o $@

$(OUT)/qemu_io.o: $(QEMU_DIR)/io.c $(QEMU_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_QEMU) -c $< -o $@

$(OUT)/qemu_watchdog.o: $(QEMU_DIR)/watchdog.c $(QEMU_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_QEMU) -c $< -o $@

$(OUT)/qemu_timer.o: $(QEMU_DIR)/timer.c $(QEMU_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_QEMU) -c $< -o $@

$(OUT)/qemu_pcb.o: $(QEMU_DIR)/pcb.c $(QEMU_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_QEMU) -c $< -o $@

$(OUT)/qemu_scheduler.o: $(QEMU_DIR)/scheduler.c $(QEMU_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_QEMU) -c $< -o $@

$(OUT)/qemu_main.o: $(QEMU_DIR)/main.c $(QEMU_DIR)/os.h | $(OUT)
	$(CC) $(CFLAGS_QEMU) -c $< -o $@

$(OUT)/stdio_qemu.o: $(LIB_DIR)/stdio.c $(LIB_DIR)/stdio.h | $(OUT)
	$(CC) $(CFLAGS_QEMU) -c $< -o $@

$(OUT)/qemu_p1.o: $(P1_DIR)/main.c $(LIB_DIR)/stdio.h | $(OUT)
	$(CC) $(CFLAGS_QEMU) -Dmain=p1_main -c $< -o $@

$(OUT)/qemu_p2.o: $(P2_DIR)/main.c $(LIB_DIR)/stdio.h | $(OUT)
	$(CC) $(CFLAGS_QEMU) -Dmain=p2_main -c $< -o $@

$(OUT)/os_qemu.elf: $(QEMU_OS_OBJS) $(QEMU_DIR)/os.ld
	$(LD) -T $(QEMU_DIR)/os.ld $(QEMU_OS_OBJS) $(LIBGCC) -o $@