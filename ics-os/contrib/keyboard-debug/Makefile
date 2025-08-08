all: keyboard-debug.exe raw-test.exe simple-editor.exe arrow-test.exe backspace-test.exe

keyboard-debug.exe: keyboard-debug.c
	gcc -m32 -w -nostdlib -fno-builtin -static -okeyboard-debug.exe keyboard-debug.c ../../sdk/tccsdk.c ../../sdk/libtcc1.c ../../sdk/crt1.c

raw-test.exe: raw-test.c
	gcc -m32 -w -nostdlib -fno-builtin -static -oraw-test.exe raw-test.c ../../sdk/tccsdk.c ../../sdk/libtcc1.c ../../sdk/crt1.c

simple-editor.exe: simple-editor.c
	gcc -m32 -w -nostdlib -fno-builtin -static -osimple-editor.exe simple-editor.c ../../sdk/tccsdk.c ../../sdk/libtcc1.c ../../sdk/crt1.c

arrow-test.exe: arrow-test.c
	gcc -m32 -w -nostdlib -fno-builtin -static -oarrow-test.exe arrow-test.c ../../sdk/tccsdk.c ../../sdk/libtcc1.c ../../sdk/crt1.c

backspace-test.exe: backspace-test.c
	gcc -m32 -w -nostdlib -fno-builtin -static -obackspace-test.exe backspace-test.c ../../sdk/tccsdk.c ../../sdk/libtcc1.c ../../sdk/crt1.c

clean:
	rm -f keyboard-debug.exe raw-test.exe simple-editor.exe arrow-test.exe backspace-test.exe

install: keyboard-debug.exe
	cp keyboard-debug.exe ../../apps
