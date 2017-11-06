SOURCES := $(wildcard *.cpp) $(wildcard *.h)
TARGETS = prodos
CFLAGS  = -O2

all:$(TARGETS)

.PHONY: clean
clean:
	@echo Sources: $(SOURCES)
	$(RM) $(TARGETS)

prodos: $(SOURCES)
	g++ $(CFLAGS) prodos.cpp -o $@

# ----------
test140: prodos
	echo "A" > foo1.txt
	echo "B" > foo2.txt
	echo "C" > foo3.txt
	echo -n "ABCDEFGHIJKLMNOPabcdefghijklmnop" >   32.txt
	cat  32.txt  32.txt  32.txt  32.txt        >  128.txt
	cat 128.txt 128.txt 128.txt 128.txt 32.txt > text.bin
	prodos test.dsk init -size=140 /TEST
	prodos test.dsk dir
	prodos test.dsk cp foo1.txt foo2.txt foo3.txt text.bin /
	prodos test.dsk dir

test800: prodos
	echo "A" > foo1.txt
	echo "B" > foo2.txt
	echo "C" > foo3.txt
	prodos test.dsk init -size=800 /TEST
	prodos test.dsk dir
	prodos test.dsk cp foo1.txt foo2.txt foo3.txt /
	prodos test.dsk dir

test32: prodos
	echo "A" > foo1.txt
	echo "B" > foo2.txt
	echo "C" > foo3.txt
	echo -n "ABCDEFGHIJKLMNOPabcdefghijklmnop" >   32.txt
	cat  32.txt  32.txt  32.txt  32.txt        >  128.txt
	cat 128.txt 128.txt 128.txt 128.txt 32.txt > text.txt
	prodos test.dsk init -size=32 /TEST
	prodos test.dsk dir
	prodos test.dsk cp foo1.txt foo2.txt foo3.txt text.txt /
	prodos test.dsk dir

