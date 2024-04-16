CFLAGS = -ffreestanding -MMD -mno-red-zone -std=c11 -target x86_64-unknown-windows
CFLAGS += $(shell pkg-config --cflags gnu-efi)
LDFLAGS = -flavor link -subsystem:efi_application -entry:efi_main
#LDFLAGS += $(shell pkg-config --libs gnu-efi)

TARGET = simple-boot.efi
SRCS := main.c

all: $(TARGET)

.c.o:
	clang $(CFLAGS) -c $< -o $@

$(TARGET): main.o
	lld $(LDFLAGS) $< -out:$@

-include $(SRCS:.c=.d)

.PHONY: clean test

clean:
	@rm -f $(TARGET) *.d *.o
	@rm -fr test

test: $(TARGET)
	rm -fr test
	mkdir -p test/efi/boot
	cp $(TARGET) test/efi/boot/bootx64.efi
	cp -f /usr/share/ovmf/OVMF.fd test/
	qemu-system-x86_64 -drive if=pflash,format=raw,file=test/OVMF.fd \
		-drive format=raw,file=fat:rw:test -net none -nographic
