SOURCE := $(wildcard *.cc)
OBJECT := $(SOURCE:.cc=.o)

CXXFLAGS += -W -Wall -Werror -I./include

TARGET := mylexer

all: $(TARGET)

.PHONY: clean
clean:
	$(RM) $(TARGET) $(OBJECT)

$(TARGET): $(OBJECT)
	$(CXX) $(CXXFLAGS) $^ -o $@
