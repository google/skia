all: \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jccolss2-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcgrass2-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcqnts2f-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcqnts2i-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcsamss2-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jdcolss2-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jdmerss2-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jdsamss2-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jfss2fst-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jfss2int-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jfsseflt-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2flt-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2fst-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2int-64.o \
    $(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2red-64.o

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jccolss2-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jccolss2-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jccolss2-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jccolss2-64.o" "../third_party/externals/libjpeg-turbo/simd/jccolss2-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcgrass2-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jcgrass2-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jcgrass2-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcgrass2-64.o" "../third_party/externals/libjpeg-turbo/simd/jcgrass2-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcqnts2f-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jcqnts2f-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jcqnts2f-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcqnts2f-64.o" "../third_party/externals/libjpeg-turbo/simd/jcqnts2f-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcqnts2i-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jcqnts2i-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jcqnts2i-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcqnts2i-64.o" "../third_party/externals/libjpeg-turbo/simd/jcqnts2i-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcsamss2-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jcsamss2-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jcsamss2-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jcsamss2-64.o" "../third_party/externals/libjpeg-turbo/simd/jcsamss2-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jdcolss2-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jdcolss2-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jdcolss2-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jdcolss2-64.o" "../third_party/externals/libjpeg-turbo/simd/jdcolss2-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jdmerss2-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jdmerss2-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jdmerss2-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jdmerss2-64.o" "../third_party/externals/libjpeg-turbo/simd/jdmerss2-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jdsamss2-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jdsamss2-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jdsamss2-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jdsamss2-64.o" "../third_party/externals/libjpeg-turbo/simd/jdsamss2-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jfss2fst-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jfss2fst-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jfss2fst-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jfss2fst-64.o" "../third_party/externals/libjpeg-turbo/simd/jfss2fst-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jfss2int-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jfss2int-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jfss2int-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jfss2int-64.o" "../third_party/externals/libjpeg-turbo/simd/jfss2int-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jfsseflt-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jfsseflt-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jfsseflt-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jfsseflt-64.o" "../third_party/externals/libjpeg-turbo/simd/jfsseflt-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2flt-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jiss2flt-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jiss2flt-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2flt-64.o" "../third_party/externals/libjpeg-turbo/simd/jiss2flt-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2fst-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jiss2fst-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jiss2fst-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2fst-64.o" "../third_party/externals/libjpeg-turbo/simd/jiss2fst-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2int-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jiss2int-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jiss2int-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2int-64.o" "../third_party/externals/libjpeg-turbo/simd/jiss2int-64.asm"

$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2red-64.o \
    : \
    ../third_party/externals/libjpeg-turbo/simd/jiss2red-64.asm
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo"
	@echo note: "Building jiss2red-64.o"
	"$(BUILT_PRODUCTS_DIR)/yasm" -fmacho64 -D__x86_64__ -DMACHO -I../third_party/externals/libjpeg-turbo/mac/ -DRGBX_FILLER_0XFF -DSTRICT_MEMORY_ACCESS -o "$(SHARED_INTERMEDIATE_DIR)/third_party/externals/libjpeg-turbo/jiss2red-64.o" "../third_party/externals/libjpeg-turbo/simd/jiss2red-64.asm"
