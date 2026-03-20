# make all   -> compila os.bin, p1.bin, p2.bin
# make clean -> borra build/

CC      = arm-none-eabi-gcc
LD      = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

OS_DIR  = os
P1_DIR  = user/P1
P2_DIR  = user/P2
OUT     = build

CFLAGS  = -mcpu=cortex-a8 -marm -nostdlib -nostdinc \
          -ffreestanding -O0 -g -Wall

.PHONY: all clean

all: $(OUT)/os.bin $(OUT)/p1.bin $(OUT)/p2.bin
	@echo ""
	@echo "======================================"
	@echo "  Cargar en BeagleBone:"
	@echo "  loady 0x82000000  -> os.bin"
	@echo "  loady 0x82100000  -> p1.bin"
	@echo "  loady 0x82200000  -> p2.bin"
	@echo "  go    0x82000000"
	@echo "======================================"

$(OUT):
	mkdir -p $(OUT)

# ── OS ─────────────────────────────────────────────────────────
$(OUT)/root.o: $(OS_DIR)/root.s | $(OUT)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/os.o: $(OS_DIR)/os.c | $(OUT)
	$(CC) $(CFLAGS) -I$(OS_DIR) -c $< -o $@

$(OUT)/os.elf: $(OUT)/root.o $(OUT)/os.o
	$(LD) -T $(OS_DIR)/os.ld $(OUT)/root.o $(OUT)/os.o -o $@

$(OUT)/os.bin: $(OUT)/os.elf
	$(OBJCOPY) -O binary $< $@
	@echo "[OK] os.bin  -> 0x82000000"

# P1
$(OUT)/p1_main.o: $(P1_DIR)/main.c | $(OUT)
	$(CC) $(CFLAGS) -I$(OS_DIR) -c $< -o $@

$(OUT)/p1.elf: $(OUT)/p1_main.o $(OUT)/os.o $(OUT)/root.o
	$(LD) -T $(P1_DIR)/p1.ld $^ -o $@

$(OUT)/p1.bin: $(OUT)/p1.elf
	$(OBJCOPY) -O binary $< $@
	@echo "[OK] p1.bin  -> 0x82100000"
# P2
$(OUT)/p2_main.o: $(P2_DIR)/main.c | $(OUT)
	$(CC) $(CFLAGS) -I$(OS_DIR) -c $< -o $@

$(OUT)/p2.elf: $(OUT)/p2_main.o $(OUT)/os.o $(OUT)/root.o
	$(LD) -T $(P2_DIR)/p2.ld $^ -o $@

$(OUT)/p2.bin: $(OUT)/p2.elf
	$(OBJCOPY) -O binary $< $@
	@echo "[OK] p2.bin  -> 0x82200000"

clean:
	rm -rf $(OUT)
	@echo "[OK] Limpio"