# Copyright 2017 Google Inc.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

executable = makegkb
CXXFLAGS += --std=c++11
LDFLAGS += -lpng

%.o : %.cpp $(wildcard *.h) Makefile
	${CXX} ${CPPFLAGS} ${CXXFLAGS} -c $< -o $@
$(executable) : $(patsubst %.cpp,%.o,$(wildcard *.cpp))
	${CXX} ${LDFLAGS} $^ -o $@
clean:
	rm -f $(executable) *.o

