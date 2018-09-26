#
#
#

SRC_H		=	gen.h  networks.h  transpose.h

SRC_C		=	main.c	networks_merging.c  networks_sorting.c	\
			target_cuda.c  target_debug.c  target_glsl.c    \
			target_opencl.c  transpose.c			\
			../../common/util.c

OBJ_C		= 	$(addsuffix .o,$(basename $(notdir $(SRC_C))))

#
#
#

GCC_OPT		=	-O2 -D NDEBUG

#
#
#

hs_gen:		$(OBJ_C)
		gcc -o $@ $^
		rm -f $(OBJ_C)

$(OBJ_C):	$(SRC_C)
		gcc $(GCC_OPT) -c $^ -I ../..

.PHONY: clean
clean:
		rm -f $(OBJ_C) hs_gen
