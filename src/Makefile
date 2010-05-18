SOURCE := $(wildcard *.cc)
OBJECT := $(SOURCE:.cc=.o)

ifdef DEBUG
CXXFLAGS += -ggdb -DDEBUG
else
CXXFLAGS += -O2
endif

CXXFLAGS += -W -Wall -Werror -I./include

TARGET := llscheme

all: $(TARGET)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECT)

$(TARGET): $(OBJECT)
	$(CXX) $(CXXFLAGS) $^ -o $@
