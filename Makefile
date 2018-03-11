
CROSS_COMPILE ?= arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
LIBOPENCM3 ?= ./libopencm3
#GIT_VERSION := $(shell git describe --abbrev=8 --dirty --always --tags)
#	-DVERSION=\"$(GIT_VERSION)\" -flto

# Config bits
FLASH_SIZE = 128
FLASH_BASE_ADDR = 0x08000000
APP_ADDRESS = 0x08001800

APP_OFFSET = $(shell echo $$(($(APP_ADDRESS) - $(FLASH_BASE_ADDR))))

# -pedantic -Wall -Werror
CFLAGS = -Os -std=gnu99 -Istm32/include \
	-ffunction-sections -fdata-sections -Wno-overlength-strings \
	-mcpu=cortex-m3 -mthumb -DSTM32F1 -fno-builtin-memcpy  \
	-I$(LIBOPENCM3)/include -DAPP_ADDRESS=$(APP_ADDRESS)   \
	-ggdb3 -Iinclude --param case-values-threshold=2

LDFLAGS = -lopencm3_stm32f1 \
	-ffunction-sections -fdata-sections \
	-Wl,-Tstm32f103.ld -nostartfiles -lc -lnosys \
	-mthumb -mcpu=cortex-m3 -L$(LIBOPENCM3)/lib/ -Wl,-gc-sections

# Benchmark image
all:	benchmark-img.bin

TESTSOBJS=tests/md5-test.o tests/sha1-test.o tests/sha256-test.o tests/chacha20-test.o tests/aes-test.o
LIBOBJS=src/libc-alt.o src/md5.o src/sha1.o src/sha256.o src/hash_common.o src/aes.o src/chacha20.o src/random.o

benchmark-img.elf: benchmark.o $(LIBOBJS) $(TESTSOBJS) | $(LIBOPENCM3)/lib/libopencm3_stm32f1.a
	$(CC) $^ -o $@ $(LDFLAGS) -Wl,-Ttext=$(APP_ADDRESS) -Wl,-Map,uusb.map

$(LIBOPENCM3)/lib/libopencm3_stm32f1.a:
	$(MAKE) -C $(LIBOPENCM3) TARGETS=stm32/f1

%.bin: %.elf
	$(OBJCOPY) -O binary $^ $@

%.o: %.c flash_config.h
	$(CC) -c $< -o $@ $(CFLAGS)

cli-tool:
	g++ -o cli-tool cli-tool.cc -lusb-1.0 -ggdb


# Testing

COVERAGE_FLAGS=-ftest-coverage -fprofile-arcs
MEMORY_CHECK_FLAGS=-fsanitize=address -lasan
FLAGS=-ggdb -O0 $(COVERAGE_FLAGS) $(MEMORY_CHECK_FLAGS)

test:
	gcc -o alltests.bin tests/test_main.c tests/md5-test.c tests/sha1-test.c \
		tests/sha256-test.c tests/aes-test.c tests/chacha20-test.c \
		src/md5.c src/sha1.c src/sha256.c src/hash_common.c src/chacha20.c src/aes.c \
		-Iinclude/ $(FLAGS) -Ddbgprintf=printf
	./alltests.bin
	lcov -c -d . -o total.info
	rm -rf coverage_report/
	genhtml -o coverage_report/ total.info

clean:
	rm -f *.gcno *.gcov *.gcda *.bin *.html *.info *.o src/*.o tests/*.o *.elf


