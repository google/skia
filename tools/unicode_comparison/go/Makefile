.PHONY: all

SOURCES = ../../../src/core/SkSemaphore.cpp \
          ../../../src/core/SkContainers.cpp \
		  ../../../src/core/SkThreadID.cpp \
          ../../../src/core/SkUTF.cpp \
		  ../../../src/core/SkUtils.cpp \
		  ../../../src/core/SkMalloc.cpp \
		  ../../../src/core/SkSafeMath.cpp \
          ../../../src/core/SkString.cpp \
		  ../../../src/core/SkTime.cpp \
          ../../../src/ports/SkLog_stdio.cpp \
          ../../../src/ports/SkMemory_malloc.cpp \
          ../../../src/core/SkChecksum.cpp

.FORCE:

bridge: .FORCE
	/usr/bin/clang++ -o ../../../out/Release/libbridge.a ../cpp/*.cpp $(SOURCES) -std=c++20 -fvisibility-inlines-hidden -O3 -Wall -Wextra -fPIC -shared -l:libskunicode.a -I../../.. -L../../../out/Release

extract_info: .FORCE bridge
	go build -a -o ../../../out/Release/extract_info extract_info/main.go

download_wiki: .FORCE bridge
	go build -a -o ../../../out/Release/download_wiki download_wiki/main.go

generate_table: .FORCE bridge
	go build -a -o ../../../out/Release/generate_table generate_table/main.go
