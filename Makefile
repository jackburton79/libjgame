.DEFAULT_GOAL := all

CXX := g++
AR := ar
RM := rm -rf

LIBS := -lz $(shell sdl2-config --libs)
GAMELIB := libjgame.a
VERSION := 4.0

# Where `make install` puts the library, the headers (in $(PREFIX)/include/jgame)
# and jgame.pc
PREFIX ?= /usr/local

OUTDIR := lib
OBJDIR := obj

# The directories with the sources (the public headers are all in include/jgame)
SUBDIRS := \
	audio \
	graphics \
	input \
	streams \
	support \
	timers

SRCS := $(wildcard *.cpp $(foreach d,$(SUBDIRS),$(d)/*.cpp))
OBJS := $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o))
DEPS := $(OBJS:.o=.d)

CPPFLAGS := -Iinclude
# (C++17: the code uses structured bindings, which a compiler whose default is older refuses)
CXXFLAGS := -std=c++17 -Wall -Werror $(shell sdl2-config --cflags)

DEBUG ?= 0

ifeq ($(DEBUG),1)
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -O3
endif

-include $(DEPS)

.PHONY: all clean echoes tests test install uninstall

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

install: all
	install -d $(DESTDIR)$(PREFIX)/include/jgame $(DESTDIR)$(PREFIX)/lib/pkgconfig
	install -m 644 include/jgame/*.h $(DESTDIR)$(PREFIX)/include/jgame
	install -m 644 $(OUTDIR)/$(GAMELIB) $(DESTDIR)$(PREFIX)/lib
	sed -e 's|@PREFIX@|$(PREFIX)|' -e 's|@VERSION@|$(VERSION)|' jgame.pc.in \
		> $(DESTDIR)$(PREFIX)/lib/pkgconfig/jgame.pc

uninstall:
	$(RM) $(DESTDIR)$(PREFIX)/include/jgame \
		$(DESTDIR)$(PREFIX)/lib/$(GAMELIB) \
		$(DESTDIR)$(PREFIX)/lib/pkgconfig/jgame.pc

clean:
	$(RM) $(OUTDIR) $(OBJDIR)

echoes:
	@echo "SRC files : $(SRCS)"
	@echo "OBJ files : $(OBJS)"
	@echo "CPPFLAGS  : $(CPPFLAGS)"
	@echo "CXXFLAGS  : $(CXXFLAGS)"
	@echo "SUBDIRS   : $(SUBDIRS)"
