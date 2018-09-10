
OPENCL_STD	:=	-cl-std=CL1.2
OPENCL_PRE	:=	__OPENCL_C_VERSION__=120

IOC_IR_OPTS_OPT	:=	$(OPENCL_STD) -cl-single-precision-constant -cl-denorms-are-zero \
			-cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math -cl-kernel-arg-info

TARGETS		:=	hs_kernels.src.xxd hs_kernels.src.len.xxd hs_kernels.bin.xxd hs_kernels.bin.len.xxd hs_target.bin

all:			$(TARGETS)

clean:
			rm $(TARGETS) hs_kernels.pre.cl hs_kernels.bin

hs_target.bin:		hs_kernels.bin
			gcc -D HS_DUMP -x c hs_target.h -o hs_dump
			hs_dump

hs_kernels.bin.len.xxd:	hs_kernels.bin
			$(shell bytes=$$(wc -c < $<); printf "%.8x" $$bytes | xxd -r -p | xxd -i 1> $@)

hs_kernels.bin.xxd:	hs_kernels.bin
			$(shell xxd -i < $< 1> $@)

hs_kernels.bin:		hs_kernels.pre.cl
			ioc64 -cmd=build -bo="$(IOC_IR_OPTS_OPT)" -device=gpu -input=$< -ir=$@

hs_kernels.src.len.xxd:	hs_kernels.pre.cl
			$(shell bytes=$$(wc -c < $<); printf "%.8x" $$bytes | xxd -r -p | xxd -i 1> $@)

hs_kernels.src.xxd:	hs_kernels.pre.cl
			$(shell xxd -i < $< 1> $@)

hs_kernels.pre.cl:	hs_kernels.cl
			gcc -x c -E -P -I . -D $(OPENCL_PRE) $< -o $@
