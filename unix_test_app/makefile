# Build the unix test app

C_INCLUDES := -I../include/core \
              -I../include/config \
              -I../include/effects \
              -I../include/images \
              -I../include/utils \
              -I../include/views \
              -I../include/xml \
              -I../include/gpu \
              -I../gpu/include \
              -I../include/utils/unix \
              -I../samplecode \
              -I../src/core

VPATH = libs:../src/ports:../samplecode:../src/core:../src/utils/unix

#generate debugging info
CFLAGS = -g

SRC_LIST := main.cpp SkOSWindow_Unix.cpp SkXMLParser_empty.cpp SkDebug.cpp ../gpu/src/unix/GrGLDefaultInterface_unix.cpp

#views files
include ../src/views/views_files.mk
SRC_LIST += $(addprefix ../src/views/, $(SOURCE))

#xml
include ../src/xml/xml_files.mk
SRC_LIST += $(addprefix ../src/xml/, $(SOURCE))

#include the samples
include ../samplecode/samplecode_files.mk
SRC_LIST += $(addprefix ../samplecode/, $(SOURCE))


out/output: $(SRC_LIST) out/keysym2ucs.o ../out/libskia.a -lX11 -lpthread -lz -lfreetype -lGL -lpng
	g++ $(C_INCLUDES) $(CFLAGS) $^ -o $@

out/keysym2ucs.o: ../src/utils/unix/keysym2ucs.c
	@mkdir -p $(dir $@)
	@gcc -c $(C_INCLUDES) $(CFLAGS) $^ -o $@

../out/libskia.a:
	@$(MAKE) -C ../ SKIA_SAMPLES_FOR_X=true

clean:
	rm -rf ../out # Copied from ../Makefile
	rm -rf out
