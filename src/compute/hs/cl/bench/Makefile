#
#
#

SRC_C	=	../../../common/cl/assert_cl.c		\
		../../../common/cl/find_cl.c		\
		../../../common/util.c			\
		../hs_cl.c				\
		../intel/gen8/u32/hs_intel_gen8_u32.c	\
		../intel/gen8/u64/hs_intel_gen8_u64.c 	\
		main.c

SRC_CPP	=	sort.cpp

OBJ_C	= 	$(addsuffix .o,$(basename $(notdir $(SRC_C))))

OBJ_CPP	= 	$(addsuffix .o,$(basename $(notdir $(SRC_CPP))))

#
#
#

hs_bench_cl:	$(OBJ_C) $(OBJ_CPP)
		g++ -o $@ $^ /usr/lib/x86_64-linux-gnu/libOpenCL.so.1

$(OBJ_C):	$(SRC_C)
		gcc -D NDEBUG -D HS_DUMP_SOURCE -c $^ -I ../../.. -I ..

$(OBJ_CPP):	$(SRC_CPP)
		g++ -D NDEBUG -std=c++11        -c $^


.PHONY: clean
clean:
		rm -f $(OBJ_C) $(OBJ_CPP) hs_bench_cl
