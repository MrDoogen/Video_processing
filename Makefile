BIN_FILE = main

CXX = g++

SRC_FILES += main.cpp
SRC_FILES += bmp.cpp
SRC_FILES += read_file.cpp
SRC_FILES += fsize.cpp

CFLAGS += -Iinc -Wall
build: CFLAGS += -s -O3
debug: CFLAGS += -g -O0

LDFLAGS += -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
build: LDFLAGS += -static

OBJ_FILES := $(patsubst %.cpp,obj/%.o,$(SRC_FILES))
QUIET_CXX = @echo '   ' CXX $(notdir $@);

VPATH := src

#.ONESHELL:
.PHONY: build debug

all: build

build: mkdirs _build

_build: $(OBJ_FILES)
	$(CXX) $^ -o bin/$(BIN_FILE) $(LDFLAGS)

debug: mkdirs _debug

_debug: $(OBJ_FILES)
	$(CXX) $^ -o bin/$(BIN_FILE) $(LDFLAGS)

obj/%.o: %.cpp
	$(QUIET_CXX) $(CXX) -c $< -o $@ $(CFLAGS)

clean:
	rm -f obj/*

mkdirs: 
	@if [ ! -d bin ] ; then \
		mkdir bin ; \
	fi

	@if [ ! -d obj ] ; then \
		mkdir obj ; \
	fi
