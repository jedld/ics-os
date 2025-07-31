# Makefile for DexEdit - Enhanced Text Editor for ICS-OS
# Author: GitHub Copilot
# Date: 2025

NAME=dexedit

SRC=$(NAME).c
EXE=$(NAME).exe

CC=gcc
ICSOS_ROOT=../..
SDK=../../sdk
CFLAGS=-m32 -w -nostdlib -fno-builtin -static
LIBS=$(SDK)/tccsdk.c $(SDK)/libtcc1.c $(SDK)/crt1.c 

$(EXE): $(SRC)
	$(CC) $(CFLAGS) -o${EXE} $(SRC) $(LIBS)

install: $(EXE)
	cp $(EXE) $(ICSOS_ROOT)/apps

uninstall:
	rm $(ICSOS_ROOT)/apps/$(EXE)

clean:
	rm -f $(EXE)

help:
	@echo "Available targets:"
	@echo "  all      - Build the editor"
	@echo "  install  - Install to apps directory"
	@echo "  uninstall- Remove from apps directory"
	@echo "  clean    - Remove build files"
	@echo "  help     - Show this help"

.PHONY: clean install uninstall help
