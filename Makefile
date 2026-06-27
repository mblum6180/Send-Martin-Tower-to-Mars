# Makefile for Send Martin Tower to Mars! (C + raylib port)
#
#   make            # desktop build (native gcc)
#   make PLATFORM=PLATFORM_WEB EMSDK=/path/to/emsdk   # WebAssembly build
#   make run        # build + run desktop
#   make clean

PLATFORM ?= PLATFORM_DESKTOP
SRC      := $(wildcard src/*.c) $(wildcard src/screens/*.c)
HDR      := $(wildcard src/*.h) $(wildcard vendor/*.h)
NAME     := martin

CFLAGS   := -std=gnu11 -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Isrc -Ivendor

ifeq ($(PLATFORM),PLATFORM_WEB)
  # Web build. Requires emscripten on PATH (source emsdk_env.sh first) and a
  # raylib built for web. By default both the headers and the static lib come
  # from the vendored raylib source (build it once with:
  #   cd vendor/raylib-src/src && make PLATFORM=PLATFORM_WEB).
  CC          := emcc
  OUTPUT      := build/index.html
  RAYLIB_SRC  ?= vendor/raylib-src/src
  RAYLIB_WEB_LIB ?= $(RAYLIB_SRC)/libraylib.a
  CFLAGS  += -Os -DPLATFORM_WEB -I$(RAYLIB_SRC)
  LDFLAGS := -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=134217728 \
             -s ALLOW_MEMORY_GROWTH=1 \
             --preload-file assets --shell-file shell.html $(RAYLIB_WEB_LIB)
else
  CC      := cc
  OUTPUT  := $(NAME)
  CFLAGS  += -O2 -g -DPLATFORM_DESKTOP $(shell pkg-config --cflags raylib 2>/dev/null)
  LDFLAGS := $(shell pkg-config --libs raylib 2>/dev/null) -lm -lpthread -ldl -lrt -lX11
endif

.PHONY: all run clean
all: $(OUTPUT)

$(OUTPUT): $(SRC) $(HDR)
ifeq ($(PLATFORM),PLATFORM_WEB)
	@mkdir -p build
endif
	$(CC) $(CFLAGS) $(SRC) -o $(OUTPUT) $(LDFLAGS)

run: all
	./$(OUTPUT)

clean:
	rm -f $(NAME)
	rm -rf build
