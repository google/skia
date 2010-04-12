# Simple makefile for skia library and test apps

# setup our defaults
CC := gcc
C_INCLUDES := -Iinclude/config -Iinclude/core -Iinclude/effects -Iinclude/images -Iinclude/utils
CFLAGS := -Wall -g # -O2 
CFLAGS_SSE2 = $(CFLAGS) -msse2
LINKER_OPTS := -lpthread
DEFINES := -DSK_CAN_USE_FLOAT
HIDE = @

ifeq ($(SKIA_SCALAR),fixed)
	DEFINES += -DSK_SCALAR_IS_FIXED
else
	DEFINES += -DSK_SCALAR_IS_FLOAT
endif

ifeq ($(SKIA_DEBUG),true)
 	DEFINES += -DSK_DEBUG -DSK_SUPPORT_UNIT
else
	DEFINES += -DSK_RELEASE
endif

DEFINES += -DSK_SUPPORT_LCDTEXT

# start with the core (required)
include src/core/core_files.mk
SRC_LIST := $(addprefix src/core/, $(SOURCE))

# add the opts (optimizations)
include src/opts/opts_files.mk
SRC_LIST += $(addprefix src/opts/, $(SOURCE))

# we usually need ports
include src/ports/ports_files.mk
SRC_LIST += $(addprefix src/ports/, $(SOURCE))

# do we want effects?
include src/effects/effects_files.mk
SRC_LIST += $(addprefix src/effects/, $(SOURCE))

# core image files
include src/images/images_files.mk
SRC_LIST += $(addprefix src/images/, $(SOURCE))

# core util files
include src/utils/utils_files.mk
SRC_LIST += $(addprefix src/utils/, $(SOURCE))

# extra files we want to build to prevent bit-rot, but not link
JUST_COMPILE_LIST := src/ports/SkFontHost_tables.cpp

# conditional files based on our platform
ifeq ($(SKIA_BUILD_FOR),mac)
	LINKER_OPTS += -framework Carbon
	DEFINES += -DSK_BUILD_FOR_MAC

	C_INCLUDES += -Iinclude/utils/mac
	SRC_LIST += src/ports/SkImageDecoder_CG.cpp
	SRC_LIST += src/utils/mac/SkCreateCGImageRef.cpp
	SRC_LIST += src/ports/SkFontHost_mac.cpp
else
	LINKER_OPTS += -lpng -lfreetype
	DEFINES += -DSK_BUILD_FOR_UNIX

	# needed for freetype support
	C_INCLUDES += -I/usr/include/freetype2
	SRC_LIST += src/ports/SkFontHost_linux.cpp
	SRC_LIST += src/ports/SkFontHost_gamma_none.cpp
	SRC_LIST += src/ports/SkFontHost_FreeType.cpp
	SRC_LIST += src/ports/SkFontHost_FreeType_Subpixel.cpp
	SRC_LIST += src/utils/SkOSFile.cpp
    # these are our registry-based factories
	SRC_LIST += src/images/SkImageDecoder_Factory.cpp
	SRC_LIST += src/images/SkImageEncoder_Factory.cpp
    # support files
	SRC_LIST += src/images/SkScaledBitmapSampler.cpp
endif

# For these files, and these files only, compile with -msse2.
SSE2_OBJS := out/src/opts/SkBlitRow_opts_SSE2.o \
             out/src/opts/SkUtils_opts_SSE2.o
$(SSE2_OBJS) : CFLAGS := $(CFLAGS_SSE2)

out/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(HIDE)$(CC) $(C_INCLUDES) $(CFLAGS) $(DEFINES) -c $< -o $@
	@echo "compiling $@"
    
%.s : %.cpp
	@mkdir -p $(dir $@)
	$(CC) $(C_INCLUDES) $(CFLAGS) $(DEFINES) -S -c $< -o $@

# now build out objects
OBJ_LIST := $(SRC_LIST:.cpp=.o)
OBJ_LIST := $(addprefix out/, $(OBJ_LIST))

# we want to compile these, but we don't actually link them
JUST_COMPILE_OBJS := $(JUST_COMPILE_LIST:.cpp=.o)
JUST_COMPILE_OBJS := $(addprefix out/, $(JUST_COMPILE_OBJS))

out/libskia.a: Makefile $(OBJ_LIST) $(JUST_COMPILE_OBJS)
	$(HIDE)$(AR) ru $@ $(OBJ_LIST)
	$(HIDE)ranlib $@

##############################################################################

BENCH_SRCS := RectBench.cpp SkBenchmark.cpp benchmain.cpp BitmapBench.cpp \
			  RepeatTileBench.cpp DecodeBench.cpp
BENCH_SRCS := $(addprefix bench/, $(BENCH_SRCS))

# add any optional codecs for this app
ifeq ($(SKIA_BUILD_FOR),mac)
    BENCH_SRCS += bench/TextBench.cpp
else
    BENCH_SRCS += src/images/SkImageDecoder_libpng.cpp
endif

BENCH_OBJS := $(BENCH_SRCS:.cpp=.o)
BENCH_OBJS := $(addprefix out/, $(BENCH_OBJS))

bench: $(BENCH_OBJS) out/libskia.a
	@echo "linking bench..."
	$(HIDE)g++ $(BENCH_OBJS) out/libskia.a -o out/bench/bench $(LINKER_OPTS)
	
##############################################################################

# we let tests cheat and see private headers, so we can unittest modules
C_INCLUDES += -Isrc/core

include tests/tests_files.mk
TESTS_SRCS := $(addprefix tests/, $(SOURCE))

TESTS_OBJS := $(TESTS_SRCS:.cpp=.o)
TESTS_OBJS := $(addprefix out/, $(TESTS_OBJS))

tests: $(TESTS_OBJS) out/libskia.a
	@echo "linking tests..."
	$(HIDE)g++ $(TESTS_OBJS) out/libskia.a -o out/tests/tests $(LINKER_OPTS)
	
##############################################################################

SKIMAGE_SRCS := skimage_main.cpp

SKIMAGE_SRCS := $(addprefix tools/, $(SKIMAGE_SRCS))

SKIMAGE_OBJS := $(SKIMAGE_SRCS:.cpp=.o)
SKIMAGE_OBJS := $(addprefix out/, $(SKIMAGE_OBJS))

skimage: $(SKIMAGE_OBJS) out/libskia.a
	@echo "linking skimage..."
	$(HIDE)g++ $(SKIMAGE_OBJS) out/libskia.a -o out/tools/skimage $(LINKER_OPTS)

##############################################################################

SKHELLO_SRCS := skhello.cpp

SKHELLO_SRCS := $(addprefix tools/, $(SKHELLO_SRCS))

SKHELLO_OBJS := $(SKHELLO_SRCS:.cpp=.o)
SKHELLO_OBJS := $(addprefix out/, $(SKHELLO_OBJS))

skhello: $(SKHELLO_OBJS) out/libskia.a
	@echo "linking shkello..."
	$(HIDE)g++ $(SKHELLO_OBJS) out/libskia.a -o out/tools/skhello $(LINKER_OPTS)

##############################################################################

include gm/gm_files.mk
GM_SRCS := $(addprefix gm/, $(SOURCE))

ifneq ($(SKIA_BUILD_FOR),mac)
    GM_SRCS += src/images/SkImageDecoder_libpng.cpp
endif

GM_OBJS := $(GM_SRCS:.cpp=.o)
GM_OBJS := $(addprefix out/, $(GM_OBJS))

gm: $(GM_OBJS) out/libskia.a
	@echo "linking gm..."
	$(HIDE)g++ $(GM_OBJS) out/libskia.a -o out/gm/gm $(LINKER_OPTS)

##############################################################################

.PHONY: all
all: $ bench gm skimage tests skhello

.PHONY: clean
clean:
	$(HIDE)rm -rf out

.PHONY: help
help:
	@echo "Targets:"
	@echo "    <default>: out/libskia.a"
	@echo "    bench: out/bench/bench"
	@echo "    gm: out/gm/gm"
	@echo "    skimage: out/tools/skimage"
	@echo "    skhello: out/tools/skhello"
	@echo "    tests: out/tests/tests"
	@echo "    clean: removes entire out/ directory"
	@echo "    help: this text"
	@echo "Options: (after make, or in bash shell)"
	@echo "    SKIA_DEBUG=true for debug build"
	@echo "    SKIA_SCALAR=fixed for fixed-point build"
	@echo "    SKIA_BUILD_FOR=mac for mac build (e.g. CG for image decoding)"
	@echo ""
