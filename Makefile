# C++ compiler
CXX = `$(WX_CONFIG) --cxx`

# Standard flags for C++
CXXFLAGS ?= -Wno-write-strings $(shell pkg-config --cflags libftdi1) 

# Standard preprocessor flags (common for CC and CXX)
CPPFLAGS ?=

# Standard linker flags
LDFLAGS ?= $(shell pkg-config --libs libftdi1)

# Location and arguments of wx-config script
WX_CONFIG ?= wx-config

# Port of the wx library to build against [gtk1,gtk2,msw,x11,motif,osx_cocoa,osx_carbon,dfb]
WX_PORT ?= $(shell $(WX_CONFIG) --query-toolkit)

# Use DLL build of wx library to use? [0,1]
WX_SHARED ?= $(shell if test -z `$(WX_CONFIG) --query-linkage`; then echo 1; else echo 0; fi)

# Compile Unicode build of wxWidgets? [0,1]
WX_UNICODE ?= $(shell $(WX_CONFIG) --query-chartype | sed 's/unicode/1/;s/ansi/0/')

# Version of the wx library to build against.
WX_VERSION ?= $(shell $(WX_CONFIG) --query-version | sed -e 's/\([0-9]*\)\.\([0-9]*\)/\1\2/')



# -------------------------------------------------------------------------
# Do not modify the rest of this file!
# -------------------------------------------------------------------------

### Variables: ###

CPPDEPS = -MT$@ -MF`echo $@ | sed -e 's,\.o$$,.d,'` -MD -MP
WX_VERSION_MAJOR = $(shell echo $(WX_VERSION) | cut -c1,1)
WX_VERSION_MINOR = $(shell echo $(WX_VERSION) | cut -c2,2)
WX_CONFIG_FLAGS = $(WX_CONFIG_UNICODE_FLAG) $(WX_CONFIG_SHARED_FLAG) \
	--toolkit=$(WX_PORT) --version=$(WX_VERSION_MAJOR).$(WX_VERSION_MINOR)
ROCKETFLASH_CXXFLAGS = -I. `$(WX_CONFIG) --cxxflags $(WX_CONFIG_FLAGS)` $(CPPFLAGS) \
	$(CXXFLAGS)
ROCKETFLASH_OBJECTS =  \
	rocketflash_rocketflash.o

### Conditionally set variables: ###

ifeq ($(WX_UNICODE),0)
WX_CONFIG_UNICODE_FLAG = --unicode=no
endif
ifeq ($(WX_UNICODE),1)
WX_CONFIG_UNICODE_FLAG = --unicode=yes
endif
ifeq ($(WX_SHARED),0)
WX_CONFIG_SHARED_FLAG = --static=yes
endif
ifeq ($(WX_SHARED),1)
WX_CONFIG_SHARED_FLAG = --static=no
endif


### Targets: ###

all: test_for_selected_wxbuild rocketflash

install:

uninstall:

clean:
	rm -f ./*.o
	rm -f ./*.d
	rm -f rocketflash

test_for_selected_wxbuild:
	@$(WX_CONFIG) $(WX_CONFIG_FLAGS)

rocketflash: $(ROCKETFLASH_OBJECTS)
	$(CXX) -o $@ $(ROCKETFLASH_OBJECTS)   $(LDFLAGS)  `$(WX_CONFIG) $(WX_CONFIG_FLAGS) --libs core,base`

rocketflash_rocketflash.o: ./rocketflash.cpp
	$(CXX) -c -o $@ $(ROCKETFLASH_CXXFLAGS) $(CPPDEPS) $<

.PHONY: all install uninstall clean
