# Copyright (C) 2017 Hideaki Narita

PROJECTROOT=..
JAVADIR=$(PROJECTROOT)/TimeMachine

BINDIR=bin
OBJDIR=obj

CC=gcc
LD=gcc

RM=rm -f
RMDIRS=rm -fr
MKDIRS=mkdir -p

INCLUDES=\
-I$(JAVADIR)/include \
-I$(JDK_HOME)/include \
-I$(JDK_HOME)/include/linux

CFLAGS=-fPIC -Wall -Werror
CPPFLAGS=-DLINUX -D_GNU_SOURCE $(INCLUDES)
LDFLAGS=-shared

COMPILE=$(CC) $(CFLAGS) $(CPPFLAGS) -c
LINK=$(LD) $(LDFLAGS)

$(OBJDIR)/%.o: %.c
	@test -d $(@D) || $(MKDIRS) $(@D)
	$(COMPILE) -o $@ $<

all::

PROJ=$(BINDIR)/libTimeMachine.so
OBJS=$(OBJDIR)/TimeMachine.o
LIBS=-ldl

$(PROJ): $(OBJS)
	@test -d $(@D) || $(MKDIRS) $(@D)
	$(LINK) -o $@ $(OBJS) $(LIBS)

all:: $(PROJ)

clean distclean::
	$(RM) *~
	$(RMDIRS) $(OBJDIR)

distclean::
	$(RMDIRS) $(BINDIR)
