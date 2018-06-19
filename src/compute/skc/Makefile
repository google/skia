#
# Copyright 2016 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can
# be found in the LICENSE file.
#

SRC		= block_pool_init.cl paths_copy.cl fills_expand.cl rasterize.cl raster_alloc.cl prefix.cl place.cl render.cl

PRE		= $(SRC:%.cl=%.pre.cl)

IR_GEN9		= $(PRE:%.cl=%.ir)

$(info PRE     : $(PRE))
$(info IR_GEN9 : $(IR_GEN9))

#
#
#

OPENCL_STD	= -cl-std=CL1.2
OPENCL_PRE	= __OPENCL_C_VERSION__=120

# OPENCL_STD	= -cl-std=CL2.0
# OPENCL_PRE	= __OPENCL_C_VERSION__=200

#
#
#

TARGETS		= $(PRE) $(IR_GEN9)

#
#
#

IOC		= ioc64

IOC_IR_OPTS_OPT = $(OPENCL_STD) -cl-single-precision-constant -cl-denorms-are-zero -cl-mad-enable \
		  -cl-no-signed-zeros -cl-fast-relaxed-math -cl-kernel-arg-info

IOC_IR_OPTS_DBG = $(OPENCL_STD) -cl-kernel-arg-info -g

IOC_IR_OPTS     = $(IOC_IR_OPTS_OPT)

#
#
#

PRE_DEPS	= $(wildcard *.h)

#
#
#

all:		$(TARGETS)


clean:
		-rm -f $(TARGETS) $(wildcard *.pre.bin.inl) $(wildcard *.pre.src.inl) $(wildcard *.gen) $(wildcard *.TMP)

#
# PREPROCESS
#

$(PRE):		%.pre.cl: %.cl $(PRE_DEPS)
		cl -I . -I "%INTELOCLSDKROOT%\include" -D $(OPENCL_PRE) -EP $< -P -Fi"$@"
		clang-format -i $@
		dos2unix $@
		xxd -i $@ $(basename $@).src.inl

#
# GEN9 -- supports OpenCL 2.0 and can emit SPIR-V / SPIR-V TEXT but cannot load it via clCreateProgramWithIL()
#

$(IR_GEN9):	%.ir: %.cl
		touch $@
		$(IOC) -cmd=build -bo="$(IOC_IR_OPTS)" -device=gpu -input=$< -ir=$@ -asm
		xxd -i $@ $(basename $@).bin.inl
