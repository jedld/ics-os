# Compiler and flags
CC = gcc
CFLAGS = -m32 -w -nostdlib -fno-builtin -static

# SDK files
SDK_DIR = ../../sdk
SDK_FILES = $(SDK_DIR)/tccsdk.c $(SDK_DIR)/libtcc1.c $(SDK_DIR)/crt1.c

# Target
TARGET = keyboard-demo.exe

# Source files
SRC = keyboard-demo.c

# Build rule
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o$(TARGET) $(SRC) $(SDK_FILES)

# Clean rule
clean:
	rm -f $(TARGET)

# Install rule
install: $(TARGET)
	cp $(TARGET) ../../apps

.PHONY: clean install
