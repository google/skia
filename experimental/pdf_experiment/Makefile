# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
all: scanvasmain

CXX := clang++

CXXFLAGS := $(CXXFLAGS) -std=c++14

%.o: %.cpp $(wildcard *.h)
	$(CXX) -O3 -c -g $(CXXFLAGS) @skia.rsp -o $@ $<

S=../../out/default

RSP = "skia-link.rsp"
ifeq ($(shell uname -s),Darwin)
    LD_FLAGS += -Wl,-rpath,@loader_path/. -Wl,-w
    RSP = "skia-link-mac.rsp"
endif

scanvasmain: PDFDocument.o main.o PDFPage.o PDFTypes.o SimpleString.o
	$(CXX) $(LD_FLAGS) $^ @$(RSP) -o $@
clean:
	rm -f *.o scanvasmain

.PHONEY: all clean
