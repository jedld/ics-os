To build C applications within ICS-OS environment, tcc should be invoked in
the manner below (hello.c is the source file).

%/apps/tcc.exe -ohello.exe hello.c -B/tcc1 /tcc1/tccsdk.c /tcc1/crt1.c

