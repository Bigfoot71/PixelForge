# Variables
CC := gcc
CFLAGS := -Wall -Wextra -pedantic -std=c99 -O3 -Wno-sign-compare
LDFLAGS :=

# Build options
PF_INSTALL := 0
PF_BUILD_STATIC := 1
PF_BUILD_SHARED := 0

# Source files
SRCS := \
    src/blend.c \
    src/render.c \
    src/texture.c \
    src/framebuffer.c

# Header files
HDRS := $(wildcard src/*.h)

# Static library target
ifeq ($(PF_BUILD_STATIC),1)
pixelforge_static: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -c $(SRCS)
	ar rcs libpixelforge.a *.o
	rm -f *.o
endif

# Shared library target
ifeq ($(PF_BUILD_SHARED),1)
	LDFLAGS += -shared -o
	ifeq ($(OS),Windows_NT)
		SHARED_EXT := dll
	else
		SHARED_EXT := so
	endif

pixelforge_dynamic: $(SRCS) $(HDRS)
	$(CC) $(CFLAGS) -fPIC $(LDFLAGS) libpixelforge.$(SHARED_EXT) $(SRCS)
endif

# Installation
ifeq ($(PF_INSTALL),1)
install:
	# Installing static libraries if the STATIC option is enabled
	ifeq ($(PF_BUILD_STATIC),1)
		cp libpixelforge.a /usr/local/lib/
	endif

	# Installing shared libraries if the SHARED option is enabled
	ifeq ($(PF_BUILD_SHARED),1)
		cp libpixelforge.$(SHARED_EXT) /usr/local/lib/
	endif

	# Installing header files
	cp $(HDRS) /usr/local/include/
endif

# Clean
clean:
	rm -f *.o libpixelforge.a libpixelforge.$(SHARED_EXT)