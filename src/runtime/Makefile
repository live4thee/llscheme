include ../common.mk

SOURCE := $(wildcard *.c)
OBJECT := $(SOURCE:.c=.o)

CFLAGS += -I../include

LLVMCC = clang -emit-llvm
LLVMAR = llvm-ar

TARGET := liblsrt.a

all: $(TARGET)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECT) *.ll *.bc *~ ../include/runtime/*~ \#* ../include/runtime/\#*

.c.o:
	$(LLVMCC) $(CFLAGS) $(CPPFLAGS) -c $<

$(TARGET): $(OBJECT)
	$(LLVMAR) rcs $@ $^
