# OpenLara IRIX/SGI Makefile (use gcc)

TARGET  := openlara
PREFIX  ?= /usr/local
CXX     ?= g++
CC      ?= gcc

SDL_CFLAGS  := $(shell sdl2-config --cflags)
#SDL_LDFLAGS := $(shell sdl2-config --libs)

# use SDL2 with GL support from flexions repo
SDL_LDFLAGS := -L/usr/sgug/lib32 /usr/sgug-flx/lib32/libSDL2-2.0.so.0 -Wl,-rpath=/usr/sgug-flx/lib32:./lib:../lib:/usr/lib32:/usr/sgug/lib32

INCLUDES := -I../../

# Flags shared by both C and C++ compilation units
COMMON_FLAGS := \
    -D__SDL2__ \
    -O2 \
    -ffunction-sections -fdata-sections \
    -Dalloca=__builtin_alloca \
    $(SDL_CFLAGS)

CXXFLAGS := $(COMMON_FLAGS) -std=c++11 -fno-exceptions -fno-rtti
CFLAGS   := $(COMMON_FLAGS)

# C++ sources
CXXSRCS := main.cpp \
           ../../libs/minimp3/minimp3.cpp

# C sources
CSRCS   := ../../libs/stb_vorbis/stb_vorbis.c \
           ../../libs/tinf/tinflate.c

CXXOBJS := $(CXXSRCS:.cpp=.o)
COBJS   := $(CSRCS:.c=.o)
OBJS    := $(CXXOBJS) $(COBJS)

LDFLAGS := $(SDL_LDFLAGS) -lGL -lGLcore -lGLU -lm -lpthread \
           -static-libstdc++ -static-libgcc \
           -Wl,--allow-shlib-undefined

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

$(CXXOBJS): %.o: %.cpp
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c $< -o $@

$(COBJS): %.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

install: $(TARGET)
	install -d $(DESTDIR)$(PREFIX)/bin/
	install $(TARGET) $(DESTDIR)$(PREFIX)/bin/OpenLara

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/OpenLara

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: install uninstall clean
