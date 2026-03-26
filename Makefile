# Makefile — 001-Multiprogramming
# Target: BeagleBone Black (AM335x Cortex-A8) y QEMU (VersatilePB)

CC      = arm-none-eabi-gcc
LD      = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy
QEMU    = qemu-system-arm

# ── NUEVAS RUTAS ACTUALIZADAS ──
OS_DIR  = os/beagle
QEMU_OS_DIR = os/qemu
P1_DIR  = user/P1
P2_DIR  = user/P2
OUT     = build

CFLAGS  = -mcpu=cortex-a8 -marm -nostdlib -nostdinc \
          -ffreestanding -O0 -g -Wall

.PHONY: all clean qemu

# =====================================================================
# ── REGLAS PARA BEAGLEBONE (El código de tu compañero) ───────────────
# =====================================================================
all: $(OUT)/os.bin $(OUT)/p1.bin $(OUT)/p2.bin
	@echo ""
	@echo "======================================"
	@echo "  Build exitoso! Cargar en BeagleBone:"
	@echo "  loady 0x82100000  -> p1.bin"
	@echo "  loady 0x82200000  -> p2.bin"
	@echo "  loady 0x82000000  -> os.bin"
	@echo "  go    0x82000000"
	@echo "======================================"

$(OUT):
	mkdir -p $(OUT)

# OS Beagle
$(OUT)/root.o: $(OS_DIR)/root.s | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/os.o: $(OS_DIR)/os.c | $(OUT)
	$(CC) $(CFLAGS) -I$(OS_DIR) -c $< -o $@

$(OUT)/os.elf: $(OUT)/root.o $(OUT)/os.o
	$(LD) -T $(OS_DIR)/os.ld $(OUT)/root.o $(OUT)/os.o -o $@

$(OUT)/os.bin: $(OUT)/os.elf
	$(OBJCOPY) -O binary $< $@
	@echo "[OK] os.bin  -> 0x82000000"

# User Process 1
$(OUT)/p1_main.o: $(P1_DIR)/main.c | $(OUT)
	$(CC) $(CFLAGS) -I$(OS_DIR) -c $< -o $@

$(OUT)/p1.elf: $(OUT)/p1_main.o
	$(LD) -T $(P1_DIR)/p1.ld $^ -o $@

$(OUT)/p1.bin: $(OUT)/p1.elf
	$(OBJCOPY) -O binary $< $@
	@echo "[OK] p1.bin  -> 0x82100000"

# User Process 2
$(OUT)/p2_main.o: $(P2_DIR)/main.c | $(OUT)
	$(CC) $(CFLAGS) -I$(OS_DIR) -c $< -o $@

$(OUT)/p2.elf: $(OUT)/p2_main.o
	$(LD) -T $(P2_DIR)/p2.ld $^ -o $@

$(OUT)/p2.bin: $(OUT)/p2.elf
	$(OBJCOPY) -O binary $< $@
	@echo "[OK] p2.bin  -> 0x82200000"

# =====================================================================
# ── SECCIÓN EXCLUSIVA PARA QEMU (MÁQUINA VERSATILEPB) ────────────────
# =====================================================================

QEMU_OS_DIR = os/qemu
LIB_DIR = lib
QEMU_EMU    = qemu-system-arm

QEMU_CFLAGS = -mcpu=arm926ej-s -marm -nostdlib -nostdinc -ffreestanding -O0 -g -Wall -I$(LIB_DIR) -I$(QEMU_OS_DIR)

# 1. Compilar root.s
$(OUT)/root_qemu.o: $(QEMU_OS_DIR)/root.s | $(OUT)
	$(CC) $(QEMU_CFLAGS) -c $< -o $@

# 2. Compilar os.c y stdio.c
$(OUT)/os_qemu.o: $(QEMU_OS_DIR)/os.c | $(OUT)
	$(CC) $(QEMU_CFLAGS) -c $< -o $@

$(OUT)/stdio_qemu.o: $(LIB_DIR)/stdio.c | $(OUT)
	$(CC) $(QEMU_CFLAGS) -c $< -o $@

# 3. NUEVO: Compilar Proceso 1 y Proceso 2 para QEMU
$(OUT)/p1_qemu.o: $(P1_DIR)/main.c | $(OUT)
	$(CC) $(QEMU_CFLAGS) -c $< -o $@

$(OUT)/p2_qemu.o: $(P2_DIR)/main.c | $(OUT)
	$(CC) $(QEMU_CFLAGS) -c $< -o $@

# 4. Unir todo en el ejecutable final
$(OUT)/os_qemu.elf: $(OUT)/root_qemu.o $(OUT)/os_qemu.o $(OUT)/stdio_qemu.o $(OUT)/p1_qemu.o $(OUT)/p2_qemu.o
	$(LD) -T $(QEMU_OS_DIR)/os.ld $^ /usr/lib/gcc/arm-none-eabi/13.2.1/libgcc.a -o $@

qemu: $(OUT)/os_qemu.elf
	@echo ""
	@echo "======================================"
	@echo "  Iniciando emulador QEMU"
	@echo "  (Para salir: Ctrl+A y luego X)"
	@echo "======================================"
	$(QEMU_EMU) -M versatilepb -m 128M -nographic -kernel $(OUT)/os_qemu.elf

# ── Clean ────────────────────────────────────────────────────
clean:
	rm -rf $(OUT)
	@echo "[OK] Limpio"