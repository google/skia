all: \
    $(INTERMEDIATE_DIR)/third_party/yasm/x86insn_nasm.c \
    $(INTERMEDIATE_DIR)/third_party/yasm/x86insn_gas.c

$(INTERMEDIATE_DIR)/third_party/yasm/x86insn_nasm.c \
    : \
    $(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86insn_nasm.gperf \
    $(BUILT_PRODUCTS_DIR)/genperf
	@mkdir -p "$(INTERMEDIATE_DIR)/third_party/yasm"
	@echo note: "yasm gperf for $(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86insn_nasm.gperf"
	"$(BUILT_PRODUCTS_DIR)/genperf" "$(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86insn_nasm.gperf" "$(INTERMEDIATE_DIR)/third_party/yasm/x86insn_nasm.c"

$(INTERMEDIATE_DIR)/third_party/yasm/x86insn_gas.c \
    : \
    $(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86insn_gas.gperf \
    $(BUILT_PRODUCTS_DIR)/genperf
	@mkdir -p "$(INTERMEDIATE_DIR)/third_party/yasm"
	@echo note: "yasm gperf for $(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86insn_gas.gperf"
	"$(BUILT_PRODUCTS_DIR)/genperf" "$(SHARED_INTERMEDIATE_DIR)/third_party/yasm/x86insn_gas.gperf" "$(INTERMEDIATE_DIR)/third_party/yasm/x86insn_gas.c"
