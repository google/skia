all: \
    $(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86cpu.c \
    $(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86regtmod.c

$(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86cpu.c \
    : \
    ../third_party/externals/yasm/source/patched-yasm/modules/arch/x86/x86cpu.gperf \
    $(BUILT_PRODUCTS_DIR)/genperf
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/yasm"
	@echo note: "yasm genperf for ../third_party/externals/yasm/source/patched-yasm/modules/arch/x86/x86cpu.gperf"
	"$(BUILT_PRODUCTS_DIR)/genperf" "../third_party/externals/yasm/source/patched-yasm/modules/arch/x86/x86cpu.gperf" "$(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86cpu.c"

$(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86regtmod.c \
    : \
    ../third_party/externals/yasm/source/patched-yasm/modules/arch/x86/x86regtmod.gperf \
    $(BUILT_PRODUCTS_DIR)/genperf
	@mkdir -p "$(SHARED_INTERMEDIATE_DIR)/third_party/yasm"
	@echo note: "yasm genperf for ../third_party/externals/yasm/source/patched-yasm/modules/arch/x86/x86regtmod.gperf"
	"$(BUILT_PRODUCTS_DIR)/genperf" "../third_party/externals/yasm/source/patched-yasm/modules/arch/x86/x86regtmod.gperf" "$(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86regtmod.c"
