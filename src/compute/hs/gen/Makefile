#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.
#

#BINS	= hotsort_cuda.cubin hotsort_igp.hsw.ir hotsort_igp.bdw.ir
#SRCS	= hotsort_cuda.cu    hotsort_igp.hsw.cl hotsort_igp.bdw.cl

GEN_SRCS	= hs_igp.cl hs_igp.h
GEN_BINS	= hs_igp.ir

#
#
#

all:		$(GEN_BINS)

clean:
		-rm -f $(GEN_SRCS) $(GEN_BINS)

$(GEN_SRCS):
		..\bin\x64\Debug\hs_gen -a 2 -s 32768 -w 8 -t 2 -b 28 -r 16 -f 1 -F 2 -c 1 -C 2 -z
		astyle -ns2 $(addprefix ./, $(GEN_SRCS))


hs_igp.ir:	hs_igp.cl
		ioc32 -cmd=build -input=hs_igp.cl -device=gpu

