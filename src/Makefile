SOURCE := $(wildcard *.cc)
OBJECT := $(SOURCE:.cc=.o)

ifdef DEBUG
CXXFLAGS += -ggdb -DDEBUG
else
CXXFLAGS += -O2
endif

CPPFLAGS += -W -Wall -I./include

CPPFLAGS += `llvm-config --cppflags`
LDFLAGS += `llvm-config --ldflags --libs core`

TARGET := llscheme.bin
LIBLSRT := runtime/liblsrt.a

all: $(TARGET) $(LIBLSRT)

.PHONY: clean FORCE
clean:
	$(RM) $(TARGET) $(OBJECT) *~ include/*~ \#* include/\#*
	$(MAKE) -C runtime clean

$(TARGET): $(OBJECT)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(LIBLSRT): FORCE
	$(MAKE) -C runtime
