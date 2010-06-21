COMMON_CFLAGS := -W -Wall

# check the existence of bdw-gc
GC_LIB := $(shell pkg-config bdw-gc --libs)
ifneq ($(GC_LIB), )
	COMMON_CFLAGS += -DBDWGC
endif

ifdef DEBUG
COMMON_CFLAGS += -ggdb -DDEBUG
else
COMMON_CFLAGS += -O2
endif

CXX := g++
CC  := gcc

CXXFLAGS += $(COMMON_CFLAGS)
CFLAGS   += $(COMMON_CFLAGS)
