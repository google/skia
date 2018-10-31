# Copyright 2018 Google LLC.
# Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
all: scanvasmain

CXX := clang++

CXXFLAGS := $(CXXFLAGS) -std=c++14

%.o: %.cpp $(wildcard *.h)
	$(CXX) -c $(CXXFLAGS) @skia.rsp -o $@ $<

scanvasmain: SCanvas.o main.o Target.o
	$(CXX) $^ @skia-link.rsp -o $@

clean:
	rm -f *.o scanvasmain

.PHONEY: all clean
