# Makefile for Send Martin Tower to Mars! (C + raylib port)
#
#   make            # desktop build (native gcc)
#   make PLATFORM=PLATFORM_WEB EMSDK=/path/to/emsdk   # WebAssembly build
#   make run        # build + run desktop
#   make clean
#
# The online leaderboard lives in the lightly-salted-engine submodule (engine/)
# and links a vendored static mbedTLS (vendor/mbedtls) for HTTPS. The prebuilt
# libmbedtls.a is committed, so a normal build never recompiles mbedTLS; delete
# it with `make clean-mbedtls` to force a one-time rebuild from source.

PLATFORM ?= PLATFORM_DESKTOP
SRC      := $(wildcard src/*.c) $(wildcard src/screens/*.c)
HDR      := $(wildcard src/*.h) $(wildcard vendor/*.h)
NAME     := martin

# Shared engine (git submodule). cJSON + leaderboard client compiled in place.
ENGINE      := engine
MBEDTLS_DIR := vendor/mbedtls
MBEDTLS_LIB := $(MBEDTLS_DIR)/libmbedtls.a
MBEDTLS_SRCS := $(wildcard $(MBEDTLS_DIR)/library/*.c)
MBEDTLS_OBJS := $(MBEDTLS_SRCS:.c=.o)

CFLAGS   := -std=gnu11 -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Isrc -Ivendor -I$(ENGINE)

ifeq ($(PLATFORM),PLATFORM_WEB)
  # Web build. Requires emscripten on PATH (source emsdk_env.sh first) and a
  # raylib built for web. By default both the headers and the static lib come
  # from the vendored raylib source (build it once with:
  #   cd vendor/raylib-src/src && make PLATFORM=PLATFORM_WEB).
  # The web leaderboard uses the single-threaded fetch() stub (no mbedTLS/pthread).
  CC          := emcc
  OUTPUT      := build/index.html
  RAYLIB_SRC  ?= vendor/raylib-src/src
  RAYLIB_WEB_LIB ?= $(RAYLIB_SRC)/libraylib.a
  CFLAGS  += -Os -DPLATFORM_WEB -I$(RAYLIB_SRC)
  LDFLAGS := -s USE_GLFW=3 -s ASYNCIFY -s TOTAL_MEMORY=134217728 \
             -s ALLOW_MEMORY_GROWTH=1 -s FETCH=1 \
             --exclude-file '*cacert.pem' \
             --preload-file assets --shell-file shell.html $(RAYLIB_WEB_LIB)
  ENGINE_SRC  := $(ENGINE)/leaderboard_web.c $(ENGINE)/cJSON.c $(ENGINE)/crt.c $(ENGINE)/vfs.c
  MBEDTLS_DEP :=
else
  CC      := cc
  OUTPUT  := $(NAME)
  CFLAGS  += -O2 -g -DPLATFORM_DESKTOP -I$(MBEDTLS_DIR)/include $(shell pkg-config --cflags raylib 2>/dev/null)
  LDFLAGS := $(shell pkg-config --libs raylib 2>/dev/null) $(MBEDTLS_LIB) -lm -lpthread -ldl -lrt -lX11
  ENGINE_SRC  := $(ENGINE)/leaderboard.c $(ENGINE)/cJSON.c $(ENGINE)/crt.c $(ENGINE)/vfs.c
  MBEDTLS_DEP := $(MBEDTLS_LIB)
endif

.PHONY: all run clean clean-mbedtls
all: $(OUTPUT)

$(OUTPUT): $(SRC) $(ENGINE_SRC) $(HDR) $(MBEDTLS_DEP)
ifeq ($(PLATFORM),PLATFORM_WEB)
	@mkdir -p build
endif
	$(CC) $(CFLAGS) $(SRC) $(ENGINE_SRC) -o $(OUTPUT) $(LDFLAGS)

# Build the vendored mbedTLS static lib from source. The prebuilt .a is committed,
# so this no-prereq rule fires only when the lib is missing (e.g. after
# clean-mbedtls), keeping normal app iteration fast.
$(MBEDTLS_LIB):
	@echo "Building vendored mbedTLS (one-time)..."
	@for f in $(MBEDTLS_SRCS); do \
	  $(CC) -std=c17 -O2 -DNDEBUG -D_POSIX_C_SOURCE=200112L -I$(MBEDTLS_DIR)/include -c $$f -o $${f%.c}.o || exit 1; \
	done
	ar rcs $@ $(MBEDTLS_OBJS)

run: all
	./$(OUTPUT)

clean:
	rm -f $(NAME)
	rm -rf build

clean-mbedtls:
	rm -f $(MBEDTLS_OBJS) $(MBEDTLS_LIB)
