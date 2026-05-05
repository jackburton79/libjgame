CC = g++
RM = rm -rf

GAMELIB = libjgame.a
LIBS = -lz `sdl2-config --libs`
CXXFLAGS = -Wall -Werror `sdl2-config --cflags`

DEBUG ?= 0
# DEBUG=TRUE or DEBUG=1
ifeq ($(DEBUG),1)
	DEBUG := TRUE
endif

ifneq ($(DEBUG),TRUE)
	CXXFLAGS += -O3
else
	CXXFLAGS += -g -O0
endif

SUBDIR = \
audio \
graphics \
shell \
streams \
support \
timers

OUTDIR = ./lib
DIR_OBJ = ./obj
SRCS = $(wildcard /*.cpp $(foreach fd, $(SUBDIR), $(fd)/*.cpp))
NODIR_SRC = $(notdir $(SRCS))
OBJS = $(addprefix $(DIR_OBJ)/, $(SRCS:cpp=o)) # obj/xxx.o obj/folder/xxx .o
INC_DIRS = -I./ $(addprefix -I, $(SUBDIR))

DEP = $(OBJS:%.o=%.d)

-include $(DEP)

#PHONY := all
all: $(GAMELIB)

PHONY := $(GAMELIB)

$(GAMELIB): $(OBJS)
	mkdir -p $(OUTDIR)
	ar rc $(OUTDIR)/$(GAMELIB) $(OBJS)
	ranlib $(OUTDIR)/$(GAMELIB)

$(DIR_OBJ)/%.o: %.cpp
	mkdir -p $(@D)
	$(CC) -o $@ $(CXXFLAGS) -MMD -c $< $(INC_DIRS)

PHONY += clean
clean:
	rm -rf $(OUTDIR)/* $(DIR_OBJ)/*

PHONY += echoes
echoes:
	@echo "INC files: $(INCS)"
	@echo "SRC files: $(SRCS)"
	@echo "OBJ files: $(OBJS)"
	@echo "LIB files: $(LIBS)"
	@echo "INC DIR: $(INC_DIRS)"
	@echo "LIB DIR: $(LIB_DIRS)"
	@echo "SUBDIR: $(SUBDIR)"

.PHONY = $(PHONY)
