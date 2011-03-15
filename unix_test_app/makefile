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
              -I../samplecode

VPATH = libs:../src/ports:../samplecode:../src/core:../out:../src/utils/unix

#generate debugging info
CFLAGS = -g

SRC_LIST := main.cpp SkOSWindow_Unix.cpp SkXMLParser_empty.cpp SkDebug.cpp keysym2ucs.c

#views files
include ../src/views/views_files.mk
SRC_LIST += $(addprefix ../src/views/, $(SOURCE))

#xml
include ../src/xml/xml_files.mk
SRC_LIST += $(addprefix ../src/xml/, $(SOURCE))

#include the samples
include ../samplecode/samplecode_files.mk
SRC_LIST += $(addprefix ../samplecode/, $(SOURCE))

output: $(SRC_LIST) -lX11 -lskia -lpthread -lz -lfreetype -lGL -lpng
	g++ $(C_INCLUDES) $(CFLAGS) $^ -o $@

