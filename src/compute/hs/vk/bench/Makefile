#
#
#

SRC_C		=	../../../common/util.c			\
			../../../common/vk/assert_vk.c		\
			../../../common/vk/host_alloc.c		\
			../hs_vk.c				\
			../intel/gen8/u32/hs_intel_gen8_u32.c	\
			../intel/gen8/u64/hs_intel_gen8_u64.c 	\
			../nvidia/sm_35/u32/hs_nvidia_sm35_u32.c\
			../nvidia/sm_35/u64/hs_nvidia_sm35_u64.c\
			main.c

SRC_CPP		=	sort.cpp

OBJ_C		= 	$(addsuffix .o,$(basename $(notdir $(SRC_C))))

OBJ_CPP		= 	$(addsuffix .o,$(basename $(notdir $(SRC_CPP))))

#
#
#

GCC_OPT		=	-O2 -D NDEBUG
GPP_OPT		=	-O2 -D NDEBUG -std=c++11

#
#
#

VULKAN_INC	=	$(VULKAN_SDK)/include

#
#
#

hs_bench_vk:	$(OBJ_C) $(OBJ_CPP)
		g++ -o $@ $^ $(VULKAN_SDK)/lib/libvulkan.so.1

$(OBJ_C):	$(SRC_C)
		gcc $(GCC_OPT) -c $^ -I ../../.. -I .. -I $(VULKAN_INC)

$(OBJ_CPP):	$(SRC_CPP)
		g++ $(GPP_OPT) -c $^


.PHONY: clean
clean:
		rm -f $(OBJ_C) $(OBJ_CPP) hs_bench_vk
