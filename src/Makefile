include ./common.mk

SOURCE := $(wildcard *.cc)
OBJECT := $(SOURCE:.cc=.o)

CPPFLAGS += -I./include `llvm-config --cppflags`
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
