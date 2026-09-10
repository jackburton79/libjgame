.DEFAULT_GOAL := all

CXX := g++
AR := ar
RM := rm -rf

LIBS := -lz $(shell sdl2-config --libs)
GAMELIB := libjgame.a

OUTDIR := lib
OBJDIR := obj

SUBDIRS := \
	audio \
	graphics \
	shell \
	streams \
	support \
	timers

SRCS := $(wildcard *.cpp $(foreach d,$(SUBDIRS),$(d)/*.cpp))
OBJS := $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o))
DEPS := $(OBJS:.o=.d)

CPPFLAGS := -I. $(addprefix -I,$(SUBDIRS))
CXXFLAGS := -Wall -Werror $(shell sdl2-config --cflags)

DEBUG ?= 0

ifeq ($(DEBUG),1)
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -O3
endif

-include $(DEPS)

.PHONY: all clean echoes tests

all: $(OUTDIR)/$(GAMELIB)

$(OUTDIR)/$(GAMELIB): $(OBJS)
	mkdir -p $(@D)
	$(AR) rcs $@ $^

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

TEST_SRCS := $(wildcard test/*.cpp)
TESTS := $(TEST_SRCS:.cpp=)

tests: $(TESTS)

$(TESTS): %: %.cpp $(OUTDIR)/$(GAMELIB)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ $< \
		-L$(OUTDIR) -ljgame $(LIBS)

clean:
	$(RM) $(OUTDIR) $(OBJDIR)

echoes:
	@echo "SRC files : $(SRCS)"
	@echo "OBJ files : $(OBJS)"
	@echo "CPPFLAGS  : $(CPPFLAGS)"
	@echo "CXXFLAGS  : $(CXXFLAGS)"
	@echo "SUBDIRS   : $(SUBDIRS)"