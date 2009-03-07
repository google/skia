# Simple makefile for skia library and test apps

# setup our defaults
CC := gcc
C_INCLUDES := -Iinclude/core -Iinclude/effects -Iinclude/images -Iinclude/utils
CFLAGS := -O2 
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

# start with the core (required)
include src/core/core_files.mk
SRC_LIST := $(addprefix src/core/, $(SOURCE))

# we usually need ports
include src/ports/ports_files.mk
SRC_LIST += $(addprefix src/ports/, $(SOURCE))

# do we want effects?
include src/effects/effects_files.mk
SRC_LIST += $(addprefix src/effects/, $(SOURCE))

# core image files
include src/images/images_files.mk
SRC_LIST += $(addprefix src/images/, $(SOURCE))

# conditional files based on our platform
ifeq ($(SKIA_BUILD_FOR),mac)
	LINKER_OPTS += -framework Carbon
	DEFINES += -DSK_BUILD_FOR_MAC

	C_INCLUDES += -Iinclude/utils/mac
	SRC_LIST += src/ports/SkImageDecoder_CG.cpp
	SRC_LIST += src/utils/mac/SkCreateCGImageRef.cpp
	SRC_LIST += src/ports/SkFontHost_mac.cpp
else
	LINKER_OPTS += -lpng
	DEFINES += -DSK_BUILD_FOR_UNIX

    # these are our registry-based factories
	SRC_LIST += src/images/SkImageDecoder_Factory.cpp
	SRC_LIST += src/images/SkImageEncoder_Factory.cpp
    # support files
	SRC_LIST += src/images/SkScaledBitmapSampler.cpp
endif

out/%.o : %.cpp
	@mkdir -p $(dir $@)
	$(HIDE)$(CC) $(C_INCLUDES) $(CFLAGS) $(DEFINES) -c $< -o $@
	@echo "compiling $@"
    
# now build out objects
OBJ_LIST := $(SRC_LIST:.cpp=.o)
OBJ_LIST := $(addprefix out/, $(OBJ_LIST))

out/libskia.a: Makefile $(OBJ_LIST)
	$(HIDE)$(AR) ru $@ $(OBJ_LIST)
	$(HIDE)ranlib $@

##############################################################################

BENCH_SRCS := RectBench.cpp SkBenchmark.cpp main.cpp BitmapBench.cpp
BENCH_SRCS := $(addprefix bench/, $(BENCH_SRCS))

# add any optional codecs for this app
ifeq ($(SKIA_BUILD_FOR),mac)
#    BENCH_SRCS += bench/TextBench.cpp
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

TESTS_SRCS := GeometryTest.cpp MathTest.cpp MatrixTest.cpp PackBitsTest.cpp \
              Sk64Test.cpp StringTest.cpp Test.cpp UtilsTest.cpp PathTest.cpp \
              ClipCubicTest.cpp SrcOverTest.cpp StreamTest.cpp SortTest.cpp \
			  PathMeasureTest.cpp main.cpp

TESTS_SRCS := $(addprefix tests/, $(TESTS_SRCS))

TESTS_OBJS := $(TESTS_SRCS:.cpp=.o)
TESTS_OBJS := $(addprefix out/, $(TESTS_OBJS))

tests: $(TESTS_OBJS) out/libskia.a
	@echo "linking tests..."
	$(HIDE)g++ $(TESTS_OBJS) out/libskia.a -o out/tests/tests $(LINKER_OPTS)
	
##############################################################################

.PHONY: clean
clean:
	$(HIDE)rm -rf out

.PHONY: help
help:
	@echo "Targets:"
	@echo "    <default>: out/libskia.a"
	@echo "    bench: out/bench/bench"
	@echo "    tests: out/tests/tests"
	@echo "    clean: removes entire out/ directory"
	@echo "    help: this text"
	@echo "Options: (after make, or in bash shell)"
	@echo "    SKIA_DEBUG=true for debug build"
	@echo "    SKIA_SCALAR=fixed for fixed-point build"
	@echo "    SKIA_BUILD_FOR=mac for mac build (e.g. CG for image decoding)"
	@echo ""
