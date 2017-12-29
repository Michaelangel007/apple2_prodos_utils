SOURCES := $(wildcard *.cpp) $(wildcard *.h)
TARGETS = prodosfs
CFLAGS  = -O2

all:$(TARGETS)

.PHONY: install
install:
	cp prodosfs ~/bin/

.PHONY: clean
clean:
	@echo Sources: $(SOURCES)
	$(RM) $(TARGETS)

prodosfs: $(SOURCES)
	g++ $(CFLAGS) prodos.cpp -o $@

# ----------
test140: prodos
	echo "A" > foo1.txt
	echo "B" > foo2.txt
	echo "C" > foo3.txt
	echo -n "ABCDEFGHIJKLMNOPabcdefghijklmnop" >   32.txt
	cat  32.txt  32.txt  32.txt  32.txt        >  128.txt
	cat 128.txt 128.txt 128.txt 128.txt 32.txt > text.bin
	prodosfs test.dsk init -size=140 /TEST
	prodosfs test.dsk dir
	prodosfs test.dsk cp foo1.txt foo2.txt foo3.txt text.bin /
	prodosfs test.dsk dir

test800: prodos
	echo "A" > foo1.txt
	echo "B" > foo2.txt
	echo "C" > foo3.txt
	prodosfs test.dsk init -size=800 /TEST
	prodosfs test.dsk dir
	prodosfs test.dsk cp foo1.txt foo2.txt foo3.txt /
	prodosfs test.dsk dir

test32: prodos
	echo "A" > foo1.txt
	echo "B" > foo2.txt
	echo "C" > foo3.txt
	echo -n "ABCDEFGHIJKLMNOPabcdefghijklmnop" >   32.txt
	cat  32.txt  32.txt  32.txt  32.txt        >  128.txt
	cat 128.txt 128.txt 128.txt 128.txt 32.txt > text.txt
	prodosfs test.dsk init -size=32 /TEST
	prodosfs test.dsk dir
	prodosfs test.dsk cp foo1.txt foo2.txt foo3.txt text.txt /
	prodosfs test.dsk dir

