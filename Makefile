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
	input \
	shell \
	streams \
	support \
	timers

SRCS := $(wildcard *.cpp $(foreach d,$(SUBDIRS),$(d)/*.cpp))
OBJS := $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o))
DEPS := $(OBJS:.o=.d)

CPPFLAGS := -I. $(addprefix -I,$(SUBDIRS))
# (C++17: the code uses structured bindings, which a compiler whose default is older refuses)
CXXFLAGS := -std=c++17 -Wall -Werror $(shell sdl2-config --cflags)

DEBUG ?= 0

ifeq ($(DEBUG),1)
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -O3
endif

-include $(DEPS)

.PHONY: all clean echoes tests test

all: $(OUTDIR)/$(GAMELIB)

$(OUTDIR)/$(GAMELIB): $(OBJS)
	mkdir -p $(@D)
	$(AR) rcs $@ $^

$(OBJDIR)/%.o: %.cpp
	mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

TESTDIR := test
TESTBINDIR := $(OBJDIR)/tests

TEST_SRCS := $(wildcard $(TESTDIR)/*.cpp)
TESTS := $(patsubst $(TESTDIR)/%.cpp,$(TESTBINDIR)/%,$(TEST_SRCS))

tests: $(TESTS)

# Runs every test, with no screen and no sound card, and stops at the first one
# that fails
test: tests
	@for t in $(TESTS); do \
		echo "== $$t"; \
		SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy $$t || exit 1; \
	done

$(TESTBINDIR)/%: $(TESTDIR)/%.cpp $(OUTDIR)/$(GAMELIB)
	mkdir -p $(@D)
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
