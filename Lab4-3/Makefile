CFLAGS += -g -Wall -Werror -m32 -O0 -fno-stack-protector -no-pie -fno-pic -I ./include
GCCVERSIONGTEQ8 := $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 8)

ifeq "$(GCCVERSIONGTEQ8)" "1"
	CFLAGS += -fcf-protection=none
endif

OBJDUMP = objdump
OBJS = ia32_disas.o

.PHONY = all objs clean

all: lab4_fib lab4_prime lab4_fibp

%.o: %.c
	$(CC) $(CFLAGS)  -c -o $@ $<
	$(OBJDUMP) -d $@ > $@.dis


lab4_fib: lab4.c ia32_callout.S fib.o ia32_disas.o
	$(CC) $(CFLAGS) -D__FIB__  $^ -static -Wl,-N -o $@
	$(OBJDUMP) -d $@ > $@.dis

lab4_prime: lab4.c ia32_callout.S isPrime.o ia32_disas.o
	$(CC) $(CFLAGS) -D__PRIME__  $^ -static -Wl,-N -o $@
	$(OBJDUMP) -d $@ > $@.dis

lab4_fibp: lab4.c ia32_callout.S fibp.o ia32_disas.o
	$(CC) $(CFLAGS) -D__FIBP__  $^ -static -Wl,-N -o $@

objs: $(OBJS)

clean:
	rm -f lab4_fib lab4_fibp lab4_prime *.o *.dis
