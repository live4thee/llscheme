SOURCE := $(wildcard *.cc)
OBJECT := $(SOURCE:.cc=.o)

ifdef DEBUG
CXXFLAGS += -ggdb -DDEBUG
else
CXXFLAGS += -O2
endif

CPPFLAGS += -W -Wall -Werror -I./include

CPPFLAGS += `llvm-config --cppflags`
LDFLAGS += `llvm-config --ldflags --libs core`

TARGET := llscheme

all: $(TARGET)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECT) *~ include/*~ \#* include/\#*

$(TARGET): $(OBJECT)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@
