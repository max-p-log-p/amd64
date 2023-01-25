CFLAGS += -g -O0 -Wall -Werror -fno-stack-protector -no-pie -fno-pic  -Wno-int-to-pointer-cast -I ./include
GCCVERSIONGTEQ8 := $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 8)

ifeq "$(GCCVERSIONGTEQ8)" "1"
	CFLAGS += -fcf-protection=none
endif

OBJDUMP = objdump
OBJS = amd64_disas.o

.PHONY = all objs clean

all: lab4_fib 

%.o: %.c
	$(CC) $(CFLAGS)  -c -o $@ $<
	$(OBJDUMP) -d $@ > $@.dis


lab4_fib: lab4.c amd64_callout.S fib.o amd64_disas.o
	$(CC) $(CFLAGS) -D__FIB__  $^ -static -Wl,-N -o $@
	$(OBJDUMP) -d $@ > $@.dis


objs: $(OBJS)

clean:
	rm -f lab4_fib *.o *.dis
