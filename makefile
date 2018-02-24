# Windows version of VBCC requires absolute path in all .h files
# e.g. timer manager has to refer to timer.h by absolute path

# TODO: refactor so 'make' without args won't recompile whole ACE

# Multi-platform
ifdef ComSpec
	# Windows
	RM = del
	CP = copy
	SLASH = \\
	CURR_DIR=$(shell chdir)
	ECHO = @echo
	NEWLINE = @echo.
	QUIETCOPY = > NUL
else
	# Linux/Amiga
	RM = rm
	CP = cp
	SLASH = /
	CURR_DIR = $(shell pwd)
	ECHO = @echo
	NEWLINE = @echo " "
	QUIETCOPY =
endif
SL= $(strip $(SLASH))
SRC_DIR = $(CURR_DIR)$(SL)src

# Directories
TMP_DIR = build
ACE_DIR = ..$(SL)ace
ACE_INC_DIR = $(ACE_DIR)$(SL)include

# Compiler stuff
LD40_CC ?= vc

INCLUDES = -I$(SRC_DIR) -I$(ACE_DIR)/include
ifeq ($(LD40_CC), vc)
	CC_FLAGS = +kick13 -c99 $(INCLUDES) -DAMIGA
	ACE_AS = vc
	AS_FLAGS = +kick13 -c
	OBJDUMP =
else ifeq ($(LD40_CC), m68k-amigaos-gcc)
	CC_FLAGS = -std=gnu11 $(INCLUDES) -DAMIGA -noixemul -Wall -fomit-frame-pointer -O3
	ACE_AS = vasm
	AS_FLAGS = -quiet -x -m68010 -Faout
	OBJDUMP = m68k-amigaos-objdump -S -d $@ > $@.dasm
endif

# File list
LD40_MAIN_FILES = $(wildcard $(SRC_DIR)/*.c)
LD40_MAIN_OBJS = $(addprefix $(TMP_DIR)$(SL), $(notdir $(LD40_MAIN_FILES:.c=.o)))

LD40_GS_GAME_FILES = $(wildcard $(SRC_DIR)/gamestates/game/*.c)
LD40_GS_GAME_OBJS = $(addprefix $(TMP_DIR)$(SL)gsgame_, $(notdir $(LD40_GS_GAME_FILES:.c=.o)))

LD40_GS_MENU_FILES = $(wildcard $(SRC_DIR)/gamestates/menu/*.c)
LD40_GS_MENU_OBJS = $(addprefix $(TMP_DIR)$(SL)gsmenu_, $(notdir $(LD40_GS_MENU_FILES:.c=.o)))

LD40_FILES = $(LD40_MAIN_FILES) $(LD40_GS_GAME_FILES) $(LD40_GS_MENU_FILES)
LD40_OBJS = $(LD40_MAIN_OBJS) $(LD40_GS_GAME_OBJS) $(LD40_GS_MENU_OBJS)
ACE_OBJS = $(wildcard $(ACE_DIR)/build/*.o)

#
ace: $(ACE_OBJS)
	-make -C $(ACE_DIR) all ACE_CC=$(LD40_CC)
	$(NEWLINE)
	$(ECHO) Copying ACE objs
	$(NEWLINE)
	@$(CP) $(ACE_DIR)$(SL)build$(SL)*.o $(TMP_DIR) $(QUIETCOPY)

ld40: $(LD40_OBJS)
	$(NEWLINE)
	$(ECHO) Linking...
	@$(LD40_CC) $(CC_FLAGS) -lamiga -o $@ $^ $(ACE_OBJS)

# Main files
$(TMP_DIR)$(SL)%.o: $(SRC_DIR)/%.c
	$(ECHO) Building $<
	@$(LD40_CC) $(CC_FLAGS) -c -o $@ $<

# Game
$(TMP_DIR)$(SL)gsgame_%.o: $(SRC_DIR)/gamestates/game/%.c
	$(ECHO) Building $<
	@$(LD40_CC) $(CC_FLAGS) -c -o $@ $<

# Menu
$(TMP_DIR)$(SL)gsmenu_%.o: $(SRC_DIR)/gamestates/menu/%.c
	$(ECHO) Building $<
	@$(LD40_CC) $(CC_FLAGS) -c -o $@ $<

all: clear ace ld40

clear:
	$(ECHO) "a" > $(TMP_DIR)$(SL)foo.o
	$(RM) $(TMP_DIR)$(SL)*.o
