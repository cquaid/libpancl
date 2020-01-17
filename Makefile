.SECONDEXPANSION:

ifeq ($(CURDIR), )
CURDIR = $(dir $(lastword $(MAKEFILELIST)))
endif

# General Settings
NAME := pancl
MAJOR := 1
MINOR := 0

STATIC_LIB_NAME     := lib$(NAME).a
DYNAMIC_LIB_SUBNAME := lib$(NAME).so.$(MAJOR)
DYNAMIC_LIB_NAME    := $(DYNAMIC_LIB_SUBNAME).$(MINOR)

# Install variables
DESTDIR ?= /usr/local
incdir ?= $(DESTDIR)/include
libdir ?= $(DESTDIR)/lib

# Build variables
BINDIR := $(CURDIR)/bin
OBJDIR := $(CURDIR)/obj
SRCDIR := $(CURDIR)/src
INCDIR := $(CURDIR)/include

SHARED_CFLAGS = \
	-std=c99 -pedantic -Wall -Werror \
	-I$(INCDIR) -I$(SRCDIR)

ifneq ($(WITH_DEBUG), )
SHARED_CFLAGS += -g
endif

SOURCES := $(wildcard \
	$(SRCDIR)/*.c \
	$(SRCDIR)/lexer/*.c \
	$(SRCDIR)/parser/*.c \
	$(SRCDIR)/types/*.c)

INCLUDE_FILES := $(wildcard $(INCDIR)/*.h)

STATIC_LIB    := $(BINDIR)/$(STATIC_LIB_NAME)
STATIC_OBJDIR := $(OBJDIR)/$(STATIC_LIB_NAME)
STATIC_OBJ    := $(patsubst $(SRCDIR)/%.c,$(STATIC_OBJDIR)/%.o,$(SOURCES))

DYNAMIC_LIB    := $(BINDIR)/$(DYNAMIC_LIB_NAME)
DYNAMIC_OBJDIR := $(OBJDIR)/$(DYNAMIC_LIB_NAME)
DYNAMIC_OBJ    := $(patsubst $(SRCDIR)/%.c,$(DYNAMIC_OBJDIR)/%.o,$(SOURCES))

.PHONY: all
all: build

.PHONY: build
build: $(STATIC_LIB) $(DYNAMIC_LIB)


$(STATIC_LIB): STATIC_CFLAGS = $(SHARED_CFLAGS)
$(STATIC_LIB): $(STATIC_OBJ)
	@mkdir -p $(dir $@)
	$(AR) $(ARFLAGS) $@ $(STATIC_OBJ)

$(STATIC_OBJ): SRC = $(patsubst $(STATIC_OBJDIR)/%.o,$(SRCDIR)/%.c,$@)
$(STATIC_OBJ): $$(SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(STATIC_CFLAGS) $(CFLAGS) -o $@ -c $(SRC)


$(DYNAMIC_LIB): DYNAMIC_CFLAGS = -fPIC $(SHARED_CFLAGS)
$(DYNAMIC_LIB): DYNAMIC_LDFLAGS = -shared -Wl,-soname,$(DYNAMIC_LIB_SUBNAME)
$(DYNAMIC_LIB): $(DYNAMIC_OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(DYNAMIC_OBJ) $(DYNAMIC_LDFLAGS) $(LDLIBS) -o $@

$(DYNAMIC_OBJ): SRC = $(patsubst $(DYNAMIC_OBJDIR)/%.o,$(SRCDIR)/%.c,$@)
$(DYNAMIC_OBJ): $$(SRC)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(DYNAMIC_CFLAGS) $(CFLAGS) -o $@ -c $(SRC)


.PHONY: clean
clean:
	-rm $(STATIC_OBJ) $(DYNAMIC_OBJ) $(STATIC_LIB) $(DYNAMIC_LIB)

.PHONY: distclean
distclean: clean
	-rm -r $(OBJDIR) $(BINDIR)


HDR_INSTALL = \
	$(patsubst $(INCDIR)/%.h,$(incdir)/$(NAME)/%.h,$(INCLUDE_FILES))

$(incdir)/$(NAME)/%: $(INCDIR)/%
	@mkdir -p $(dir $@)
	cp $< $@

.PHONY: install-headers
install-headers: $(HDR_INSTALL)


$(libdir)/$(STATIC_LIB_NAME): $(STATIC_LIB)
	@mkdir -p $(dir $@)
	cp $< $@

.PHONY: install-static
install-static: $(libdir)/$(STATIC_LIB_NAME)


$(libdir)/$(DYNAMIC_LIB_NAME): $(DYNAMIC_LIB)
	@mkdir -p $(dir $@)
	cp $< $@

.PHONY: install-dynamic
install-dynamic: $(libdir)/$(DYNAMIC_LIB_NAME)


.PHONY: install
install: build install-headers install-static install-dynamic

.PHONY: uninstall
uninstall:
	-rm $(HDR_INSTALL)
	-rm -r $(incdir)/$(NAME)/
	-rm $(libdir)/$(STATIC_LIB_NAME)
	-rm $(libdir)/$(DYNAMIC_LIB_NAME)

# vim:ts=4:sw=4:autoindent
