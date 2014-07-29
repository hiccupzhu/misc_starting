###
###     Makefile for H.264/AVC encoder
###
###             generated for UNIX/LINUX environments
###             by H. Schwarz, Limin Wang
###



NAME=   lencod

### include debug information: 1=yes, 0=no
DBG?= 0
### include MMX optimization : 1=yes, 0=no
M32?= 0

DEPEND= dependencies

BINDIR= ../bin
INCDIR= inc
SRCDIR= src
OBJDIR= obj

ADDSRCDIR= ../lcommon/src
ADDINCDIR= ../lcommon/inc

ifeq ($(M32),1)
CC=     $(shell which gcc) -m32
else
CC=     $(shell which gcc) 
endif

LIBS=   -lm
AFLAGS=  
CFLAGS=  -std=gnu99 -pedantic -ffloat-store -fno-strict-aliasing -fsigned-char
FLAGS=  $(CFLAGS) -Wall -I$(INCDIR) -I$(ADDINCDIR) -D __USE_LARGEFILE64 -D _FILE_OFFSET_BITS=64

ifeq ($(DBG),1)
SUFFIX= .dbg
FLAGS+= -g
else
SUFFIX=
FLAGS+= -O3
endif

OBJSUF= .o$(SUFFIX)

SRC=    $(wildcard $(SRCDIR)/*.c) 
ADDSRC= $(wildcard $(ADDSRCDIR)/*.c)
OBJ=    $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o$(SUFFIX)) $(ADDSRC:$(ADDSRCDIR)/%.c=$(OBJDIR)/%.o$(SUFFIX)) 
BIN=    $(BINDIR)/$(NAME)$(SUFFIX).exe

.PHONY: default distclean clean tags depend

default: messages objdir_mk depend bin 

messages:
ifeq ($(M32),1)
	@echo 'Compiling with M32 support...'
endif
ifeq ($(DBG),1)
	@echo 'Compiling with Debug support...'
endif

clean:
	@echo remove all objects
	@rm -rf $(OBJDIR)

distclean: clean
	@rm -f $(DEPEND) tags
	@rm -f $(BIN)

tags:
	@echo update tag table
	@ctags inc/*.h src/*.c

bin:    $(OBJ)
	@echo
	@echo 'creating binary "$(BIN)"'
	@$(CC) $(AFLAGS) -o $(BIN) $(OBJ) $(LIBS)
	@echo '... done'
	@echo

depend:
	@echo
	@echo 'checking dependencies'
	@$(SHELL) -ec '$(CC) $(AFLAGS) -MM $(CFLAGS) -I$(INCDIR) -I$(ADDINCDIR) $(SRC) $(ADDSRC)                  \
         | sed '\''s@\(.*\)\.o[ :]@$(OBJDIR)/\1.o$(SUFFIX):@g'\''               \
         >$(DEPEND)'
	@echo

$(OBJDIR)/%.o$(SUFFIX): $(SRCDIR)/%.c
	@echo 'compiling object file "$@" ...'
	@$(CC) -c -o $@ $(FLAGS) $<

$(OBJDIR)/%.o$(SUFFIX): $(ADDSRCDIR)/%.c
	@echo 'compiling object file "$@" ...'
	@$(CC) -c -o $@ $(FLAGS) $<

objdir_mk:
	@echo 'Creating $(OBJDIR) ...'
	@mkdir -p $(OBJDIR)

-include $(DEPEND)

