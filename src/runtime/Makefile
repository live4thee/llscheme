SOURCE := $(wildcard *.c)
OBJECT := $(SOURCE:.c=.o)

ifdef DEBUG
CFLAGS += -ggdb -DDEBUG
else
CFLAGS += -O2
endif

CPPFLAGS += -W -Wall -Werror -I../include

LLVMCC = llvm-gcc -emit-llvm
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
