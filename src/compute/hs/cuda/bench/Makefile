#
#
#

SRC_C		=	../../../common/util.c			\
			../../../common/cuda/assert_cuda.c	\
			main.c

SRC_CPP		=	sort.cpp				\

SRC_CU		=	../sm_35/u32/hs_cuda_u32.cu		\
			../sm_35/u64/hs_cuda_u64.cu

OBJ_C		= 	$(addsuffix .o,$(basename $(notdir $(SRC_C))))

OBJ_CPP		= 	$(addsuffix .o,$(basename $(notdir $(SRC_CPP))))

#
#
#

NVCC_OPTS	= 	-std=c++11 -res-usage -O2 -D NDEBUG


NVCC_ARCH	=	-gencode arch=compute_37,code=sm_37	\
			-gencode arch=compute_60,code=sm_60	\
			-gencode arch=compute_61,code=sm_61	\
			-gencode arch=compute_70,code=sm_70

# NVCC_ARCH	=	-gencode arch=compute_37,code=sm_37

#
#
#

GCC_OPTS	=	-O2 -D NDEBUG
GPP_OPTS	=	-O2 -D NDEBUG -std=c++11

#
#
#

hs_bench_cuda:	$(OBJ_C) $(OBJ_CPP) $(SRC_CU)
		nvcc $(NVCC_ARCH) $(NVCC_OPTS) -I ../../.. -o $@ $^

$(OBJ_C):	$(SRC_C)
		gcc  $(GCC_OPTS) -c $^ -I ../../.. -I .. -I $(CUDA_HOME)/include

$(OBJ_CPP):	$(SRC_CPP)
		g++  $(GPP_OPTS) -c $^ -I ../../../ -I $(CUDA_HOME)/include


.PHONY: clean
clean:
		rm -f $(OBJ_C) $(OBJ_CPP) hs_bench_cuda
