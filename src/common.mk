COMMON_CFLAGS := -W -Wall

ifdef DEBUG
COMMON_CFLAGS += -ggdb -DDEBUG
endif

CXX := clang++
CC  := clang

CPPFLAGS += $(COMMON_CFLAGS)
CFLAGS   += $(COMMON_CFLAGS)
