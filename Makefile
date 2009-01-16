
CC = gcc
C_INCLUDES := -Iinclude/core -Iinclude/effects -Iinclude/images -Iinclude/utils
CFLAGS = -O2 
DEFINES = -DSK_BUILD_FOR_UNIX
HIDE = @

DEFINES += -DSK_RELEASE
#DEFINES += -DSK_DEBUG -DSK_SUPPORT_UNITTEST
#DEFINES += -DSK_SCALAR_IS_FIXED

# start with the core (required)
include src/core/core_files.mk
SRC_LIST := $(addprefix src/core/, $(SOURCE))

# we usually need ports
include src/ports/ports_files.mk
SRC_LIST += $(addprefix src/ports/, $(SOURCE))

# do we want effects?
include src/effects/effects_files.mk
SRC_LIST += $(addprefix src/effects/, $(SOURCE))

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

BENCH_SRCS := RectBench.cpp SkBenchmark.cpp main.cpp
BENCH_SRCS := $(addprefix bench/, $(BENCH_SRCS))
BENCH_OBJS := $(BENCH_SRCS:.cpp=.o)
BENCH_OBJS := $(addprefix out/, $(BENCH_OBJS))

bench: $(BENCH_OBJS) out/libskia.a
	g++ $(BENCH_OBJS) out/libskia.a -o out/bench/bench -lpthread
	
clean:
	$(HIDE)rm -rf out
    
