# main targets:
#   all             build everything
#   clean           clean everything but binaries
#   distclean       clean everything


#########################
# configuration section #
#########################
#OS  operating system (UNIX OR WINDOWS)
#default: detected automatically
#e.g. OS=UNIX

#GCCEXTRAFLAGS  additional flags
GCCEXTRAFLAGS=-Wall
#GCCEXTRAFLAGS=-Wall -L/mingw64/lib -I/mingw64/include

###############################
# DO NOT CHANGE THE FOLLOWING #
###############################
.PHONY: all clean distclean

UNIX_TARGETS=ucecho memfifo libztex.so
WIN_TARGETS=ucecho.exe memfifo.exe

GCC=gcc -g -std=gnu99  $(GCCEXTRAFLAGS) 

ifeq ($(OS),WINDOWS_NT)
OS=WINDOWS
endif

ifneq ($(OS),WINDOWS)
ifneq ($(OS),UNIX)
ifeq ($(WINDIR),)
OS=UNIX
else
OS=WINDOWS
endif
endif
endif

ifeq ($(OS),WINDOWS)
TARGETS=$(WIN_TARGETS)
LIBS=-Wl,-Bstatic -lusb-1.0
else
TARGETS=$(UNIX_TARGETS)
LIBS=-lusb-1.0
endif

all: $(TARGETS)

libztex.so: ztex.c ztex.h
	$(GCC)-fPIC -shared -o $@ $< 

ztex.o: ztex.c ztex.h
	$(GCC) -c -o $@ $< $(LIBS)
	

%: %.c ztex.o
	$(GCC) ztex.o $< $(LIBS) -o $@ 

%.exe : %.c ztex.o
	$(GCC) ztex.o $< $(LIBS) -o $@ 

clean:
	rm -f *.o *.old *.bak *~

distclean: clean
	rm -f $(TARGETS)
