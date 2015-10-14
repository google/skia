; Generated from x86id.c by GCC 3.2.3 with -O -S -I. -masm=intel.
; Many changes made to make nasm-syntax compatible.

	;.file	"x86id.c"
;#APP
	;.ident	"$Id$"
;#NO_APP
[extern yasm_internal_error_]
[extern yasm_expr_copy]
[extern yasm_expr_expr]
[extern yasm_expr_new]
[extern yasm_symrec_define_label]
[extern yasm_x86_LTX_mode_bits]
[extern yasm_x86__bc_new_jmp]
[extern yasm_ea_get_disp]
[extern yasm_expr__contains]
[extern yasm_x86__get_reg_size]
[extern yasm__error]
[extern yasm_intnum_new_uint]
[extern yasm_expr_int]
[extern yasm_ea_delete]
[extern yasm_expr_delete]
[extern yasm_x86__ea_new_reg]
[extern yasm_x86__ea_set_disponly]
[extern yasm_x86__ea_new_imm]
[extern yasm_x86__set_rex_from_reg]
[extern yasm_xfree]
[extern yasm_x86__bc_new_insn]
[extern yasm__warning]

	[section .data]
	[align 4]
	;.type	cpu_enabled,@object
	;.size	cpu_enabled,4
cpu_enabled:
	dd	-1
	[section	.rodata]
	[align 4]
	;.type	not64_insn,@object
	;.size	not64_insn,28
not64_insn:
	dd	33554432
	dd	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db 0
	dd	0
	dd	0
	dd	0
	[align 4]
	;.type	onebyte_insn,@object
	;.size	onebyte_insn,28
onebyte_insn:
	dd	0
	dd	80
	db	0
	db	1
	db	0
	db	0
	db	0
	db	0
	db	0
	db 0
	dd	0
	dd	0
	dd	0
	[align 4]
	;.type	twobyte_insn,@object
	;.size	twobyte_insn,28
twobyte_insn:
	dd	0
	dd	20
	db	0
	db	2
	db	0
	db	0
	db	0
	db	0
	db	0
	db 0
	dd	0
	dd	0
	dd	0
	[align 4]
	;.type	threebyte_insn,@object
	;.size	threebyte_insn,28
threebyte_insn:
	dd	0
	dd	21
	db	0
	db	3
	db	0
	db	0
	db	0
	db	0
	db	0
	db 0
	dd	0
	dd	0
	dd	0
	[align 4]
	;.type	onebytemem_insn,@object
	;.size	onebytemem_insn,28
onebytemem_insn:
	dd	0
	dd	48
	db	0
	db	1
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4098
	dd	0
	dd	0
	[align 4]
	;.type	twobytemem_insn,@object
	;.size	twobytemem_insn,28
twobytemem_insn:
	dd	0
	dd	52
	db	0
	db	2
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4098
	dd	0
	dd	0
	[align 32]
	;.type	mov_insn,@object
	;.size	mov_insn,1260
mov_insn:
	dd	0
	dd	0
	db	0
	db	1
	db	-96
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	43
	dd	4405
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-95
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	75
	dd	4437
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-95
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	107
	dd	4469
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-95
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	139
	dd	4501
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-94
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4405
	dd	43
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-93
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4437
	dd	75
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-93
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4469
	dd	107
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-93
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4501
	dd	139
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-120
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4387
	dd	16417
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-119
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	16449
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-119
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	16481
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-119
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4483
	dd	16513
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-118
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16417
	dd	4387
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-117
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4419
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-117
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4451
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-117
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16513
	dd	4483
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-116
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4418
	dd	16710
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-116
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4161
	dd	16710
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-116
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4193
	dd	16710
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-116
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4225
	dd	16710
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-114
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16710
	dd	4419
	dd	0
	dd	4
	dd	0
	db	0
	db	1
	db	-114
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16710
	dd	4193
	dd	0
	dd	16779264
	dd	0
	db	0
	db	1
	db	-114
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16710
	dd	4225
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-80
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	20513
	dd	8480
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-72
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	20545
	dd	8512
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-72
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	20577
	dd	8544
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-72
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	20609
	dd	8576
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-58
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4387
	dd	8224
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-57
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	8256
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-57
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	8288
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-57
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4483
	dd	8288
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-58
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4131
	dd	8480
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-57
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4163
	dd	8512
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-57
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4195
	dd	8544
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-57
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4227
	dd	8544
	dd	0
	dd	41943056
	dd	0
	db	0
	db	2
	db	15
	db	34
	db	0
	db	0
	db	2
	db 0
	dd	16500
	dd	4193
	dd	0
	dd	41943044
	dd	0
	db	0
	db	2
	db	15
	db	34
	db	0
	db	0
	db	2
	db 0
	dd	16487
	dd	4193
	dd	0
	dd	25167872
	dd	0
	db	0
	db	2
	db	15
	db	34
	db	0
	db	0
	db	2
	db 0
	dd	16487
	dd	4225
	dd	0
	dd	41943056
	dd	0
	db	0
	db	2
	db	15
	db	32
	db	0
	db	0
	db	2
	db 0
	dd	4193
	dd	16500
	dd	0
	dd	41943044
	dd	0
	db	0
	db	2
	db	15
	db	32
	db	0
	db	0
	db	2
	db 0
	dd	4193
	dd	16487
	dd	0
	dd	25167872
	dd	0
	db	0
	db	2
	db	15
	db	32
	db	0
	db	0
	db	2
	db 0
	dd	4225
	dd	16487
	dd	0
	dd	41943044
	dd	0
	db	0
	db	2
	db	15
	db	35
	db	0
	db	0
	db	2
	db 0
	dd	16488
	dd	4193
	dd	0
	dd	25167872
	dd	0
	db	0
	db	2
	db	15
	db	35
	db	0
	db	0
	db	2
	db 0
	dd	16488
	dd	4225
	dd	0
	dd	41943044
	dd	0
	db	0
	db	2
	db	15
	db	33
	db	0
	db	0
	db	2
	db 0
	dd	4193
	dd	16488
	dd	0
	dd	25167872
	dd	0
	db	0
	db	2
	db	15
	db	33
	db	0
	db	0
	db	2
	db 0
	dd	4225
	dd	16488
	dd	0
	[align 32]
	;.type	movszx_insn,@object
	;.size	movszx_insn,140
movszx_insn:
	dd	4
	dd	4
	db	16
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4387
	dd	0
	dd	4
	dd	4
	db	32
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4131
	dd	0
	dd	16779264
	dd	4
	db	64
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16513
	dd	4131
	dd	0
	dd	4
	dd	4
	db	32
	db	2
	db	15
	db	1
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4163
	dd	0
	dd	16779264
	dd	4
	db	64
	db	2
	db	15
	db	1
	db	0
	db	0
	db	2
	db 0
	dd	16513
	dd	4163
	dd	0
	[align 4]
	;.type	movsxd_insn,@object
	;.size	movsxd_insn,28
movsxd_insn:
	dd	16779264
	dd	0
	db	64
	db	1
	db	99
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16513
	dd	4195
	dd	0
	[align 32]
	;.type	push_insn,@object
	;.size	push_insn,784
push_insn:
	dd	0
	dd	0
	db	16
	db	1
	db	80
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	20545
	dd	0
	dd	0
	dd	33554436
	dd	0
	db	32
	db	1
	db	80
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	20577
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	0
	db	1
	db	80
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	20609
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-1
	db	0
	db	0
	db	6
	db	1
	db 0
	dd	4163
	dd	0
	dd	0
	dd	33554436
	dd	0
	db	32
	db	1
	db	-1
	db	0
	db	0
	db	6
	db	1
	db 0
	dd	4195
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	0
	db	1
	db	-1
	db	0
	db	0
	db	6
	db	1
	db 0
	dd	4227
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	106
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	8224
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	104
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	8256
	dd	0
	dd	0
	dd	33554436
	dd	0
	db	32
	db	1
	db	104
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	8288
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	104
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	8320
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	0
	db	1
	db	14
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	14
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	16
	db	1
	db	14
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	78
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	32
	db	1
	db	14
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	110
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	0
	db	1
	db	22
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	19
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	16
	db	1
	db	22
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	83
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	32
	db	1
	db	22
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	115
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	0
	db	1
	db	30
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	15
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	16
	db	1
	db	30
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	79
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	32
	db	1
	db	30
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	111
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	0
	db	1
	db	6
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	16
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	16
	db	1
	db	6
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	80
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	32
	db	1
	db	6
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	112
	dd	0
	dd	0
	dd	4
	dd	0
	db	0
	db	2
	db	15
	db	-96
	db	0
	db	0
	db	1
	db 0
	dd	17
	dd	0
	dd	0
	dd	4
	dd	0
	db	16
	db	2
	db	15
	db	-96
	db	0
	db	0
	db	1
	db 0
	dd	81
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	2
	db	15
	db	-96
	db	0
	db	0
	db	1
	db 0
	dd	113
	dd	0
	dd	0
	dd	4
	dd	0
	db	0
	db	2
	db	15
	db	-88
	db	0
	db	0
	db	1
	db 0
	dd	18
	dd	0
	dd	0
	dd	4
	dd	0
	db	16
	db	2
	db	15
	db	-88
	db	0
	db	0
	db	1
	db 0
	dd	82
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	2
	db	15
	db	-88
	db	0
	db	0
	db	1
	db 0
	dd	114
	dd	0
	dd	0
	[align 32]
	;.type	pop_insn,@object
	;.size	pop_insn,588
pop_insn:
	dd	0
	dd	0
	db	16
	db	1
	db	88
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	20545
	dd	0
	dd	0
	dd	33554436
	dd	0
	db	32
	db	1
	db	88
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	20577
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	0
	db	1
	db	88
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	20609
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-113
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4163
	dd	0
	dd	0
	dd	33554436
	dd	0
	db	32
	db	1
	db	-113
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4195
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	0
	db	1
	db	-113
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4227
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	0
	db	1
	db	23
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	19
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	16
	db	1
	db	23
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	83
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	32
	db	1
	db	23
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	115
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	0
	db	1
	db	31
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	15
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	16
	db	1
	db	31
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	79
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	32
	db	1
	db	31
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	111
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	0
	db	1
	db	7
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	16
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	16
	db	1
	db	7
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	80
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	32
	db	1
	db	7
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	112
	dd	0
	dd	0
	dd	4
	dd	0
	db	0
	db	2
	db	15
	db	-95
	db	0
	db	0
	db	1
	db 0
	dd	17
	dd	0
	dd	0
	dd	4
	dd	0
	db	16
	db	2
	db	15
	db	-95
	db	0
	db	0
	db	1
	db 0
	dd	81
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	2
	db	15
	db	-95
	db	0
	db	0
	db	1
	db 0
	dd	113
	dd	0
	dd	0
	dd	4
	dd	0
	db	0
	db	2
	db	15
	db	-87
	db	0
	db	0
	db	1
	db 0
	dd	18
	dd	0
	dd	0
	dd	4
	dd	0
	db	16
	db	2
	db	15
	db	-87
	db	0
	db	0
	db	1
	db 0
	dd	82
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	2
	db	15
	db	-87
	db	0
	db	0
	db	1
	db 0
	dd	114
	dd	0
	dd	0
	[align 32]
	;.type	xchg_insn,@object
	;.size	xchg_insn,392
xchg_insn:
	dd	0
	dd	0
	db	0
	db	1
	db	-122
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4387
	dd	16417
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-122
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16417
	dd	4387
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-112
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	75
	dd	20545
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-112
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	20545
	dd	75
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-121
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	16449
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-121
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4419
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-112
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	107
	dd	20577
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-112
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	20577
	dd	107
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-121
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	16481
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-121
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4451
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-112
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	139
	dd	20609
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-112
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	20609
	dd	139
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-121
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4483
	dd	16513
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-121
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16513
	dd	4483
	dd	0
	[align 32]
	;.type	in_insn,@object
	;.size	in_insn,168
in_insn:
	dd	0
	dd	0
	db	0
	db	1
	db	-28
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	43
	dd	8480
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-27
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	75
	dd	8480
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-27
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	107
	dd	8480
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-20
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	43
	dd	77
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-19
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	75
	dd	77
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-19
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	107
	dd	77
	dd	0
	[align 32]
	;.type	out_insn,@object
	;.size	out_insn,168
out_insn:
	dd	0
	dd	0
	db	0
	db	1
	db	-26
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	8480
	dd	43
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-25
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	8480
	dd	75
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-25
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	8480
	dd	107
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-18
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	77
	dd	43
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-17
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	77
	dd	75
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-17
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	77
	dd	107
	dd	0
	[align 32]
	;.type	lea_insn,@object
	;.size	lea_insn,84
lea_insn:
	dd	0
	dd	0
	db	16
	db	1
	db	-115
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4418
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-115
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4450
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-115
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16513
	dd	4482
	dd	0
	[align 32]
	;.type	ldes_insn,@object
	;.size	ldes_insn,56
ldes_insn:
	dd	33554432
	dd	16
	db	16
	db	1
	db	0
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4098
	dd	0
	dd	33554436
	dd	16
	db	32
	db	1
	db	0
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4098
	dd	0
	[align 32]
	;.type	lfgss_insn,@object
	;.size	lfgss_insn,56
lfgss_insn:
	dd	4
	dd	4
	db	16
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4098
	dd	0
	dd	4
	dd	4
	db	32
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4098
	dd	0
	[align 32]
	;.type	arith_insn,@object
	;.size	arith_insn,644
arith_insn:
	dd	0
	dd	16
	db	0
	db	1
	db	4
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	43
	dd	8480
	dd	0
	dd	0
	dd	16
	db	16
	db	1
	db	5
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	75
	dd	8512
	dd	0
	dd	4
	dd	16
	db	32
	db	1
	db	5
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	107
	dd	8544
	dd	0
	dd	16779264
	dd	16
	db	64
	db	1
	db	5
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	139
	dd	8544
	dd	0
	dd	0
	dd	34
	db	0
	db	1
	db	-128
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4131
	dd	8480
	dd	0
	dd	0
	dd	34
	db	0
	db	1
	db	-128
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4387
	dd	8224
	dd	0
	dd	0
	dd	34
	db	16
	db	1
	db	-125
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4163
	dd	12320
	dd	0
	dd	0
	dd	34
	db	16
	db	1
	db	-127
	db	-125
	db	0
	db	0
	db	2
	db 0
	dd	4163
	dd	139584
	dd	0
	dd	0
	dd	34
	db	16
	db	1
	db	-127
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	8256
	dd	0
	dd	4
	dd	34
	db	32
	db	1
	db	-125
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4195
	dd	12320
	dd	0
	dd	4
	dd	34
	db	32
	db	1
	db	-127
	db	-125
	db	0
	db	0
	db	2
	db 0
	dd	4195
	dd	139616
	dd	0
	dd	4
	dd	34
	db	32
	db	1
	db	-127
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	8288
	dd	0
	dd	16779264
	dd	34
	db	64
	db	1
	db	-125
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4227
	dd	12320
	dd	0
	dd	16779264
	dd	34
	db	64
	db	1
	db	-127
	db	-125
	db	0
	db	0
	db	2
	db 0
	dd	4227
	dd	139616
	dd	0
	dd	16779264
	dd	34
	db	64
	db	1
	db	-127
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4483
	dd	8288
	dd	0
	dd	0
	dd	16
	db	0
	db	1
	db	0
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4387
	dd	16417
	dd	0
	dd	0
	dd	16
	db	16
	db	1
	db	1
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	16449
	dd	0
	dd	4
	dd	16
	db	32
	db	1
	db	1
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	16481
	dd	0
	dd	16779264
	dd	16
	db	64
	db	1
	db	1
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4483
	dd	16513
	dd	0
	dd	0
	dd	16
	db	0
	db	1
	db	2
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16417
	dd	4387
	dd	0
	dd	0
	dd	16
	db	16
	db	1
	db	3
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4419
	dd	0
	dd	4
	dd	16
	db	32
	db	1
	db	3
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4451
	dd	0
	dd	16779264
	dd	16
	db	64
	db	1
	db	3
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16513
	dd	4483
	dd	0
	[align 32]
	;.type	incdec_insn,@object
	;.size	incdec_insn,168
incdec_insn:
	dd	0
	dd	34
	db	0
	db	1
	db	-2
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4131
	dd	0
	dd	0
	dd	33554432
	dd	16
	db	16
	db	1
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	20545
	dd	0
	dd	0
	dd	0
	dd	34
	db	16
	db	1
	db	-1
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4163
	dd	0
	dd	0
	dd	33554436
	dd	16
	db	32
	db	1
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	20577
	dd	0
	dd	0
	dd	4
	dd	34
	db	32
	db	1
	db	-1
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4195
	dd	0
	dd	0
	dd	16779264
	dd	34
	db	64
	db	1
	db	-1
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4227
	dd	0
	dd	0
	[align 32]
	;.type	f6_insn,@object
	;.size	f6_insn,112
f6_insn:
	dd	0
	dd	32
	db	0
	db	1
	db	-10
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4131
	dd	0
	dd	0
	dd	0
	dd	32
	db	16
	db	1
	db	-9
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4163
	dd	0
	dd	0
	dd	4
	dd	32
	db	32
	db	1
	db	-9
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4195
	dd	0
	dd	0
	dd	16779264
	dd	32
	db	64
	db	1
	db	-9
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4227
	dd	0
	dd	0
	[align 32]
	;.type	test_insn,@object
	;.size	test_insn,560
test_insn:
	dd	0
	dd	0
	db	0
	db	1
	db	-88
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	43
	dd	8480
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-87
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	75
	dd	8512
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-87
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	107
	dd	8544
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-87
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	139
	dd	8544
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-10
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4131
	dd	8480
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-10
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4387
	dd	8224
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-9
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4163
	dd	8512
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-9
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	8256
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-9
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4195
	dd	8544
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-9
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	8288
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-9
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4227
	dd	8544
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-9
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4483
	dd	8288
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-124
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4387
	dd	16417
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-123
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	16449
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-123
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	16481
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-123
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4483
	dd	16513
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-124
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16417
	dd	4387
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-123
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4419
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-123
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4451
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-123
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16513
	dd	4483
	dd	0
	[align 32]
	;.type	aadm_insn,@object
	;.size	aadm_insn,56
aadm_insn:
	dd	0
	dd	16
	db	0
	db	2
	db	-44
	db	10
	db	0
	db	0
	db	0
	db 0
	dd	0
	dd	0
	dd	0
	dd	0
	dd	16
	db	0
	db	1
	db	-44
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	8480
	dd	0
	dd	0
	[align 32]
	;.type	imul_insn,@object
	;.size	imul_insn,532
imul_insn:
	dd	0
	dd	0
	db	0
	db	1
	db	-10
	db	0
	db	0
	db	5
	db	1
	db 0
	dd	4131
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-9
	db	0
	db	0
	db	5
	db	1
	db 0
	dd	4163
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-9
	db	0
	db	0
	db	5
	db	1
	db 0
	dd	4195
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	-9
	db	0
	db	0
	db	5
	db	1
	db 0
	dd	4227
	dd	0
	dd	0
	dd	4
	dd	0
	db	16
	db	2
	db	15
	db	-81
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4419
	dd	0
	dd	4
	dd	0
	db	32
	db	2
	db	15
	db	-81
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4451
	dd	0
	dd	16779264
	dd	0
	db	64
	db	2
	db	15
	db	-81
	db	0
	db	0
	db	2
	db 0
	dd	16513
	dd	4483
	dd	0
	dd	1
	dd	0
	db	16
	db	1
	db	107
	db	0
	db	0
	db	0
	db	3
	db 0
	dd	16449
	dd	4419
	dd	12320
	dd	4
	dd	0
	db	32
	db	1
	db	107
	db	0
	db	0
	db	0
	db	3
	db 0
	dd	16481
	dd	4451
	dd	12320
	dd	16779264
	dd	0
	db	64
	db	1
	db	107
	db	0
	db	0
	db	0
	db	3
	db 0
	dd	16513
	dd	4483
	dd	12320
	dd	1
	dd	0
	db	16
	db	1
	db	107
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	28737
	dd	12320
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	107
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	28769
	dd	12320
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	107
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	28801
	dd	12320
	dd	0
	dd	1
	dd	0
	db	16
	db	1
	db	105
	db	107
	db	0
	db	0
	db	3
	db 0
	dd	16449
	dd	4419
	dd	143680
	dd	4
	dd	0
	db	32
	db	1
	db	105
	db	107
	db	0
	db	0
	db	3
	db 0
	dd	16481
	dd	4451
	dd	143712
	dd	16779264
	dd	0
	db	64
	db	1
	db	105
	db	107
	db	0
	db	0
	db	3
	db 0
	dd	16513
	dd	4483
	dd	143712
	dd	1
	dd	0
	db	16
	db	1
	db	105
	db	107
	db	0
	db	0
	db	2
	db 0
	dd	28737
	dd	143680
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	105
	db	107
	db	0
	db	0
	db	2
	db 0
	dd	28769
	dd	143712
	dd	0
	dd	16779264
	dd	0
	db	64
	db	1
	db	105
	db	107
	db	0
	db	0
	db	2
	db 0
	dd	28801
	dd	143712
	dd	0
	[align 32]
	;.type	shift_insn,@object
	;.size	shift_insn,224
shift_insn:
	dd	0
	dd	32
	db	0
	db	1
	db	-46
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4131
	dd	44
	dd	0
	dd	0
	dd	32
	db	0
	db	1
	db	-64
	db	-48
	db	0
	db	0
	db	2
	db 0
	dd	4131
	dd	74016
	dd	0
	dd	0
	dd	32
	db	16
	db	1
	db	-45
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4163
	dd	44
	dd	0
	dd	0
	dd	32
	db	16
	db	1
	db	-63
	db	-47
	db	0
	db	0
	db	2
	db 0
	dd	4163
	dd	74016
	dd	0
	dd	0
	dd	32
	db	32
	db	1
	db	-45
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4195
	dd	44
	dd	0
	dd	0
	dd	32
	db	32
	db	1
	db	-63
	db	-47
	db	0
	db	0
	db	2
	db 0
	dd	4195
	dd	74016
	dd	0
	dd	16779264
	dd	32
	db	64
	db	1
	db	-45
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4227
	dd	44
	dd	0
	dd	16779264
	dd	32
	db	64
	db	1
	db	-63
	db	-47
	db	0
	db	0
	db	2
	db 0
	dd	4227
	dd	74016
	dd	0
	[align 32]
	;.type	shlrd_insn,@object
	;.size	shlrd_insn,168
shlrd_insn:
	dd	4
	dd	4
	db	16
	db	2
	db	15
	db	0
	db	0
	db	0
	db	3
	db 0
	dd	4419
	dd	16449
	dd	8480
	dd	4
	dd	4
	db	16
	db	2
	db	15
	db	1
	db	0
	db	0
	db	3
	db 0
	dd	4419
	dd	16449
	dd	44
	dd	4
	dd	4
	db	32
	db	2
	db	15
	db	0
	db	0
	db	0
	db	3
	db 0
	dd	4451
	dd	16481
	dd	8480
	dd	4
	dd	4
	db	32
	db	2
	db	15
	db	1
	db	0
	db	0
	db	3
	db 0
	dd	4451
	dd	16481
	dd	44
	dd	16779264
	dd	4
	db	64
	db	2
	db	15
	db	0
	db	0
	db	0
	db	3
	db 0
	dd	4483
	dd	16513
	dd	8480
	dd	16779264
	dd	4
	db	64
	db	2
	db	15
	db	1
	db	0
	db	0
	db	3
	db 0
	dd	4483
	dd	16513
	dd	44
	[align 32]
	;.type	call_insn,@object
	;.size	call_insn,560
call_insn:
	dd	0
	dd	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	32768
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	0
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	32832
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	0
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	32864
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-24
	db	-102
	db	0
	db	0
	db	1
	db 0
	dd	229952
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-24
	db	-102
	db	0
	db	0
	db	1
	db 0
	dd	229984
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-24
	db	-102
	db	0
	db	0
	db	1
	db 0
	dd	229888
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-1
	db	0
	db	0
	db	2
	db	1
	db 0
	dd	4163
	dd	0
	dd	0
	dd	33554436
	dd	0
	db	32
	db	1
	db	-1
	db	0
	db	0
	db	2
	db	1
	db 0
	dd	4195
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	0
	db	1
	db	-1
	db	0
	db	0
	db	2
	db	1
	db 0
	dd	4227
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-1
	db	0
	db	0
	db	2
	db	1
	db 0
	dd	4098
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-1
	db	0
	db	0
	db	2
	db	1
	db 0
	dd	4675
	dd	0
	dd	0
	dd	33554436
	dd	0
	db	32
	db	1
	db	-1
	db	0
	db	0
	db	2
	db	1
	db 0
	dd	4707
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	0
	db	1
	db	-1
	db	0
	db	0
	db	2
	db	1
	db 0
	dd	4739
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-1
	db	0
	db	0
	db	2
	db	1
	db 0
	dd	4610
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-102
	db	0
	db	0
	db	3
	db	1
	db 0
	dd	34368
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-102
	db	0
	db	0
	db	3
	db	1
	db 0
	dd	34400
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-102
	db	0
	db	0
	db	3
	db	1
	db 0
	dd	34304
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-1
	db	0
	db	0
	db	3
	db	1
	db 0
	dd	5698
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-1
	db	0
	db	0
	db	3
	db	1
	db 0
	dd	5730
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-1
	db	0
	db	0
	db	3
	db	1
	db 0
	dd	5634
	dd	0
	dd	0
	[align 32]
	;.type	jmp_insn,@object
	;.size	jmp_insn,588
jmp_insn:
	dd	0
	dd	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	32768
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	0
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	32832
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	32864
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-21
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	33792
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-23
	db	-22
	db	0
	db	0
	db	1
	db 0
	dd	229952
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-23
	db	-22
	db	0
	db	0
	db	1
	db 0
	dd	229984
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-23
	db	-22
	db	0
	db	0
	db	1
	db 0
	dd	229888
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-1
	db	0
	db	0
	db	4
	db	1
	db 0
	dd	4163
	dd	0
	dd	0
	dd	33554436
	dd	0
	db	32
	db	1
	db	-1
	db	0
	db	0
	db	4
	db	1
	db 0
	dd	4195
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	0
	db	1
	db	-1
	db	0
	db	0
	db	4
	db	1
	db 0
	dd	4227
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-1
	db	0
	db	0
	db	4
	db	1
	db 0
	dd	4098
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-1
	db	0
	db	0
	db	4
	db	1
	db 0
	dd	4675
	dd	0
	dd	0
	dd	33554436
	dd	0
	db	32
	db	1
	db	-1
	db	0
	db	0
	db	4
	db	1
	db 0
	dd	4707
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	0
	db	1
	db	-1
	db	0
	db	0
	db	4
	db	1
	db 0
	dd	4739
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-1
	db	0
	db	0
	db	4
	db	1
	db 0
	dd	4610
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-22
	db	0
	db	0
	db	3
	db	1
	db 0
	dd	34368
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-22
	db	0
	db	0
	db	3
	db	1
	db 0
	dd	34400
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-22
	db	0
	db	0
	db	3
	db	1
	db 0
	dd	34304
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	1
	db	-1
	db	0
	db	0
	db	5
	db	1
	db 0
	dd	5698
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	-1
	db	0
	db	0
	db	5
	db	1
	db 0
	dd	5730
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
	db	1
	db	-1
	db	0
	db	0
	db	5
	db	1
	db 0
	dd	5634
	dd	0
	dd	0
	[align 32]
	;.type	retnf_insn,@object
	;.size	retnf_insn,56
retnf_insn:
	dd	0
	dd	16
	db	0
	db	1
	db	1
	db	0
	db	0
	db	0
	db	0
	db 0
	dd	0
	dd	0
	dd	0
	dd	0
	dd	16
	db	0
	db	1
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	8512
	dd	0
	dd	0
	[align 4]
	;.type	enter_insn,@object
	;.size	enter_insn,28
enter_insn:
	dd	1
	dd	0
	db	0
	db	1
	db	-56
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4416
	dd	8480
	dd	0
	[align 32]
	;.type	jcc_insn,@object
	;.size	jcc_insn,196
jcc_insn:
	dd	0
	dd	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	32768
	dd	0
	dd	0
	dd	0
	dd	0
	db	16
	db	0
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	32832
	dd	0
	dd	0
	dd	4
	dd	0
	db	32
	db	0
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	32864
	dd	0
	dd	0
	dd	0
	dd	16
	db	0
	db	1
	db	112
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	33792
	dd	0
	dd	0
	dd	4
	dd	4
	db	16
	db	2
	db	15
	db	-128
	db	0
	db	0
	db	1
	db 0
	dd	33344
	dd	0
	dd	0
	dd	4
	dd	4
	db	32
	db	2
	db	15
	db	-128
	db	0
	db	0
	db	1
	db 0
	dd	33376
	dd	0
	dd	0
	dd	4
	dd	4
	db	0
	db	2
	db	15
	db	-128
	db	0
	db	0
	db	1
	db 0
	dd	33280
	dd	0
	dd	0
	[align 32]
	;.type	jcxz_insn,@object
	;.size	jcxz_insn,56
jcxz_insn:
	dd	0
	dd	256
	db	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	32768
	dd	0
	dd	0
	dd	0
	dd	256
	db	0
	db	1
	db	-29
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	33792
	dd	0
	dd	0
	[align 32]
	;.type	loop_insn,@object
	;.size	loop_insn,224
loop_insn:
	dd	0
	dd	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	32768
	dd	0
	dd	0
	dd	33554432
	dd	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	32768
	dd	36940
	dd	0
	dd	4
	dd	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	32768
	dd	36972
	dd	0
	dd	16779264
	dd	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	32768
	dd	37004
	dd	0
	dd	33554432
	dd	16
	db	0
	db	1
	db	-32
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	33792
	dd	0
	dd	0
	dd	0
	dd	16
	db	0
	db	1
	db	-32
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	33792
	dd	36940
	dd	0
	dd	4
	dd	16
	db	0
	db	1
	db	-32
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	33792
	dd	36972
	dd	0
	dd	16779264
	dd	16
	db	0
	db	1
	db	-32
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	33792
	dd	37004
	dd	0
	[align 4]
	;.type	setcc_insn,@object
	;.size	setcc_insn,28
setcc_insn:
	dd	4
	dd	4
	db	0
	db	2
	db	15
	db	-112
	db	0
	db	2
	db	1
	db 0
	dd	4387
	dd	0
	dd	0
	[align 32]
	;.type	bittest_insn,@object
	;.size	bittest_insn,168
bittest_insn:
	dd	4
	dd	4
	db	16
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	16449
	dd	0
	dd	4
	dd	4
	db	32
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	16481
	dd	0
	dd	16779264
	dd	4
	db	64
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4483
	dd	16513
	dd	0
	dd	4
	dd	34
	db	16
	db	2
	db	15
	db	-70
	db	0
	db	0
	db	2
	db 0
	dd	4163
	dd	8224
	dd	0
	dd	4
	dd	34
	db	32
	db	2
	db	15
	db	-70
	db	0
	db	0
	db	2
	db 0
	dd	4195
	dd	8224
	dd	0
	dd	16779264
	dd	34
	db	64
	db	2
	db	15
	db	-70
	db	0
	db	0
	db	2
	db 0
	dd	4227
	dd	8224
	dd	0
	[align 32]
	;.type	bsfr_insn,@object
	;.size	bsfr_insn,84
bsfr_insn:
	dd	2
	dd	4
	db	16
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4419
	dd	0
	dd	4
	dd	4
	db	32
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4451
	dd	0
	dd	16779264
	dd	4
	db	64
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16513
	dd	4483
	dd	0
	[align 4]
	;.type	int_insn,@object
	;.size	int_insn,28
int_insn:
	dd	0
	dd	0
	db	0
	db	1
	db	-51
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	8480
	dd	0
	dd	0
	[align 32]
	;.type	bound_insn,@object
	;.size	bound_insn,56
bound_insn:
	dd	1
	dd	0
	db	16
	db	1
	db	98
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4418
	dd	0
	dd	4
	dd	0
	db	32
	db	1
	db	98
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4450
	dd	0
	[align 4]
	;.type	arpl_insn,@object
	;.size	arpl_insn,28
arpl_insn:
	dd	1048578
	dd	0
	db	0
	db	1
	db	99
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	16449
	dd	0
	[align 32]
	;.type	str_insn,@object
	;.size	str_insn,112
str_insn:
	dd	2048
	dd	0
	db	16
	db	2
	db	15
	db	0
	db	0
	db	1
	db	1
	db 0
	dd	4161
	dd	0
	dd	0
	dd	2048
	dd	0
	db	32
	db	2
	db	15
	db	0
	db	0
	db	1
	db	1
	db 0
	dd	4193
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	64
	db	2
	db	15
	db	0
	db	0
	db	1
	db	1
	db 0
	dd	4225
	dd	0
	dd	0
	dd	2
	dd	36
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4419
	dd	0
	dd	0
	[align 4]
	;.type	prot286_insn,@object
	;.size	prot286_insn,28
prot286_insn:
	dd	2
	dd	36
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4419
	dd	0
	dd	0
	[align 32]
	;.type	sldtmsw_insn,@object
	;.size	sldtmsw_insn,168
sldtmsw_insn:
	dd	2
	dd	36
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4418
	dd	0
	dd	0
	dd	4
	dd	36
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4450
	dd	0
	dd	0
	dd	16779264
	dd	36
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4482
	dd	0
	dd	0
	dd	2
	dd	36
	db	16
	db	2
	db	15
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4161
	dd	0
	dd	0
	dd	4
	dd	36
	db	32
	db	2
	db	15
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4193
	dd	0
	dd	0
	dd	16779264
	dd	36
	db	64
	db	2
	db	15
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4225
	dd	0
	dd	0
	[align 32]
	;.type	fldstp_insn,@object
	;.size	fldstp_insn,112
fldstp_insn:
	dd	4096
	dd	34
	db	0
	db	1
	db	-39
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4194
	dd	0
	dd	0
	dd	4096
	dd	34
	db	0
	db	1
	db	-35
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4226
	dd	0
	dd	0
	dd	4096
	dd	42
	db	0
	db	1
	db	-37
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4258
	dd	0
	dd	0
	dd	4096
	dd	4
	db	0
	db	2
	db	-39
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	24737
	dd	0
	dd	0
	[align 32]
	;.type	fildstp_insn,@object
	;.size	fildstp_insn,84
fildstp_insn:
	dd	4096
	dd	32
	db	0
	db	1
	db	-33
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4162
	dd	0
	dd	0
	dd	4096
	dd	32
	db	0
	db	1
	db	-37
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4194
	dd	0
	dd	0
	dd	4096
	dd	34
	db	0
	db	1
	db	-33
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4226
	dd	0
	dd	0
	[align 4]
	;.type	fbldstp_insn,@object
	;.size	fbldstp_insn,28
fbldstp_insn:
	dd	4096
	dd	32
	db	0
	db	1
	db	-33
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4514
	dd	0
	dd	0
	[align 32]
	;.type	fst_insn,@object
	;.size	fst_insn,84
fst_insn:
	dd	4096
	dd	0
	db	0
	db	1
	db	-39
	db	0
	db	0
	db	2
	db	1
	db 0
	dd	4194
	dd	0
	dd	0
	dd	4096
	dd	0
	db	0
	db	1
	db	-35
	db	0
	db	0
	db	2
	db	1
	db 0
	dd	4226
	dd	0
	dd	0
	dd	4096
	dd	0
	db	0
	db	2
	db	-35
	db	-48
	db	0
	db	0
	db	1
	db 0
	dd	24737
	dd	0
	dd	0
	[align 32]
	;.type	fxch_insn,@object
	;.size	fxch_insn,112
fxch_insn:
	dd	4096
	dd	0
	db	0
	db	2
	db	-39
	db	-56
	db	0
	db	0
	db	1
	db 0
	dd	24737
	dd	0
	dd	0
	dd	4096
	dd	0
	db	0
	db	2
	db	-39
	db	-56
	db	0
	db	0
	db	2
	db 0
	dd	170
	dd	24737
	dd	0
	dd	4096
	dd	0
	db	0
	db	2
	db	-39
	db	-56
	db	0
	db	0
	db	2
	db 0
	dd	24737
	dd	170
	dd	0
	dd	4096
	dd	0
	db	0
	db	2
	db	-39
	db	-55
	db	0
	db	0
	db	0
	db 0
	dd	0
	dd	0
	dd	0
	[align 32]
	;.type	fcom_insn,@object
	;.size	fcom_insn,112
fcom_insn:
	dd	4096
	dd	34
	db	0
	db	1
	db	-40
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4194
	dd	0
	dd	0
	dd	4096
	dd	34
	db	0
	db	1
	db	-36
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4226
	dd	0
	dd	0
	dd	4096
	dd	4
	db	0
	db	2
	db	-40
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	24737
	dd	0
	dd	0
	dd	4096
	dd	4
	db	0
	db	2
	db	-40
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	170
	dd	24737
	dd	0
	[align 32]
	;.type	fcom2_insn,@object
	;.size	fcom2_insn,56
fcom2_insn:
	dd	4098
	dd	20
	db	0
	db	2
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	24737
	dd	0
	dd	0
	dd	4098
	dd	20
	db	0
	db	2
	db	0
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	170
	dd	24737
	dd	0
	[align 32]
	;.type	farith_insn,@object
	;.size	farith_insn,168
farith_insn:
	dd	4096
	dd	42
	db	0
	db	1
	db	-40
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4194
	dd	0
	dd	0
	dd	4096
	dd	42
	db	0
	db	1
	db	-36
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4226
	dd	0
	dd	0
	dd	4096
	dd	6
	db	0
	db	2
	db	-40
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	24737
	dd	0
	dd	0
	dd	4096
	dd	6
	db	0
	db	2
	db	-40
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	170
	dd	24737
	dd	0
	dd	4096
	dd	4
	db	0
	db	2
	db	-36
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	26785
	dd	0
	dd	0
	dd	4096
	dd	4
	db	0
	db	2
	db	-36
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	24737
	dd	170
	dd	0
	[align 32]
	;.type	farithp_insn,@object
	;.size	farithp_insn,84
farithp_insn:
	dd	4096
	dd	4
	db	0
	db	2
	db	-34
	db	1
	db	0
	db	0
	db	0
	db 0
	dd	0
	dd	0
	dd	0
	dd	4096
	dd	4
	db	0
	db	2
	db	-34
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	24737
	dd	0
	dd	0
	dd	4096
	dd	4
	db	0
	db	2
	db	-34
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	24737
	dd	170
	dd	0
	[align 32]
	;.type	fiarith_insn,@object
	;.size	fiarith_insn,56
fiarith_insn:
	dd	4096
	dd	48
	db	0
	db	1
	db	4
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4162
	dd	0
	dd	0
	dd	4096
	dd	48
	db	0
	db	1
	db	0
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4194
	dd	0
	dd	0
	[align 4]
	;.type	fldnstcw_insn,@object
	;.size	fldnstcw_insn,28
fldnstcw_insn:
	dd	4096
	dd	32
	db	0
	db	1
	db	-39
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4418
	dd	0
	dd	0
	[align 4]
	;.type	fstcw_insn,@object
	;.size	fstcw_insn,28
fstcw_insn:
	dd	4096
	dd	0
	db	0
	db	2
	db	-101
	db	-39
	db	0
	db	7
	db	1
	db 0
	dd	4418
	dd	0
	dd	0
	[align 32]
	;.type	fnstsw_insn,@object
	;.size	fnstsw_insn,56
fnstsw_insn:
	dd	4096
	dd	0
	db	0
	db	1
	db	-35
	db	0
	db	0
	db	7
	db	1
	db 0
	dd	4418
	dd	0
	dd	0
	dd	4096
	dd	0
	db	0
	db	2
	db	-33
	db	-32
	db	0
	db	0
	db	1
	db 0
	dd	75
	dd	0
	dd	0
	[align 32]
	;.type	fstsw_insn,@object
	;.size	fstsw_insn,56
fstsw_insn:
	dd	4096
	dd	0
	db	0
	db	2
	db	-101
	db	-35
	db	0
	db	7
	db	1
	db 0
	dd	4418
	dd	0
	dd	0
	dd	4096
	dd	0
	db	0
	db	3
	db	-101
	db	-33
	db	-32
	db	0
	db	1
	db 0
	dd	75
	dd	0
	dd	0
	[align 4]
	;.type	ffree_insn,@object
	;.size	ffree_insn,28
ffree_insn:
	dd	4096
	dd	16
	db	0
	db	2
	db	0
	db	-64
	db	0
	db	0
	db	1
	db 0
	dd	24737
	dd	0
	dd	0
	[align 32]
	;.type	bswap_insn,@object
	;.size	bswap_insn,56
bswap_insn:
	dd	8
	dd	0
	db	32
	db	2
	db	15
	db	-56
	db	0
	db	0
	db	1
	db 0
	dd	24673
	dd	0
	dd	0
	dd	16779264
	dd	0
	db	64
	db	2
	db	15
	db	-56
	db	0
	db	0
	db	1
	db 0
	dd	24705
	dd	0
	dd	0
	[align 32]
	;.type	cmpxchgxadd_insn,@object
	;.size	cmpxchgxadd_insn,112
cmpxchgxadd_insn:
	dd	8
	dd	4
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4387
	dd	16417
	dd	0
	dd	8
	dd	4
	db	16
	db	2
	db	15
	db	1
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	16449
	dd	0
	dd	8
	dd	4
	db	32
	db	2
	db	15
	db	1
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	16481
	dd	0
	dd	16779264
	dd	4
	db	64
	db	2
	db	15
	db	1
	db	0
	db	0
	db	2
	db 0
	dd	4483
	dd	16513
	dd	0
	[align 4]
	;.type	cmpxchg8b_insn,@object
	;.size	cmpxchg8b_insn,28
cmpxchg8b_insn:
	dd	16
	dd	0
	db	0
	db	2
	db	15
	db	-57
	db	0
	db	1
	db	1
	db 0
	dd	4482
	dd	0
	dd	0
	[align 32]
	;.type	cmovcc_insn,@object
	;.size	cmovcc_insn,84
cmovcc_insn:
	dd	32
	dd	4
	db	16
	db	2
	db	15
	db	64
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4419
	dd	0
	dd	32
	dd	4
	db	32
	db	2
	db	15
	db	64
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4451
	dd	0
	dd	16779264
	dd	4
	db	64
	db	2
	db	15
	db	64
	db	0
	db	0
	db	2
	db 0
	dd	16513
	dd	4483
	dd	0
	[align 4]
	;.type	fcmovcc_insn,@object
	;.size	fcmovcc_insn,28
fcmovcc_insn:
	dd	4128
	dd	20
	db	0
	db	2
	db	0
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	170
	dd	24737
	dd	0
	[align 32]
	;.type	movnti_insn,@object
	;.size	movnti_insn,56
movnti_insn:
	dd	128
	dd	0
	db	0
	db	2
	db	15
	db	-61
	db	0
	db	0
	db	2
	db 0
	dd	4450
	dd	16481
	dd	0
	dd	16779264
	dd	0
	db	64
	db	2
	db	15
	db	-61
	db	0
	db	0
	db	2
	db 0
	dd	4482
	dd	16513
	dd	0
	[align 4]
	;.type	clflush_insn,@object
	;.size	clflush_insn,28
clflush_insn:
	dd	64
	dd	0
	db	0
	db	2
	db	15
	db	-82
	db	0
	db	7
	db	1
	db 0
	dd	4386
	dd	0
	dd	0
	[align 32]
	;.type	movd_insn,@object
	;.size	movd_insn,224
movd_insn:
	dd	8192
	dd	0
	db	0
	db	2
	db	15
	db	110
	db	0
	db	0
	db	2
	db 0
	dd	16516
	dd	4451
	dd	0
	dd	16787456
	dd	0
	db	64
	db	2
	db	15
	db	110
	db	0
	db	0
	db	2
	db 0
	dd	16516
	dd	4483
	dd	0
	dd	8192
	dd	0
	db	0
	db	2
	db	15
	db	126
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	16516
	dd	0
	dd	16787456
	dd	0
	db	64
	db	2
	db	15
	db	126
	db	0
	db	0
	db	2
	db 0
	dd	4483
	dd	16516
	dd	0
	dd	32768
	dd	0
	db	0
	db	3
	db	102
	db	15
	db	110
	db	0
	db	2
	db 0
	dd	16580
	dd	4451
	dd	0
	dd	16812032
	dd	0
	db	64
	db	3
	db	102
	db	15
	db	110
	db	0
	db	2
	db 0
	dd	16580
	dd	4483
	dd	0
	dd	32768
	dd	0
	db	0
	db	3
	db	102
	db	15
	db	126
	db	0
	db	2
	db 0
	dd	4451
	dd	16580
	dd	0
	dd	16812032
	dd	0
	db	64
	db	3
	db	102
	db	15
	db	126
	db	0
	db	2
	db 0
	dd	4483
	dd	16580
	dd	0
	[align 32]
	;.type	movq_insn,@object
	;.size	movq_insn,140
movq_insn:
	dd	8192
	dd	0
	db	0
	db	2
	db	15
	db	111
	db	0
	db	0
	db	2
	db 0
	dd	16516
	dd	4485
	dd	0
	dd	8192
	dd	0
	db	0
	db	2
	db	15
	db	127
	db	0
	db	0
	db	2
	db 0
	dd	4485
	dd	16516
	dd	0
	dd	32768
	dd	0
	db	0
	db	3
	db	-13
	db	15
	db	126
	db	0
	db	2
	db 0
	dd	16580
	dd	4292
	dd	0
	dd	32768
	dd	0
	db	0
	db	3
	db	-13
	db	15
	db	126
	db	0
	db	2
	db 0
	dd	16580
	dd	4485
	dd	0
	dd	32768
	dd	0
	db	0
	db	3
	db	102
	db	15
	db	-42
	db	0
	db	2
	db 0
	dd	4485
	dd	16580
	dd	0
	[align 32]
	;.type	mmxsse2_insn,@object
	;.size	mmxsse2_insn,56
mmxsse2_insn:
	dd	8192
	dd	4
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16516
	dd	4485
	dd	0
	dd	32768
	dd	1
	db	0
	db	3
	db	102
	db	15
	db	0
	db	0
	db	2
	db 0
	dd	16580
	dd	4549
	dd	0
	[align 32]
	;.type	pshift_insn,@object
	;.size	pshift_insn,112
pshift_insn:
	dd	8192
	dd	4
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16516
	dd	4485
	dd	0
	dd	8192
	dd	38
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	4228
	dd	8480
	dd	0
	dd	32768
	dd	1
	db	0
	db	3
	db	102
	db	15
	db	0
	db	0
	db	2
	db 0
	dd	16580
	dd	4549
	dd	0
	dd	32768
	dd	35
	db	0
	db	3
	db	102
	db	15
	db	0
	db	0
	db	2
	db 0
	dd	4292
	dd	8480
	dd	0
	[align 4]
	;.type	sseps_insn,@object
	;.size	sseps_insn,28
sseps_insn:
	dd	16384
	dd	4
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16580
	dd	4549
	dd	0
	[align 4]
	;.type	ssess_insn,@object
	;.size	ssess_insn,28
ssess_insn:
	dd	16384
	dd	17
	db	0
	db	3
	db	0
	db	15
	db	0
	db	0
	db	2
	db 0
	dd	16580
	dd	4549
	dd	0
	[align 4]
	;.type	ssecmpps_insn,@object
	;.size	ssecmpps_insn,28
ssecmpps_insn:
	dd	16384
	dd	128
	db	0
	db	2
	db	15
	db	-62
	db	0
	db	0
	db	2
	db 0
	dd	16580
	dd	4549
	dd	0
	[align 4]
	;.type	ssecmpss_insn,@object
	;.size	ssecmpss_insn,28
ssecmpss_insn:
	dd	16384
	dd	144
	db	0
	db	3
	db	0
	db	15
	db	-62
	db	0
	db	2
	db 0
	dd	16580
	dd	4549
	dd	0
	[align 4]
	;.type	ssepsimm_insn,@object
	;.size	ssepsimm_insn,28
ssepsimm_insn:
	dd	16384
	dd	4
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	3
	db 0
	dd	16580
	dd	4549
	dd	8480
	[align 4]
	;.type	ssessimm_insn,@object
	;.size	ssessimm_insn,28
ssessimm_insn:
	dd	16384
	dd	17
	db	0
	db	3
	db	0
	db	15
	db	0
	db	0
	db	3
	db 0
	dd	16580
	dd	4549
	dd	8480
	[align 4]
	;.type	ldstmxcsr_insn,@object
	;.size	ldstmxcsr_insn,28
ldstmxcsr_insn:
	dd	16384
	dd	32
	db	0
	db	2
	db	15
	db	-82
	db	0
	db	0
	db	1
	db 0
	dd	4450
	dd	0
	dd	0
	[align 4]
	;.type	maskmovq_insn,@object
	;.size	maskmovq_insn,28
maskmovq_insn:
	dd	8256
	dd	0
	db	0
	db	2
	db	15
	db	-9
	db	0
	db	0
	db	2
	db 0
	dd	16516
	dd	4228
	dd	0
	[align 32]
	;.type	movaups_insn,@object
	;.size	movaups_insn,56
movaups_insn:
	dd	16384
	dd	4
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16580
	dd	4549
	dd	0
	dd	16384
	dd	4
	db	0
	db	2
	db	15
	db	1
	db	0
	db	0
	db	2
	db 0
	dd	4549
	dd	16580
	dd	0
	[align 4]
	;.type	movhllhps_insn,@object
	;.size	movhllhps_insn,28
movhllhps_insn:
	dd	16384
	dd	4
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16580
	dd	4292
	dd	0
	[align 32]
	;.type	movhlps_insn,@object
	;.size	movhlps_insn,56
movhlps_insn:
	dd	16384
	dd	4
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16580
	dd	4482
	dd	0
	dd	16384
	dd	4
	db	0
	db	2
	db	15
	db	1
	db	0
	db	0
	db	2
	db 0
	dd	4482
	dd	16580
	dd	0
	[align 4]
	;.type	movmskps_insn,@object
	;.size	movmskps_insn,28
movmskps_insn:
	dd	16384
	dd	0
	db	0
	db	2
	db	15
	db	80
	db	0
	db	0
	db	2
	db 0
	dd	4193
	dd	16580
	dd	0
	[align 4]
	;.type	movntps_insn,@object
	;.size	movntps_insn,28
movntps_insn:
	dd	16384
	dd	0
	db	0
	db	2
	db	15
	db	43
	db	0
	db	0
	db	2
	db 0
	dd	4546
	dd	16577
	dd	0
	[align 4]
	;.type	movntq_insn,@object
	;.size	movntq_insn,28
movntq_insn:
	dd	16384
	dd	0
	db	0
	db	2
	db	15
	db	-25
	db	0
	db	0
	db	2
	db 0
	dd	4482
	dd	16580
	dd	0
	[align 32]
	;.type	movss_insn,@object
	;.size	movss_insn,84
movss_insn:
	dd	16384
	dd	0
	db	0
	db	3
	db	-13
	db	15
	db	16
	db	0
	db	2
	db 0
	dd	16580
	dd	4292
	dd	0
	dd	16384
	dd	0
	db	0
	db	3
	db	-13
	db	15
	db	16
	db	0
	db	2
	db 0
	dd	16580
	dd	4482
	dd	0
	dd	16384
	dd	0
	db	0
	db	3
	db	-13
	db	15
	db	17
	db	0
	db	2
	db 0
	dd	4482
	dd	16580
	dd	0
	[align 32]
	;.type	pextrw_insn,@object
	;.size	pextrw_insn,56
pextrw_insn:
	dd	8256
	dd	0
	db	0
	db	2
	db	15
	db	-59
	db	0
	db	0
	db	3
	db 0
	dd	4193
	dd	16516
	dd	8480
	dd	32768
	dd	0
	db	0
	db	3
	db	102
	db	15
	db	-59
	db	0
	db	3
	db 0
	dd	4193
	dd	16580
	dd	8480
	[align 32]
	;.type	pinsrw_insn,@object
	;.size	pinsrw_insn,112
pinsrw_insn:
	dd	8256
	dd	0
	db	0
	db	2
	db	15
	db	-60
	db	0
	db	0
	db	3
	db 0
	dd	16516
	dd	4193
	dd	8480
	dd	8256
	dd	0
	db	0
	db	2
	db	15
	db	-60
	db	0
	db	0
	db	3
	db 0
	dd	16516
	dd	4419
	dd	8480
	dd	32768
	dd	0
	db	0
	db	3
	db	102
	db	15
	db	-60
	db	0
	db	3
	db 0
	dd	16580
	dd	4193
	dd	8480
	dd	32768
	dd	0
	db	0
	db	3
	db	102
	db	15
	db	-60
	db	0
	db	3
	db 0
	dd	16516
	dd	4419
	dd	8480
	[align 32]
	;.type	pmovmskb_insn,@object
	;.size	pmovmskb_insn,56
pmovmskb_insn:
	dd	8256
	dd	0
	db	0
	db	2
	db	15
	db	-41
	db	0
	db	0
	db	2
	db 0
	dd	4193
	dd	16516
	dd	0
	dd	32768
	dd	0
	db	0
	db	3
	db	102
	db	15
	db	-41
	db	0
	db	2
	db 0
	dd	4193
	dd	16580
	dd	0
	[align 4]
	;.type	pshufw_insn,@object
	;.size	pshufw_insn,28
pshufw_insn:
	dd	8256
	dd	0
	db	0
	db	2
	db	15
	db	112
	db	0
	db	0
	db	3
	db 0
	dd	16516
	dd	4485
	dd	8480
	[align 32]
	;.type	cmpsd_insn,@object
	;.size	cmpsd_insn,56
cmpsd_insn:
	dd	0
	dd	0
	db	32
	db	1
	db	-89
	db	0
	db	0
	db	0
	db	0
	db 0
	dd	0
	dd	0
	dd	0
	dd	32768
	dd	0
	db	0
	db	3
	db	-14
	db	15
	db	-62
	db	0
	db	3
	db 0
	dd	16580
	dd	4549
	dd	8480
	[align 32]
	;.type	movaupd_insn,@object
	;.size	movaupd_insn,56
movaupd_insn:
	dd	32768
	dd	1
	db	0
	db	3
	db	102
	db	15
	db	0
	db	0
	db	2
	db 0
	dd	16580
	dd	4549
	dd	0
	dd	32768
	dd	1
	db	0
	db	3
	db	102
	db	15
	db	1
	db	0
	db	2
	db 0
	dd	4549
	dd	16580
	dd	0
	[align 32]
	;.type	movhlpd_insn,@object
	;.size	movhlpd_insn,56
movhlpd_insn:
	dd	32768
	dd	1
	db	0
	db	3
	db	102
	db	15
	db	0
	db	0
	db	2
	db 0
	dd	16580
	dd	4482
	dd	0
	dd	32768
	dd	1
	db	0
	db	3
	db	102
	db	15
	db	1
	db	0
	db	2
	db 0
	dd	4482
	dd	16580
	dd	0
	[align 4]
	;.type	movmskpd_insn,@object
	;.size	movmskpd_insn,28
movmskpd_insn:
	dd	32768
	dd	0
	db	0
	db	3
	db	102
	db	15
	db	80
	db	0
	db	2
	db 0
	dd	4193
	dd	16580
	dd	0
	[align 4]
	;.type	movntpddq_insn,@object
	;.size	movntpddq_insn,28
movntpddq_insn:
	dd	32768
	dd	1
	db	0
	db	3
	db	102
	db	15
	db	0
	db	0
	db	2
	db 0
	dd	4546
	dd	16580
	dd	0
	[align 32]
	;.type	movsd_insn,@object
	;.size	movsd_insn,112
movsd_insn:
	dd	0
	dd	0
	db	32
	db	1
	db	-91
	db	0
	db	0
	db	0
	db	0
	db 0
	dd	0
	dd	0
	dd	0
	dd	32768
	dd	0
	db	0
	db	3
	db	-14
	db	15
	db	16
	db	0
	db	2
	db 0
	dd	16580
	dd	4292
	dd	0
	dd	32768
	dd	0
	db	0
	db	3
	db	-14
	db	15
	db	16
	db	0
	db	2
	db 0
	dd	16580
	dd	4482
	dd	0
	dd	32768
	dd	0
	db	0
	db	3
	db	-14
	db	15
	db	17
	db	0
	db	2
	db 0
	dd	4482
	dd	16580
	dd	0
	[align 4]
	;.type	maskmovdqu_insn,@object
	;.size	maskmovdqu_insn,28
maskmovdqu_insn:
	dd	32768
	dd	0
	db	0
	db	3
	db	102
	db	15
	db	-9
	db	0
	db	2
	db 0
	dd	16580
	dd	4292
	dd	0
	[align 32]
	;.type	movdqau_insn,@object
	;.size	movdqau_insn,56
movdqau_insn:
	dd	32768
	dd	16
	db	0
	db	3
	db	0
	db	15
	db	111
	db	0
	db	2
	db 0
	dd	16580
	dd	4549
	dd	0
	dd	32768
	dd	16
	db	0
	db	3
	db	0
	db	15
	db	127
	db	0
	db	2
	db 0
	dd	4549
	dd	16580
	dd	0
	[align 4]
	;.type	movdq2q_insn,@object
	;.size	movdq2q_insn,28
movdq2q_insn:
	dd	32768
	dd	0
	db	0
	db	3
	db	-14
	db	15
	db	-42
	db	0
	db	2
	db 0
	dd	16516
	dd	4292
	dd	0
	[align 4]
	;.type	movq2dq_insn,@object
	;.size	movq2dq_insn,28
movq2dq_insn:
	dd	32768
	dd	0
	db	0
	db	3
	db	-13
	db	15
	db	-42
	db	0
	db	2
	db 0
	dd	16580
	dd	4228
	dd	0
	[align 4]
	;.type	pslrldq_insn,@object
	;.size	pslrldq_insn,28
pslrldq_insn:
	dd	32768
	dd	32
	db	0
	db	3
	db	102
	db	15
	db	115
	db	0
	db	2
	db 0
	dd	4292
	dd	8480
	dd	0
	[align 4]
	;.type	now3d_insn,@object
	;.size	now3d_insn,28
now3d_insn:
	dd	65536
	dd	128
	db	0
	db	2
	db	15
	db	15
	db	0
	db	0
	db	2
	db 0
	dd	16516
	dd	4485
	dd	0
	[align 4]
	;.type	cyrixmmx_insn,@object
	;.size	cyrixmmx_insn,28
cyrixmmx_insn:
	dd	139264
	dd	4
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	2
	db 0
	dd	16516
	dd	4485
	dd	0
	[align 4]
	;.type	pmachriw_insn,@object
	;.size	pmachriw_insn,28
pmachriw_insn:
	dd	139264
	dd	0
	db	0
	db	2
	db	15
	db	94
	db	0
	db	0
	db	2
	db 0
	dd	16516
	dd	4482
	dd	0
	[align 4]
	;.type	rsdc_insn,@object
	;.size	rsdc_insn,28
rsdc_insn:
	dd	655368
	dd	0
	db	0
	db	2
	db	15
	db	121
	db	0
	db	0
	db	2
	db 0
	dd	16454
	dd	4514
	dd	0
	[align 4]
	;.type	cyrixsmm_insn,@object
	;.size	cyrixsmm_insn,28
cyrixsmm_insn:
	dd	655368
	dd	4
	db	0
	db	2
	db	15
	db	0
	db	0
	db	0
	db	1
	db 0
	dd	4514
	dd	0
	dd	0
	[align 4]
	;.type	svdc_insn,@object
	;.size	svdc_insn,28
svdc_insn:
	dd	655368
	dd	0
	db	0
	db	2
	db	15
	db	120
	db	0
	db	0
	db	2
	db 0
	dd	4514
	dd	16454
	dd	0
	[align 32]
	;.type	ibts_insn,@object
	;.size	ibts_insn,56
ibts_insn:
	dd	6291460
	dd	0
	db	16
	db	2
	db	15
	db	-89
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	16449
	dd	0
	dd	6291460
	dd	0
	db	32
	db	2
	db	15
	db	-89
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	16481
	dd	0
	[align 32]
	;.type	umov_insn,@object
	;.size	umov_insn,168
umov_insn:
	dd	2097156
	dd	0
	db	0
	db	2
	db	15
	db	16
	db	0
	db	0
	db	2
	db 0
	dd	4387
	dd	16417
	dd	0
	dd	2097156
	dd	0
	db	16
	db	2
	db	15
	db	17
	db	0
	db	0
	db	2
	db 0
	dd	4419
	dd	16449
	dd	0
	dd	2097156
	dd	0
	db	32
	db	2
	db	15
	db	17
	db	0
	db	0
	db	2
	db 0
	dd	4451
	dd	16481
	dd	0
	dd	2097156
	dd	0
	db	0
	db	2
	db	15
	db	18
	db	0
	db	0
	db	2
	db 0
	dd	16417
	dd	4387
	dd	0
	dd	2097156
	dd	0
	db	16
	db	2
	db	15
	db	19
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4419
	dd	0
	dd	2097156
	dd	0
	db	32
	db	2
	db	15
	db	19
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4451
	dd	0
	[align 32]
	;.type	xbts_insn,@object
	;.size	xbts_insn,56
xbts_insn:
	dd	6291460
	dd	0
	db	16
	db	2
	db	15
	db	-90
	db	0
	db	0
	db	2
	db 0
	dd	16449
	dd	4418
	dd	0
	dd	6291460
	dd	0
	db	32
	db	2
	db	15
	db	-90
	db	0
	db	0
	db	2
	db 0
	dd	16481
	dd	4450
	dd	0
	;.type	size_lookup.0,@object
	;.size	size_lookup.0,8
size_lookup.0:
	db	0
	db	8
	db	16
	db	32
	db	64
	db	80
	db	-128
	db	0
	[section	.rodata];.str1.1,"aMS",@progbits,1
LC0:
	db	"invalid operand conversion", 0
LC1:
	db	"./modules/arch/x86/x86id.re", 0
LC2:
	db	"$", 0
	[section .text]
	;.type	x86_new_jmp,@function
x86_new_jmp:
	push	ebp
	mov	ebp, esp
	push	edi
	push	esi
	push	ebx
	sub	esp, 76
	mov	edx, DWORD [ebp+8]
	mov	eax, DWORD [edx+4]
	movzx	esi, al
	mov	ebx, DWORD [edx]
	shr	eax, 8
	mov	DWORD [ebp-60], eax
	mov	edi, DWORD [ebp+32]
	mov	DWORD [ebp-56], edi
	mov	eax, DWORD [ebp+16]
	mov	edi, DWORD [eax]
	cmp	DWORD [edi+4], 4
	je	.L2
	sub	esp, 4
	push	DWORD LC0
	push	DWORD 1543
	push	DWORD LC1
	call	[DWORD yasm_internal_error_]
	add	esp, 16
.L2:
	mov	edx, DWORD [ebp+20]
	mov	eax, DWORD [edx+16]
	and	eax, 3584
	cmp	eax, 1536
	jne	.L3
	push	DWORD [ebp+32]
	sub	esp, 8
	push	DWORD [edi+8]
	call	yasm_expr_copy
	mov	DWORD [esp], eax
	call	yasm_expr_expr
	add	esp, 12
	push	eax
	sub	esp, 8
	push	DWORD [ebp+32]
	push	DWORD 0
	sub	esp, 4
	push	DWORD [edi+8]
	call	yasm_expr_expr
	add	esp, 8
	push	eax
	push	DWORD 25
	call	yasm_expr_new
	add	esp, 20
	push	eax
	call	yasm_expr_expr
	add	esp, 8
	push	eax
	push	DWORD 27
	call	yasm_expr_new
	mov	DWORD [ebp-52], eax
	add	esp, 16
	jmp	.L4
.L3:
	mov	eax, DWORD [edi+8]
	mov	DWORD [ebp-52], eax
.L4:
	sub	esp, 12
	push	DWORD [ebp+32]
	push	DWORD 0
	push	DWORD [ebp+28]
	push	DWORD [ebp+24]
	push	DWORD LC2
	call	yasm_symrec_define_label
	mov	DWORD [ebp-48], eax
	mov	BYTE [ebp-32], 0
	add	esp, 32
	mov	edi, DWORD [ebp+20]
	mov	eax, DWORD [edi+16]
	and	eax, 3584
	cmp	eax, 1024
	je	.L6
	cmp	eax, 1024
	jg	.L11
	cmp	eax, 512
	je	.L7
	jmp	.L9
.L11:
	cmp	eax, 1536
	je	.L8
	jmp	.L9
.L6:
	mov	DWORD [ebp-44], 3
	jmp	.L5
.L7:
	mov	DWORD [ebp-44], 4
	jmp	.L5
.L8:
	mov	DWORD [ebp-44], 5
	mov	al, BYTE [ebx+9]
	mov	BYTE [ebp-32], al
	mov	al, BYTE [ebx+10]
	mov	BYTE [ebp-31], al
	mov	al, BYTE [ebx+11]
	mov	BYTE [ebp-30], al
	mov	al, BYTE [ebx+12]
	mov	BYTE [ebp-29], al
	jmp	.L5
.L9:
	mov	DWORD [ebp-44], 0
.L5:
	mov	edx, DWORD [ebp+20]
	mov	al, BYTE [edx+8]
	mov	BYTE [ebp-27], al
	cmp	BYTE [edx+14], 1
	jbe	.L12
	mov	edx, DWORD [edx+20]
	mov	eax, edx
	and	eax, 61440
	cmp	eax, 36864
	jne	.L12
	mov	eax, edx
	and	eax, 224
	shr	eax, 5
	mov	al, BYTE [size_lookup.0+eax]
	mov	BYTE [ebp-28], al
	jmp	.L13
.L12:
	mov	BYTE [ebp-28], 0
.L13:
	mov	eax, DWORD [ebp+20]
	test	BYTE [eax+5], 1
	je	.L14
	mov	dl, BYTE [ebp-60]
	mov	BYTE [ebp-28], dl
.L14:
	mov	BYTE [ebp-40], 0
	mov	BYTE [ebp-36], 0
	test	esi, esi
	jle	.L16
	mov	cl, BYTE [yasm_x86_LTX_mode_bits]
	mov	edi, DWORD [cpu_enabled]
	mov	DWORD [ebp-76], edi
	mov	al, BYTE [ebp-27]
	mov	BYTE [ebp-61], al
.L34:
	mov	eax, DWORD [ebx]
	mov	edx, eax
	mov	edi, DWORD [ebp+8]
	or	edx, DWORD [edi+8]
	test	edx, 16777216
	je	.L20
	cmp	cl, 64
	jne	.L17
.L20:
	test	edx, 33554432
	je	.L21
	cmp	cl, 64
	je	.L17
.L21:
	and	edx, -50331649
	mov	eax, DWORD [ebp-76]
	and	eax, edx
	cmp	eax, edx
	jne	.L17
	cmp	BYTE [ebx+14], 0
	je	.L17
	mov	edx, DWORD [ebx+16]
	mov	eax, edx
	and	eax, 61440
	cmp	eax, 32768
	jne	.L17
	mov	al, BYTE [ebp-61]
	cmp	BYTE [ebx+8], al
	jne	.L17
	mov	eax, edx
	and	eax, 3584
	cmp	eax, 512
	je	.L29
	cmp	eax, 1024
	jne	.L17
	mov	al, BYTE [ebx+9]
	mov	BYTE [ebp-40], al
	mov	dl, BYTE [ebx+10]
	mov	BYTE [ebp-39], dl
	mov	al, BYTE [ebx+11]
	mov	BYTE [ebp-38], al
	mov	al, BYTE [ebx+12]
	mov	BYTE [ebp-37], al
	test	BYTE [ebx+4], 16
	je	.L17
	add	edx, DWORD [ebp-60]
	mov	BYTE [ebp-39], dl
	jmp	.L17
.L29:
	mov	al, BYTE [ebx+9]
	mov	BYTE [ebp-36], al
	mov	al, BYTE [ebx+10]
	mov	BYTE [ebp-35], al
	mov	dl, BYTE [ebx+11]
	mov	BYTE [ebp-34], dl
	mov	al, BYTE [ebx+12]
	mov	BYTE [ebp-33], al
	test	BYTE [ebx+4], 4
	je	.L30
	add	edx, DWORD [ebp-60]
	mov	BYTE [ebp-34], dl
.L30:
	mov	eax, DWORD [ebx+16]
	and	eax, 196608
	cmp	eax, 196608
	jne	.L17
	mov	BYTE [ebp-32], 1
	movzx	eax, BYTE [ebx+9]
	mov	al, BYTE [eax+10+ebx]
	mov	BYTE [ebp-31], al
.L17:
	dec	esi
	add	ebx, 28
	test	esi, esi
	jle	.L16
	cmp	BYTE [ebp-40], 0
	je	.L34
	cmp	BYTE [ebp-36], 0
	je	.L34
.L16:
	sub	esp, 12
	lea	eax, [ebp-56]
	push	eax
	call	yasm_x86__bc_new_jmp
	lea	esp, [ebp-12]
	pop	ebx
	pop	esi
	pop	edi
	leave
	ret
.Lfe1:
	;.size	x86_new_jmp,.Lfe1-x86_new_jmp
	[section	.rodata]
	[align 32]
	;.type	size_lookup.1,@object
	;.size	size_lookup.1,32
size_lookup.1:
	dd	0
	dd	1
	dd	2
	dd	4
	dd	8
	dd	10
	dd	16
	dd	0
	[section	.rodata];.str1.1
LC3:
	db	"invalid operand type", 0
LC4:
	db	"invalid target modifier type", 0
LC6:
	db	"mismatch in operand sizes", 0
LC7:
	db	"operand size not specified", 0
	[section	.rodata];.str1.32,"aMS",@progbits,1
	[align 32]
LC8:
	db	"unrecognized x86 ext mod index", 0
	[align 32]
LC9:
	db	"unrecognized x86 extended modifier", 0
	[align 32]
LC5:
	db	"invalid combination of opcode and operands", 0
	[section	.rodata];.str1.1
LC10:
	db	"unknown operand action", 0
	[section	.rodata];.str1.32
	[align 32]
LC11:
	db	"unknown operand postponed action", 0
	[section .text]
[global yasm_x86__parse_insn]
	;.type	yasm_x86__parse_insn,@function
yasm_x86__parse_insn:
	push	ebp
	mov	ebp, esp
	push	edi
	push	esi
	push	ebx
	sub	esp, 76
	mov	edx, DWORD [ebp+8]
	mov	eax, DWORD [edx+4]
	mov	ebx, DWORD [edx]
	mov	ecx, eax
	shr	ecx, 8
	mov	DWORD [ebp-68], ecx
	mov	DWORD [ebp-72], 0
	and	eax, 255
	mov	DWORD [ebp-64], eax
	jle	.L38
.L166:
	mov	DWORD [ebp-80], 0
	mov	eax, DWORD [ebx]
	mov	edx, eax
	mov	ecx, DWORD [ebp+8]
	or	edx, DWORD [ecx+8]
	test	edx, 16777216
	je	.L42
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L39
.L42:
	test	edx, 33554432
	je	.L43
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L39
.L43:
	and	edx, -50331649
	mov	eax, edx
	and	eax, DWORD [cpu_enabled]
	cmp	eax, edx
	jne	.L39
	movzx	eax, BYTE [ebx+14]
	cmp	DWORD [ebp+12], eax
	jne	.L39
	cmp	DWORD [ebp+16], 0
	je	.L261
	mov	DWORD [ebp-76], 0
	mov	eax, DWORD [ebp+16]
	mov	edi, DWORD [eax]
	test	edi, edi
	je	.L48
	movzx	eax, BYTE [ebx+14]
	cmp	DWORD [ebp-76], eax
	jge	.L48
	cmp	DWORD [ebp-80], 0
	jne	.L39
.L164:
	mov	edx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+edx*4]
	and	eax, 31
	cmp	eax, 21
	ja	.L139
	jmp	DWORD [.L140+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L140:
	dd	.L53
	dd	.L57
	dd	.L71
	dd	.L55
	dd	.L75
	dd	.L73
	dd	.L83
	dd	.L85
	dd	.L88
	dd	.L91
	dd	.L94
	dd	.L97
	dd	.L103
	dd	.L109
	dd	.L115
	dd	.L118
	dd	.L121
	dd	.L124
	dd	.L127
	dd	.L130
	dd	.L133
	dd	.L136
	[section .text]
.L53:
	cmp	DWORD [edi+4], 4
	jmp	.L273
.L55:
	cmp	DWORD [edi+4], 3
	je	.L52
.L57:
	cmp	DWORD [edi+4], 1
	jne	.L138
	mov	eax, DWORD [edi+8]
	and	eax, -16
	cmp	eax, 48
	je	.L52
	cmp	eax, 48
	ja	.L69
	cmp	eax, 16
	je	.L52
	cmp	eax, 32
	jmp	.L273
.L69:
	cmp	eax, 80
	je	.L52
	cmp	eax, 80
	ja	.L70
	cmp	eax, 64
	jmp	.L273
.L70:
	cmp	eax, 96
	jmp	.L273
.L71:
	cmp	DWORD [edi+4], 3
	jmp	.L273
.L73:
	cmp	DWORD [edi+4], 3
	je	.L52
.L75:
	cmp	DWORD [edi+4], 1
	jne	.L138
	mov	eax, DWORD [edi+8]
	and	eax, -16
	cmp	eax, 112
	je	.L52
	cmp	eax, 128
	jmp	.L273
.L83:
	cmp	DWORD [edi+4], 2
	jmp	.L273
.L85:
	cmp	DWORD [edi+4], 1
	jne	.L138
	mov	eax, DWORD [edi+8]
	and	eax, -16
	cmp	eax, 144
	jmp	.L273
.L88:
	cmp	DWORD [edi+4], 1
	jne	.L138
	mov	eax, DWORD [edi+8]
	and	eax, -16
	cmp	eax, 160
	jmp	.L273
.L91:
	cmp	DWORD [edi+4], 1
	jne	.L138
	mov	eax, DWORD [edi+8]
	and	eax, -16
	cmp	eax, 176
	jmp	.L273
.L94:
	cmp	DWORD [edi+4], 1
	jne	.L138
	cmp	DWORD [edi+8], 96
	jmp	.L273
.L97:
	cmp	DWORD [edi+4], 1
	jne	.L138
	mov	ecx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+ecx*4]
	and	eax, 224
	cmp	eax, 32
	jne	.L100
	cmp	DWORD [edi+8], 16
	je	.L100
	cmp	DWORD [edi+8], 32
	jne	.L138
.L100:
	mov	edx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+edx*4]
	and	eax, 224
	cmp	eax, 64
	jne	.L101
	cmp	DWORD [edi+8], 48
	jne	.L138
.L101:
	mov	ecx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+ecx*4]
	and	eax, 224
	cmp	eax, 96
	jne	.L102
	cmp	DWORD [edi+8], 64
	jne	.L138
.L102:
	mov	edx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+edx*4]
	and	eax, 224
	cmp	eax, 128
	jne	.L52
	cmp	DWORD [edi+8], 80
	jmp	.L273
.L103:
	cmp	DWORD [edi+4], 1
	jne	.L138
	mov	ecx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+ecx*4]
	and	eax, 224
	cmp	eax, 32
	jne	.L106
	cmp	DWORD [edi+8], 17
	je	.L106
	cmp	DWORD [edi+8], 33
	jne	.L138
.L106:
	mov	edx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+edx*4]
	and	eax, 224
	cmp	eax, 64
	jne	.L107
	cmp	DWORD [edi+8], 49
	jne	.L138
.L107:
	mov	ecx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+ecx*4]
	and	eax, 224
	cmp	eax, 96
	jne	.L108
	cmp	DWORD [edi+8], 65
	jne	.L138
.L108:
	mov	edx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+edx*4]
	and	eax, 224
	cmp	eax, 128
	jne	.L52
	cmp	DWORD [edi+8], 81
	jmp	.L273
.L109:
	cmp	DWORD [edi+4], 1
	jne	.L138
	mov	ecx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+ecx*4]
	and	eax, 224
	cmp	eax, 32
	jne	.L112
	cmp	DWORD [edi+8], 18
	je	.L112
	cmp	DWORD [edi+8], 34
	jne	.L138
.L112:
	mov	edx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+edx*4]
	and	eax, 224
	cmp	eax, 64
	jne	.L113
	cmp	DWORD [edi+8], 50
	jne	.L138
.L113:
	mov	ecx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+ecx*4]
	and	eax, 224
	cmp	eax, 96
	jne	.L114
	cmp	DWORD [edi+8], 66
	jne	.L138
.L114:
	mov	edx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+edx*4]
	and	eax, 224
	cmp	eax, 128
	jne	.L52
	cmp	DWORD [edi+8], 82
	jmp	.L273
.L115:
	cmp	DWORD [edi+4], 2
	jne	.L138
	mov	eax, DWORD [edi+8]
	and	eax, 15
	cmp	eax, 1
	jmp	.L273
.L118:
	cmp	DWORD [edi+4], 2
	jne	.L138
	mov	eax, DWORD [edi+8]
	and	eax, 15
	cmp	eax, 3
	jmp	.L273
.L121:
	cmp	DWORD [edi+4], 2
	jne	.L138
	test	BYTE [edi+8], 15
	jmp	.L273
.L124:
	cmp	DWORD [edi+4], 2
	jne	.L138
	mov	eax, DWORD [edi+8]
	and	eax, 15
	cmp	eax, 4
	jmp	.L273
.L127:
	cmp	DWORD [edi+4], 2
	jne	.L138
	mov	eax, DWORD [edi+8]
	and	eax, 15
	cmp	eax, 5
	jmp	.L273
.L130:
	cmp	DWORD [edi+4], 2
	jne	.L138
	mov	eax, DWORD [edi+8]
	and	eax, 15
	cmp	eax, 2
	jmp	.L273
.L133:
	cmp	DWORD [edi+4], 1
	jne	.L138
	cmp	DWORD [edi+8], 148
	jmp	.L273
.L136:
	cmp	DWORD [edi+4], 3
	jne	.L138
	sub	esp, 8
	push	DWORD 1
	push	DWORD [edi+8]
	call	yasm_ea_get_disp
	mov	DWORD [esp], eax
	call	yasm_expr__contains
	add	esp, 16
	test	eax, eax
.L273:
	je	.L52
.L138:
	mov	DWORD [ebp-80], 1
	jmp	.L52
.L139:
	sub	esp, 4
	push	DWORD LC3
	push	DWORD 1849
	push	DWORD LC1
	call	[DWORD yasm_internal_error_]
	add	esp, 16
.L52:
	cmp	DWORD [ebp-80], 0
	jne	.L39
	mov	ecx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+ecx*4]
	and	eax, 224
	shr	eax, 5
	mov	esi, DWORD [size_lookup.1+eax*4]
	cmp	DWORD [edi+4], 1
	jne	.L142
	cmp	DWORD [edi+16], 0
	jne	.L142
	sub	esp, 12
	push	DWORD [edi+8]
	call	yasm_x86__get_reg_size
	add	esp, 16
	cmp	eax, esi
	jmp	.L274
.L142:
	mov	eax, DWORD [ebp-76]
	test	BYTE [ebx+17+eax*4], 1
	je	.L145
	test	esi, esi
	je	.L144
	cmp	DWORD [edi+16], esi
	je	.L144
	cmp	DWORD [edi+16], 0
	jmp	.L274
.L145:
	cmp	DWORD [edi+16], esi
.L274:
	je	.L144
	mov	DWORD [ebp-80], 1
.L144:
	cmp	DWORD [ebp-80], 0
	jne	.L39
	mov	edx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+edx*4]
	and	eax, 3584
	cmp	eax, 1024
	je	.L155
	cmp	eax, 1024
	jg	.L163
	test	eax, eax
	je	.L151
	cmp	eax, 512
	je	.L153
	jmp	.L161
.L163:
	cmp	eax, 1536
	je	.L157
	cmp	eax, 2048
	je	.L159
	jmp	.L161
.L151:
	cmp	DWORD [edi+12], 0
	jmp	.L275
.L153:
	cmp	DWORD [edi+12], 1
	jmp	.L275
.L155:
	cmp	DWORD [edi+12], 2
	jmp	.L275
.L157:
	cmp	DWORD [edi+12], 3
	jmp	.L275
.L159:
	cmp	DWORD [edi+12], 4
.L275:
	je	.L49
	mov	DWORD [ebp-80], 1
	jmp	.L49
.L161:
	sub	esp, 4
	push	DWORD LC4
	push	DWORD 1899
	push	DWORD LC1
	call	[DWORD yasm_internal_error_]
	add	esp, 16
.L49:
	mov	edi, DWORD [edi]
	inc	DWORD [ebp-76]
	test	edi, edi
	je	.L48
	movzx	eax, BYTE [ebx+14]
	cmp	DWORD [ebp-76], eax
	jge	.L48
	cmp	DWORD [ebp-80], 0
	je	.L164
	jmp	.L39
.L48:
	cmp	DWORD [ebp-80], 0
	je	.L261
.L39:
	dec	DWORD [ebp-64]
	add	ebx, 28
	cmp	DWORD [ebp-64], 0
	jle	.L38
	cmp	DWORD [ebp-72], 0
	je	.L166
	jmp	.L167
.L38:
	cmp	DWORD [ebp-72], 0
	jne	.L167
	jmp	.L277
.L261:
	mov	DWORD [ebp-72], 1
	jmp	.L38
.L167:
	mov	eax, DWORD [ebx+4]
	and	eax, -268435456
	cmp	eax, 268435456
	je	.L170
	cmp	eax, 268435456
	jg	.L182
	test	eax, eax
	je	.L168
	jmp	.L180
.L182:
	cmp	eax, 536870912
	je	.L178
	jmp	.L180
.L170:
	mov	eax, DWORD [ebx+4]
	and	eax, 267386880
	shr	eax, 20
	je	.L172
	cmp	eax, 1
	je	.L173
	jmp	.L174
.L172:
	sub	esp, 8
	push	DWORD LC6
	jmp	.L268
.L173:
	sub	esp, 8
	push	DWORD LC7
.L268:
	push	DWORD [ebp+28]
	call	yasm__error
	jmp	.L276
.L174:
	sub	esp, 4
	push	DWORD LC8
	push	DWORD 1930
	push	DWORD LC1
	call	[DWORD yasm_internal_error_]
	jmp	.L276
.L178:
	sub	esp, 4
	push	DWORD LC8
	push	DWORD 1937
	jmp	DWORD .L269
.L180:
	sub	esp, 4
	push	DWORD LC9
	push	DWORD 1941
.L269:
	push	DWORD LC1
	call	[DWORD yasm_internal_error_]
	add	esp, 16
.L168:
	cmp	DWORD [ebp+16], 0
	je	.L183
	mov	eax, DWORD [ebx+16]
	and	eax, 61440
	cmp	eax, 32768
	jne	.L183
	sub	esp, 4
	push	DWORD [ebp+28]
	push	DWORD [ebp+24]
	push	DWORD [ebp+20]
	push	ebx
	push	DWORD [ebp+16]
	push	DWORD [ebp+12]
	push	DWORD [ebp+8]
	call	x86_new_jmp
	jmp	.L36
.L183:
	mov	ecx, DWORD [ebp+28]
	mov	DWORD [ebp-56], ecx
	mov	DWORD [ebp-52], 0
	mov	DWORD [ebp-48], 0
	mov	al, BYTE [ebx+8]
	mov	BYTE [ebp-44], al
	mov	al, BYTE [ebx+9]
	mov	BYTE [ebp-43], al
	mov	al, BYTE [ebx+10]
	mov	BYTE [ebp-42], al
	mov	al, BYTE [ebx+11]
	mov	BYTE [ebp-41], al
	mov	al, BYTE [ebx+12]
	mov	BYTE [ebp-40], al
	mov	al, BYTE [ebx+13]
	mov	BYTE [ebp-39], al
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L184
	cmp	BYTE [ebx+8], 64
	jne	.L184
	mov	al, 72
	jmp	.L185
.L184:
	mov	al, 0
.L185:
	mov	BYTE [ebp-38], al
	mov	BYTE [ebp-37], 0
	mov	BYTE [ebp-36], 0
	mov	BYTE [ebp-35], 0
	mov	BYTE [ebp-34], 0
	test	BYTE [ebx+4], 1
	je	.L186
	mov	al, BYTE [ebp-68]
	add	BYTE [ebp-40], al
	shr	DWORD [ebp-68], 8
.L186:
	test	BYTE [ebx+4], 2
	je	.L187
	shr	DWORD [ebp-68], 8
.L187:
	test	BYTE [ebx+4], 4
	je	.L188
	mov	dl, BYTE [ebp-68]
	add	BYTE [ebp-41], dl
	shr	DWORD [ebp-68], 8
.L188:
	test	BYTE [ebx+4], 8
	je	.L189
	shr	DWORD [ebp-68], 8
.L189:
	test	BYTE [ebx+4], 16
	je	.L190
	mov	cl, BYTE [ebp-68]
	add	BYTE [ebp-42], cl
	shr	DWORD [ebp-68], 8
.L190:
	test	BYTE [ebx+4], 32
	je	.L191
	mov	al, BYTE [ebp-68]
	add	BYTE [ebp-39], al
	shr	DWORD [ebp-68], 8
.L191:
	test	BYTE [ebx+4], 64
	je	.L192
	mov	dl, BYTE [ebp-68]
	mov	BYTE [ebp-44], dl
	shr	DWORD [ebp-68], 8
.L192:
	cmp	BYTE [ebx+4], 0
	jns	.L193
	push	DWORD [ebp+28]
	push	DWORD 0
	sub	esp, 4
	movzx	eax, BYTE [ebp-68]
	push	eax
	call	yasm_intnum_new_uint
	mov	DWORD [esp], eax
	call	yasm_expr_int
	add	esp, 8
	push	eax
	push	DWORD 0
	call	yasm_expr_new
	mov	DWORD [ebp-48], eax
	mov	BYTE [ebp-37], 1
	add	esp, 16
.L193:
	cmp	DWORD [ebp+16], 0
	je	.L194
	mov	DWORD [ebp-76], 0
	mov	ecx, DWORD [ebp+16]
	mov	edi, DWORD [ecx]
	test	edi, edi
	je	.L194
	movzx	eax, BYTE [ebx+14]
	cmp	DWORD [ebp-76], eax
	jge	.L194
.L257:
	mov	edx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+edx*4]
	and	eax, 61440
	cmp	eax, 12288
	je	.L223
	cmp	eax, 12288
	jg	.L247
	cmp	eax, 4096
	je	.L210
	cmp	eax, 4096
	jg	.L248
	test	eax, eax
	je	.L201
	jmp	.L245
.L248:
	cmp	eax, 8192
	je	.L220
	jmp	.L245
.L247:
	cmp	eax, 20480
	je	.L232
	cmp	eax, 20480
	jg	.L249
	cmp	eax, 16384
	je	.L226
	jmp	.L245
.L249:
	cmp	eax, 24576
	je	.L236
	cmp	eax, 28672
	je	.L239
	jmp	.L245
.L201:
	mov	eax, DWORD [edi+4]
	cmp	eax, 3
	je	.L205
	cmp	eax, 3
	jbe	.L200
	cmp	eax, 4
	je	.L206
	jmp	.L200
.L205:
	sub	esp, 12
	push	DWORD [edi+8]
	call	yasm_ea_delete
	jmp	.L271
.L206:
	sub	esp, 12
	push	DWORD [edi+8]
	call	yasm_expr_delete
	jmp	.L271
.L210:
	mov	eax, DWORD [edi+4]
	cmp	eax, 2
	je	.L213
	cmp	eax, 2
	ja	.L219
	cmp	eax, 1
	je	.L212
	jmp	.L200
.L219:
	cmp	eax, 3
	je	.L214
	cmp	eax, 4
	je	.L216
	jmp	.L200
.L212:
	sub	esp, 4
	movzx	eax, BYTE [yasm_x86_LTX_mode_bits]
	push	eax
	lea	eax, [ebp-38]
	push	eax
	push	DWORD [edi+8]
	call	yasm_x86__ea_new_reg
	jmp	.L272
.L213:
	sub	esp, 4
	push	DWORD LC0
	push	DWORD 2025
	push	DWORD LC1
	call	[DWORD yasm_internal_error_]
	add	esp, 16
.L214:
	mov	edx, DWORD [edi+8]
	mov	DWORD [ebp-52], edx
	mov	ecx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+ecx*4]
	and	eax, 31
	cmp	eax, 21
	jne	.L200
	sub	esp, 12
	push	edx
	call	yasm_x86__ea_set_disponly
	jmp	.L271
.L216:
	sub	esp, 8
	mov	edx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+edx*4]
	and	eax, 224
	shr	eax, 5
	push	DWORD [size_lookup.1+eax*4]
	push	DWORD [edi+8]
	call	yasm_x86__ea_new_imm
.L272:
	mov	DWORD [ebp-52], eax
	jmp	.L271
.L220:
	cmp	DWORD [edi+4], 4
	jne	.L221
	mov	eax, DWORD [edi+8]
	mov	DWORD [ebp-48], eax
	mov	ecx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+ecx*4]
	and	eax, 224
	shr	eax, 5
	mov	al, BYTE [size_lookup.1+eax*4]
	mov	BYTE [ebp-37], al
	jmp	.L200
.L221:
	sub	esp, 4
	push	DWORD LC0
	push	DWORD 2045
	jmp	.L270
.L223:
	cmp	DWORD [edi+4], 4
	jne	.L224
	mov	eax, DWORD [edi+8]
	mov	DWORD [ebp-48], eax
	mov	edx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+edx*4]
	and	eax, 224
	shr	eax, 5
	mov	al, BYTE [size_lookup.1+eax*4]
	mov	BYTE [ebp-37], al
	mov	BYTE [ebp-36], 1
	jmp	.L200
.L224:
	sub	esp, 4
	push	DWORD LC0
	push	DWORD 2054
	jmp	.L270
.L226:
	cmp	DWORD [edi+4], 2
	jne	.L227
	mov	al, BYTE [edi+8]
	and	eax, 7
	mov	BYTE [ebp-39], al
	jmp	.L200
.L227:
	cmp	DWORD [edi+4], 1
	jne	.L229
	sub	esp, 12
	push	DWORD 2
	movzx	eax, BYTE [yasm_x86_LTX_mode_bits]
	push	eax
	push	DWORD [edi+8]
	lea	eax, [ebp-39]
	push	eax
	lea	eax, [ebp-38]
	push	eax
	call	yasm_x86__set_rex_from_reg
	add	esp, 32
	test	eax, eax
	je	.L200
.L277:
	sub	esp, 8
	push	DWORD LC5
	push	DWORD [ebp+28]
	call	yasm__error
	jmp	.L243
.L229:
	sub	esp, 4
	push	DWORD LC0
	push	DWORD 2068
	jmp	.L270
.L232:
	cmp	DWORD [edi+4], 1
	jne	.L233
	sub	esp, 12
	push	DWORD 0
	movzx	eax, BYTE [yasm_x86_LTX_mode_bits]
	push	eax
	push	DWORD [edi+8]
	lea	eax, [ebp-57]
	push	eax
	lea	eax, [ebp-38]
	push	eax
	call	yasm_x86__set_rex_from_reg
	add	esp, 32
	test	eax, eax
	jne	.L277
	mov	al, BYTE [ebp-42]
	add	al, BYTE [ebp-57]
	mov	BYTE [ebp-42], al
	jmp	.L200
.L233:
	sub	esp, 4
	push	DWORD LC0
	push	DWORD 2082
	jmp	.L270
.L236:
	cmp	DWORD [edi+4], 1
	jne	.L237
	mov	al, BYTE [edi+8]
	and	eax, 7
	add	BYTE [ebp-41], al
	jmp	.L200
.L237:
	sub	esp, 4
	push	DWORD LC0
	push	DWORD 2089
	jmp	.L270
.L239:
	cmp	DWORD [edi+4], 1
	jne	.L240
	sub	esp, 4
	movzx	eax, BYTE [yasm_x86_LTX_mode_bits]
	push	eax
	lea	esi, [ebp-38]
	push	esi
	push	DWORD [edi+8]
	call	yasm_x86__ea_new_reg
	mov	DWORD [ebp-52], eax
	add	esp, 16
	test	eax, eax
	je	.L242
	sub	esp, 12
	push	DWORD 2
	movzx	eax, BYTE [yasm_x86_LTX_mode_bits]
	push	eax
	push	DWORD [edi+8]
	lea	eax, [ebp-39]
	push	eax
	push	esi
	call	yasm_x86__set_rex_from_reg
	add	esp, 32
	test	eax, eax
	je	.L200
.L242:
	sub	esp, 8
	push	DWORD LC5
	push	DWORD [ebp+28]
	call	yasm__error
	add	esp, 16
	cmp	DWORD [ebp-52], 0
	je	.L243
	sub	esp, 12
	push	DWORD [ebp-52]
	call	[DWORD yasm_xfree]
.L276:
	add	esp, 16
.L243:
	mov	eax, 0
	jmp	.L36
.L240:
	sub	esp, 4
	push	DWORD LC0
	push	DWORD 2106
	jmp	.L270
.L245:
	sub	esp, 4
	push	DWORD LC10
	push	DWORD 2109
.L270:
	push	DWORD LC1
	call	[DWORD yasm_internal_error_]
.L271:
	add	esp, 16
.L200:
	mov	ecx, DWORD [ebp-76]
	mov	eax, DWORD [ebx+16+ecx*4]
	and	eax, 196608
	cmp	eax, 65536
	je	.L252
	cmp	eax, 65536
	jg	.L256
	test	eax, eax
	je	.L197
	jmp	.L254
.L256:
	cmp	eax, 131072
	je	.L253
	jmp	.L254
.L252:
	mov	BYTE [ebp-35], 1
	jmp	.L197
.L253:
	mov	BYTE [ebp-34], 1
	jmp	.L197
.L254:
	sub	esp, 4
	push	DWORD LC11
	push	DWORD 2123
	push	DWORD LC1
	call	[DWORD yasm_internal_error_]
	add	esp, 16
.L197:
	mov	edi, DWORD [edi]
	inc	DWORD [ebp-76]
	test	edi, edi
	je	.L194
	movzx	eax, BYTE [ebx+14]
	cmp	DWORD [ebp-76], eax
	jl	.L257
.L194:
	sub	esp, 12
	lea	eax, [ebp-56]
	push	eax
	call	yasm_x86__bc_new_insn
.L36:
	lea	esp, [ebp-12]
	pop	ebx
	pop	esi
	pop	edi
	leave
	ret
.Lfe2:
	;.size	yasm_x86__parse_insn,.Lfe2-yasm_x86__parse_insn
	[section	.rodata];.str1.32
	[align 32]
LC12:
	db	"unrecognized CPU identifier `s'", 0
	[section .text]
[global yasm_x86__parse_cpu]
	;.type	yasm_x86__parse_cpu,@function
yasm_x86__parse_cpu:
	push	ebp
	mov	ebp, esp
	push	ebx
	sub	esp, 4
	mov	edx, DWORD [ebp+8]
	mov	ebx, DWORD [ebp+12]
.L279:
	movsx	eax, BYTE [edx]
	cmp	eax, 119
	ja	.L338
	jmp	DWORD [.L339+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L339:
	dd	.L283
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L285
	dd	.L287
	dd	.L289
	dd	.L291
	dd	.L293
	dd	.L295
	dd	.L338
	dd	.L297
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L300
	dd	.L338
	dd	.L303
	dd	.L338
	dd	.L338
	dd	.L306
	dd	.L338
	dd	.L309
	dd	.L312
	dd	.L338
	dd	.L315
	dd	.L338
	dd	.L318
	dd	.L321
	dd	.L324
	dd	.L327
	dd	.L338
	dd	.L338
	dd	.L330
	dd	.L338
	dd	.L333
	dd	.L338
	dd	.L336
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L338
	dd	.L300
	dd	.L338
	dd	.L303
	dd	.L338
	dd	.L338
	dd	.L306
	dd	.L338
	dd	.L309
	dd	.L312
	dd	.L338
	dd	.L315
	dd	.L338
	dd	.L318
	dd	.L321
	dd	.L324
	dd	.L327
	dd	.L338
	dd	.L338
	dd	.L330
	dd	.L338
	dd	.L333
	dd	.L338
	dd	.L336
	[section .text]
.L297:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 48
	je	.L341
	jmp	.L342
.L343:
.L312:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 64
	jg	.L344
	cmp	cl, 51
	jle	.L925
	cmp	cl, 52
	jle	.L291
	cmp	cl, 53
	jle	.L293
	cmp	cl, 54
	jle	.L295
	jmp	.L342
.L344:
	cmp	cl, 96
	jg	.L355
	cmp	cl, 65
	jle	.L357
	cmp	cl, 84
	jmp	.L913
.L355:
	cmp	cl, 97
	jle	.L357
	cmp	cl, 116
.L913:
	je	.L359
	jmp	.L342
.L285:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 56
	je	.L364
	jmp	.L342
.L287:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 56
	je	.L366
	jmp	.L342
.L289:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 67
	jle	.L912
	cmp	cl, 68
	jle	.L372
	cmp	cl, 100
	je	.L372
	jmp	.L342
.L291:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 56
	je	.L375
	jmp	.L342
.L293:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 56
	je	.L377
	jmp	.L342
.L327:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 79
	jg	.L378
	cmp	cl, 52
	jle	.L923
	cmp	cl, 54
	jg	.L387
	cmp	cl, 53
	jle	.L389
	jmp	.L390
.L387:
	cmp	cl, 69
	je	.L393
	jmp	.L342
.L378:
	cmp	cl, 101
	jg	.L395
	cmp	cl, 81
	jg	.L396
	cmp	cl, 80
	jle	.L824
	jmp	.L342
.L396:
	cmp	cl, 82
	jle	.L401
	cmp	cl, 100
	jle	.L342
	jmp	.L393
.L395:
	cmp	cl, 112
	jg	.L404
	cmp	cl, 111
	jle	.L342
	jmp	.L824
.L404:
	cmp	cl, 114
	je	.L401
	jmp	.L342
.L295:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 56
	je	.L409
	jmp	.L342
.L315:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 64
	jg	.L410
	cmp	cl, 53
	jle	.L342
	cmp	cl, 54
	jle	.L413
	cmp	cl, 55
	jle	.L415
	jmp	.L342
.L410:
	cmp	cl, 65
	jle	.L418
	cmp	cl, 97
	je	.L418
	jmp	.L342
.L336:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 73
	je	.L421
	cmp	cl, 105
	je	.L421
	jmp	.L342
.L300:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 84
	jg	.L423
	cmp	cl, 77
	je	.L425
	cmp	cl, 83
	jle	.L342
	jmp	.L427
.L423:
	cmp	cl, 109
	jg	.L429
	cmp	cl, 108
	jle	.L342
	jmp	.L425
.L429:
	cmp	cl, 116
	je	.L427
	jmp	.L342
.L330:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 83
	jg	.L433
	cmp	cl, 76
	jg	.L434
	cmp	cl, 75
	jle	.L342
	jmp	.L436
.L434:
	cmp	cl, 77
	jle	.L439
	cmp	cl, 82
	jle	.L342
	jmp	.L441
.L433:
	cmp	cl, 109
	jg	.L443
	cmp	cl, 107
	jle	.L342
	cmp	cl, 108
	jle	.L436
	jmp	.L439
.L443:
	cmp	cl, 115
	je	.L441
	jmp	.L342
.L309:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 65
	je	.L449
	cmp	cl, 97
	je	.L449
	jmp	.L342
.L324:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 80
	jg	.L451
	cmp	cl, 66
	je	.L453
	cmp	cl, 79
	jle	.L342
	jmp	.L455
.L451:
	cmp	cl, 98
	jg	.L457
	cmp	cl, 97
	jle	.L342
	jmp	.L453
.L457:
	cmp	cl, 112
	je	.L455
	jmp	.L342
.L306:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 80
	je	.L462
	cmp	cl, 112
	je	.L462
	jmp	.L342
.L321:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 79
	je	.L465
	cmp	cl, 111
	je	.L465
	jmp	.L342
.L318:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 77
	je	.L468
	cmp	cl, 109
	je	.L468
	jmp	.L342
.L303:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 89
	je	.L471
	cmp	cl, 121
	je	.L471
	jmp	.L342
.L333:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 78
	je	.L474
	cmp	cl, 110
	je	.L474
	jmp	.L342
.L338:
	inc	edx
	mov	cl, BYTE [edx]
.L342:
	test	cl, cl
	jle	.L922
	jmp	.L338
.L283:
	inc	edx
.L922:
	push	edx
	push	DWORD LC12
	push	ebx
	push	DWORD 0
	call	yasm__warning
	jmp	.L278
.L474:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 68
	je	.L479
	cmp	cl, 100
	jne	.L342
.L479:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 79
	je	.L482
	cmp	cl, 111
	jne	.L342
.L482:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 67
	je	.L485
	cmp	cl, 99
	jne	.L342
.L485:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	or	DWORD [cpu_enabled], 2097152
	jmp	.L278
.L471:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 82
	je	.L489
	cmp	cl, 114
	jne	.L342
.L489:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 73
	je	.L492
	cmp	cl, 105
	jne	.L342
.L492:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 88
	je	.L495
	cmp	cl, 120
	jne	.L342
.L495:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	or	DWORD [cpu_enabled], 131072
	jmp	.L278
.L468:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 88
	je	.L499
	cmp	cl, 120
	jne	.L342
.L499:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	or	DWORD [cpu_enabled], 8192
	jmp	.L278
.L465:
	inc	edx
	mov	cl, BYTE [edx]
	movsx	eax, cl
	sub	eax, 51
	cmp	eax, 66
	ja	.L342
	jmp	DWORD [.L530+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L530:
	dd	.L504
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L507
	dd	.L342
	dd	.L510
	dd	.L342
	dd	.L342
	dd	.L513
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L516
	dd	.L342
	dd	.L519
	dd	.L522
	dd	.L342
	dd	.L342
	dd	.L525
	dd	.L342
	dd	.L528
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L507
	dd	.L342
	dd	.L510
	dd	.L342
	dd	.L342
	dd	.L513
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L342
	dd	.L516
	dd	.L342
	dd	.L519
	dd	.L522
	dd	.L342
	dd	.L342
	dd	.L525
	dd	.L342
	dd	.L528
	[section .text]
.L513:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 80
	je	.L532
	cmp	cl, 112
	je	.L532
	jmp	.L342
.L516:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 77
	je	.L535
	cmp	cl, 109
	je	.L535
	jmp	.L342
.L525:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 83
	jg	.L537
	cmp	cl, 77
	je	.L539
	cmp	cl, 82
	jle	.L342
	jmp	.L541
.L537:
	cmp	cl, 109
	jg	.L543
	cmp	cl, 108
	jle	.L342
	jmp	.L539
.L543:
	cmp	cl, 115
	je	.L541
	jmp	.L342
.L504:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 68
	je	.L548
	cmp	cl, 100
	je	.L548
	jmp	.L342
.L510:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 89
	je	.L551
	cmp	cl, 121
	je	.L551
	jmp	.L342
.L507:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 77
	je	.L554
	cmp	cl, 109
	je	.L554
	jmp	.L342
.L522:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 82
	je	.L557
	cmp	cl, 114
	je	.L557
	jmp	.L342
.L528:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 78
	je	.L560
	cmp	cl, 110
	je	.L560
	jmp	.L342
.L519:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 66
	je	.L563
	cmp	cl, 98
	jne	.L342
.L563:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 83
	je	.L566
	cmp	cl, 115
	jne	.L342
.L566:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	and	DWORD [cpu_enabled], -4194305
	jmp	.L278
.L560:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 68
	je	.L570
	cmp	cl, 100
	jne	.L342
.L570:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 79
	je	.L573
	cmp	cl, 111
	jne	.L342
.L573:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 67
	je	.L576
	cmp	cl, 99
	jne	.L342
.L576:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	and	DWORD [cpu_enabled], -2097153
	jmp	.L278
.L557:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 79
	jg	.L579
	cmp	cl, 73
	je	.L581
	cmp	cl, 78
	jle	.L342
	jmp	.L583
.L579:
	cmp	cl, 105
	jg	.L584
	cmp	cl, 104
	jle	.L342
	jmp	.L581
.L584:
	cmp	cl, 111
	jne	.L342
.L583:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 84
	je	.L589
	cmp	cl, 116
	je	.L589
	jmp	.L342
.L581:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 86
	je	.L592
	cmp	cl, 118
	jne	.L342
.L592:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	and	DWORD [cpu_enabled], -8388609
	jmp	.L278
.L589:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	and	DWORD [cpu_enabled], -1048577
	jmp	.L278
.L554:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 68
	je	.L597
	cmp	cl, 100
	jne	.L342
.L597:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	and	DWORD [cpu_enabled], -262145
	jmp	.L278
.L551:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 82
	je	.L601
	cmp	cl, 114
	jne	.L342
.L601:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 73
	je	.L604
	cmp	cl, 105
	jne	.L342
.L604:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 88
	je	.L607
	cmp	cl, 120
	jne	.L342
.L607:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	and	DWORD [cpu_enabled], -131073
	jmp	.L278
.L548:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 78
	je	.L611
	cmp	cl, 110
	jne	.L342
.L611:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 79
	je	.L614
	cmp	cl, 111
	jne	.L342
.L614:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 87
	je	.L617
	cmp	cl, 119
	jne	.L342
.L617:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	and	DWORD [cpu_enabled], -65537
	jmp	.L278
.L539:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 77
	je	.L621
	cmp	cl, 109
	je	.L621
	jmp	.L342
.L541:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 69
	je	.L624
	cmp	cl, 101
	jne	.L342
.L624:
	inc	edx
	mov	cl, BYTE [edx]
	test	cl, cl
	jle	.L627
	cmp	cl, 50
	je	.L629
	jmp	.L338
.L627:
	and	DWORD [cpu_enabled], -16385
	jmp	.L278
.L629:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	and	DWORD [cpu_enabled], -32769
	jmp	.L278
.L621:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	and	DWORD [cpu_enabled], -524289
	jmp	.L278
.L535:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 88
	je	.L633
	cmp	cl, 120
	jne	.L342
.L633:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	and	DWORD [cpu_enabled], -8193
	jmp	.L278
.L532:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 85
	je	.L637
	cmp	cl, 117
	jne	.L342
.L637:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	and	DWORD [cpu_enabled], -4097
	jmp	.L278
.L462:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 85
	je	.L641
	cmp	cl, 117
	jne	.L342
.L641:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	or	DWORD [cpu_enabled], 4096
	jmp	.L278
.L453:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 83
	je	.L645
	cmp	cl, 115
	je	.L645
	jmp	.L342
.L455:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 84
	je	.L648
	cmp	cl, 116
	jne	.L342
.L648:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 69
	je	.L651
	cmp	cl, 101
	jne	.L342
.L651:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 82
	je	.L654
	cmp	cl, 114
	jne	.L342
.L654:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 79
	je	.L657
	cmp	cl, 111
	jne	.L342
.L657:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 78
	je	.L660
	cmp	cl, 110
.L914:
	jne	.L342
.L660:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	mov	DWORD [cpu_enabled], 10059327
	jmp	.L278
.L645:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	or	DWORD [cpu_enabled], 4194304
	jmp	.L278
.L449:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 77
	je	.L665
	cmp	cl, 109
	jne	.L342
.L665:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 77
	je	.L668
	cmp	cl, 109
	jne	.L342
.L668:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 69
	je	.L671
	cmp	cl, 101
	jne	.L342
.L671:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 82
	je	.L660
	cmp	cl, 114
	jmp	.L914
.L436:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 69
	je	.L676
	cmp	cl, 101
	je	.L676
	jmp	.L342
.L439:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 77
	je	.L679
	cmp	cl, 109
	je	.L679
	jmp	.L342
.L441:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 69
	je	.L682
	cmp	cl, 101
	jne	.L342
.L682:
	inc	edx
	mov	cl, BYTE [edx]
	test	cl, cl
	jle	.L685
	cmp	cl, 50
	je	.L687
	jmp	.L338
.L685:
	or	DWORD [cpu_enabled], 16384
	jmp	.L278
.L687:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	or	DWORD [cpu_enabled], 32768
	jmp	.L278
.L679:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	or	DWORD [cpu_enabled], 524288
	jmp	.L278
.L676:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 68
	je	.L691
	cmp	cl, 100
	jne	.L342
.L691:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 71
	je	.L694
	cmp	cl, 103
	jne	.L342
.L694:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 69
	je	.L697
	cmp	cl, 101
	jne	.L342
.L697:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 72
	je	.L309
	cmp	cl, 104
	je	.L309
	jmp	.L342
.L425:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 68
	je	.L702
	cmp	cl, 100
	je	.L702
	jmp	.L342
.L427:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 72
	je	.L705
	cmp	cl, 104
	jne	.L342
.L705:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 76
	je	.L708
	cmp	cl, 108
	jne	.L342
.L708:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 79
	je	.L711
	cmp	cl, 111
	jne	.L342
.L711:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 78
	je	.L714
	cmp	cl, 110
	jne	.L342
.L714:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 45
	jg	.L716
	test	cl, cl
	jle	.L718
	cmp	cl, 44
	jle	.L338
	jmp	.L720
.L716:
	cmp	cl, 54
	je	.L723
	jmp	.L338
.L718:
	mov	DWORD [cpu_enabled], 10057279
	jmp	.L278
.L720:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 54
	jne	.L342
.L723:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 52
	jmp	.L914
.L702:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	or	DWORD [cpu_enabled], 262144
	jmp	.L278
.L421:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 76
	je	.L728
	cmp	cl, 108
	jne	.L342
.L728:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 76
	je	.L731
	cmp	cl, 108
	jne	.L342
.L731:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 73
	je	.L734
	cmp	cl, 105
	jne	.L342
.L734:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 65
	je	.L737
	cmp	cl, 97
	jne	.L342
.L737:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 77
	je	.L740
	cmp	cl, 109
	jne	.L342
.L740:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 69
	je	.L743
	cmp	cl, 101
	jne	.L342
.L743:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 84
	je	.L746
	cmp	cl, 116
	jne	.L342
.L746:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 84
	je	.L749
	cmp	cl, 116
	jne	.L342
.L749:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 69
	je	.L385
	cmp	cl, 101
.L915:
	jne	.L342
.L385:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	mov	DWORD [cpu_enabled], 10023167
	jmp	.L278
.L413:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	mov	DWORD [cpu_enabled], 10039871
	jmp	.L278
.L415:
	inc	edx
	cmp	BYTE [edx], 0
	jle	.L718
	jmp	.L338
.L418:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 84
	je	.L757
	cmp	cl, 116
	jne	.L342
.L757:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 77
	je	.L760
	cmp	cl, 109
	jne	.L342
.L760:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 65
	je	.L763
	cmp	cl, 97
	jne	.L342
.L763:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 73
	je	.L384
	cmp	cl, 105
	jne	.L342
.L384:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	mov	DWORD [cpu_enabled], 9990271
	jmp	.L278
.L409:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 54
.L917:
	jne	.L342
.L390:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	mov	DWORD [cpu_enabled], 9965631
	jmp	.L278
.L382:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
.L771:
	mov	DWORD [cpu_enabled], 9973823
	jmp	.L278
.L389:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
.L773:
	mov	DWORD [cpu_enabled], 9965599
	jmp	.L278
.L393:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 78
	je	.L775
	cmp	cl, 110
	je	.L775
	jmp	.L342
.L398:
.L401:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 79
	jg	.L780
	cmp	cl, 73
	je	.L782
	cmp	cl, 78
	jle	.L342
	jmp	.L784
.L780:
	cmp	cl, 105
	jg	.L785
	cmp	cl, 104
	jle	.L342
	jmp	.L782
.L785:
	cmp	cl, 111
	jne	.L342
.L784:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 84
	je	.L790
	cmp	cl, 116
	je	.L790
	jmp	.L342
.L782:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 86
	je	.L793
	cmp	cl, 118
	jne	.L342
.L793:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	or	DWORD [cpu_enabled], 8388608
	jmp	.L278
.L790:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	or	DWORD [cpu_enabled], 1048576
	jmp	.L278
.L778:
.L775:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 84
	je	.L800
	cmp	cl, 116
	jne	.L342
.L800:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 73
	je	.L803
	cmp	cl, 105
	jne	.L342
.L803:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 85
	je	.L806
	cmp	cl, 117
	jne	.L342
.L806:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 77
	je	.L809
	cmp	cl, 109
	jne	.L342
.L809:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 72
	jg	.L811
	cmp	cl, 49
	jg	.L812
	test	cl, cl
	jle	.L773
	cmp	cl, 45
	jne	.L338
	jmp	.L819
.L812:
	cmp	cl, 50
	jle	.L382
	cmp	cl, 51
	jle	.L384
	cmp	cl, 52
	jle	.L385
	jmp	.L338
.L811:
	cmp	cl, 104
	jg	.L820
	cmp	cl, 73
	jle	.L822
	cmp	cl, 80
	jmp	.L908
.L820:
	cmp	cl, 105
	jle	.L822
	cmp	cl, 112
.L908:
	je	.L824
	jmp	.L338
.L819:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 52
	jg	.L828
.L923:
	cmp	cl, 49
	jle	.L342
	cmp	cl, 50
	jle	.L382
	cmp	cl, 51
	jle	.L384
	jmp	.L385
.L828:
	cmp	cl, 73
	jg	.L833
	cmp	cl, 72
	jle	.L342
	jmp	.L822
.L833:
	cmp	cl, 105
	jne	.L342
.L822:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 86
	jg	.L837
	cmp	cl, 73
	je	.L839
	cmp	cl, 85
	jle	.L342
	jmp	.L385
.L837:
	cmp	cl, 105
	jg	.L842
	cmp	cl, 104
	jle	.L342
	jmp	.L839
.L842:
	cmp	cl, 118
	jmp	.L915
.L824:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 82
	je	.L847
	cmp	cl, 114
	jne	.L342
.L847:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 79
	je	.L390
	cmp	cl, 111
	jmp	.L917
.L839:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 73
	jg	.L851
	test	cl, cl
	jle	.L771
	cmp	cl, 72
	jle	.L338
	jmp	.L384
.L851:
	cmp	cl, 105
	je	.L384
	jmp	.L338
.L377:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 54
	je	.L389
	jmp	.L342
.L375:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 54
	jne	.L342
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	mov	DWORD [cpu_enabled], 9965583
	jmp	.L278
.L369:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 54
	je	.L860
	jmp	.L342
.L372:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 78
	je	.L862
	cmp	cl, 110
	jne	.L342
.L862:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 79
	je	.L865
	cmp	cl, 111
	jne	.L342
.L865:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 87
	je	.L868
	cmp	cl, 119
	jne	.L342
.L868:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	or	DWORD [cpu_enabled], 65536
	jmp	.L278
.L860:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	mov	DWORD [cpu_enabled], 9961479
	jmp	.L278
.L366:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 54
	jne	.L342
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	mov	DWORD [cpu_enabled], 8388611
	jmp	.L278
.L364:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 54
	jne	.L342
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	mov	DWORD [cpu_enabled], 8388609
	jmp	.L278
.L349:
	inc	edx
	mov	cl, BYTE [edx]
.L912:
	cmp	cl, 56
	je	.L369
	jmp	.L342
.L357:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 45
	je	.L878
	jmp	.L909
.L359:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 65
	je	.L882
	cmp	cl, 97
	jne	.L342
.L882:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 78
	je	.L885
	cmp	cl, 110
	jne	.L342
.L885:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 73
	je	.L888
	cmp	cl, 105
	jne	.L342
.L888:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 85
	je	.L891
	cmp	cl, 117
	jne	.L342
.L891:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 77
	je	.L894
	cmp	cl, 109
.L920:
	jne	.L342
.L894:
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	mov	DWORD [cpu_enabled], 10023423
	jmp	.L278
.L878:
	inc	edx
	mov	cl, BYTE [edx]
.L909:
	cmp	cl, 54
	jne	.L342
.L880:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 52
	jmp	.L920
.L341:
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 51
	jg	.L899
.L925:
	cmp	cl, 48
	jle	.L342
	cmp	cl, 49
	jle	.L285
	cmp	cl, 50
	jle	.L287
	jmp	.L349
.L899:
	cmp	cl, 52
	jle	.L291
	cmp	cl, 56
	jne	.L342
	inc	edx
	mov	cl, BYTE [edx]
	cmp	cl, 54
	jne	.L342
	inc	edx
	cmp	BYTE [edx], 0
	jg	.L338
	mov	DWORD [cpu_enabled], 8388608
.L278:
	mov	ebx, DWORD [ebp-4]
	leave
	ret
.Lfe3:
	;.size	yasm_x86__parse_cpu,.Lfe3-yasm_x86__parse_cpu
	[section	.rodata];.str1.32
	[align 32]
LC16:
	db	"`s' segment register ignored in 64-bit mode", 0
	[align 32]
LC18:
	db	"Cannot override address size to 16 bits in 64-bit mode", 0
	[align 32]
LC17:
	db	"`s' is a prefix in 64-bit mode", 0
	[align 32]
LC15:
	db	"`s' is a register in 64-bit mode", 0
	[align 32]
LC13:
	db	"`s' is an instruction in 64-bit mode", 0
	[section	.rodata];.str1.1
LC14:
	db	"`s' invalid in 64-bit mode", 0
	[section .text]
[global yasm_x86__parse_check_id]
	;.type	yasm_x86__parse_check_id,@function
yasm_x86__parse_check_id:
	push	ebp
	mov	ebp, esp
	push	edi
	push	esi
	push	ebx
	sub	esp, 12
	mov	edi, DWORD [ebp+8]
	mov	ecx, DWORD [ebp+12]
	mov	esi, DWORD [ebp+16]
	mov	ebx, ecx
.L927:
	movsx	eax, BYTE [ecx]
	cmp	eax, 120
	ja	.L999
	jmp	DWORD [.L1000+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1000:
	dd	.L8702
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L934
	dd	.L937
	dd	.L940
	dd	.L943
	dd	.L946
	dd	.L949
	dd	.L952
	dd	.L955
	dd	.L958
	dd	.L961
	dd	.L999
	dd	.L964
	dd	.L967
	dd	.L970
	dd	.L973
	dd	.L976
	dd	.L999
	dd	.L979
	dd	.L982
	dd	.L985
	dd	.L988
	dd	.L991
	dd	.L994
	dd	.L997
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L999
	dd	.L934
	dd	.L937
	dd	.L940
	dd	.L943
	dd	.L946
	dd	.L949
	dd	.L952
	dd	.L955
	dd	.L958
	dd	.L961
	dd	.L999
	dd	.L964
	dd	.L967
	dd	.L970
	dd	.L973
	dd	.L976
	dd	.L999
	dd	.L979
	dd	.L982
	dd	.L985
	dd	.L988
	dd	.L991
	dd	.L994
	dd	.L997
	[section .text]
.L970:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	jg	.L1001
	cmp	dl, 69
	je	.L1003
	cmp	dl, 78
	jle	.L1005
	jmp	.L1006
.L1001:
	cmp	dl, 101
	jg	.L1008
	cmp	dl, 100
	jle	.L1005
	jmp	.L1003
.L1008:
	cmp	dl, 111
	je	.L1006
	jmp	.L1005
.L1012:
.L982:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 56
	ja	.L1005
	jmp	DWORD [.L1069+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1069:
	dd	.L1016
	dd	.L1019
	dd	.L1022
	dd	.L1005
	dd	.L1025
	dd	.L1028
	dd	.L1031
	dd	.L1034
	dd	.L1037
	dd	.L1005
	dd	.L1005
	dd	.L1040
	dd	.L1043
	dd	.L1005
	dd	.L1005
	dd	.L1046
	dd	.L1049
	dd	.L1005
	dd	.L1052
	dd	.L1055
	dd	.L1058
	dd	.L1061
	dd	.L1064
	dd	.L1005
	dd	.L1067
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1016
	dd	.L1019
	dd	.L1022
	dd	.L1005
	dd	.L1025
	dd	.L1028
	dd	.L1031
	dd	.L1034
	dd	.L1037
	dd	.L1005
	dd	.L1005
	dd	.L1040
	dd	.L1043
	dd	.L1005
	dd	.L1005
	dd	.L1046
	dd	.L1049
	dd	.L1005
	dd	.L1052
	dd	.L1055
	dd	.L1058
	dd	.L1061
	dd	.L1064
	dd	.L1005
	dd	.L1067
	[section .text]
.L949:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 50
	cmp	eax, 71
	ja	.L1005
	jmp	DWORD [.L1128+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1128:
	dd	.L1072
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1075
	dd	.L1078
	dd	.L1081
	dd	.L1084
	dd	.L1087
	dd	.L1090
	dd	.L1005
	dd	.L1005
	dd	.L1093
	dd	.L1005
	dd	.L1005
	dd	.L1096
	dd	.L1099
	dd	.L1102
	dd	.L1005
	dd	.L1105
	dd	.L1005
	dd	.L1108
	dd	.L1111
	dd	.L1114
	dd	.L1117
	dd	.L1005
	dd	.L1120
	dd	.L1123
	dd	.L1126
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1075
	dd	.L1078
	dd	.L1081
	dd	.L1084
	dd	.L1087
	dd	.L1090
	dd	.L1005
	dd	.L1005
	dd	.L1093
	dd	.L1005
	dd	.L1005
	dd	.L1096
	dd	.L1099
	dd	.L1102
	dd	.L1005
	dd	.L1105
	dd	.L1005
	dd	.L1108
	dd	.L1111
	dd	.L1114
	dd	.L1117
	dd	.L1005
	dd	.L1120
	dd	.L1123
	dd	.L1126
	[section .text]
.L985:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L1129
	cmp	dl, 78
	jg	.L1130
	cmp	dl, 69
	jmp	.L8674
.L1130:
	cmp	dl, 79
	jle	.L1135
	cmp	dl, 81
	jle	.L1005
	jmp	.L1137
.L1129:
	cmp	dl, 110
	jg	.L1139
	cmp	dl, 101
.L8674:
	je	.L1132
	jmp	.L1005
.L1139:
	cmp	dl, 111
	jle	.L1135
	cmp	dl, 114
	je	.L1137
	jmp	.L1005
.L973:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	jg	.L1144
	cmp	dl, 50
	jg	.L1145
	cmp	dl, 49
	je	.L1147
	jmp	.L1005
.L1145:
	cmp	dl, 51
	jle	.L1150
	cmp	dl, 54
	je	.L1152
	jmp	.L1005
.L1144:
	cmp	dl, 113
	jg	.L1154
	cmp	dl, 82
	jle	.L1156
	cmp	dl, 85
	jmp	.L8673
.L1154:
	cmp	dl, 114
	jle	.L1156
	cmp	dl, 117
.L8673:
	je	.L1158
	jmp	.L1005
.L934:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 49
	cmp	eax, 71
	ja	.L1005
	jmp	DWORD [.L1191+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1191:
	dd	.L1164
	dd	.L1005
	dd	.L1166
	dd	.L1005
	dd	.L1005
	dd	.L1168
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1171
	dd	.L1005
	dd	.L1005
	dd	.L1174
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1177
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1180
	dd	.L1005
	dd	.L1183
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1186
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1189
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1171
	dd	.L1005
	dd	.L1005
	dd	.L1174
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1177
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1180
	dd	.L1005
	dd	.L1183
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1186
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1189
	[section .text]
.L964:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 51
	ja	.L1005
	jmp	DWORD [.L1227+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1227:
	dd	.L1195
	dd	.L1005
	dd	.L1005
	dd	.L1198
	dd	.L1201
	dd	.L1204
	dd	.L1207
	dd	.L1005
	dd	.L1210
	dd	.L1005
	dd	.L1005
	dd	.L1213
	dd	.L1216
	dd	.L1005
	dd	.L1219
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1222
	dd	.L1225
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1195
	dd	.L1005
	dd	.L1005
	dd	.L1198
	dd	.L1201
	dd	.L1204
	dd	.L1207
	dd	.L1005
	dd	.L1210
	dd	.L1005
	dd	.L1005
	dd	.L1213
	dd	.L1216
	dd	.L1005
	dd	.L1219
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1222
	dd	.L1225
	[section .text]
.L979:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 49
	cmp	eax, 66
	ja	.L1005
	jmp	DWORD [.L1259+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1259:
	dd	.L1230
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1233
	dd	.L1233
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1236
	dd	.L1239
	dd	.L1242
	dd	.L1245
	dd	.L1248
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1251
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1254
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1257
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1236
	dd	.L1239
	dd	.L1242
	dd	.L1245
	dd	.L1248
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1251
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1254
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1257
	[section .text]
.L940:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 55
	ja	.L1005
	jmp	DWORD [.L1301+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1301:
	dd	.L1263
	dd	.L1266
	dd	.L1005
	dd	.L1269
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1272
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1275
	dd	.L1278
	dd	.L1005
	dd	.L1281
	dd	.L1284
	dd	.L1005
	dd	.L1287
	dd	.L1290
	dd	.L1005
	dd	.L1005
	dd	.L1293
	dd	.L1296
	dd	.L1299
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1263
	dd	.L1266
	dd	.L1005
	dd	.L1269
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1272
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1275
	dd	.L1278
	dd	.L1005
	dd	.L1281
	dd	.L1284
	dd	.L1005
	dd	.L1287
	dd	.L1290
	dd	.L1005
	dd	.L1005
	dd	.L1293
	dd	.L1296
	dd	.L1299
	[section .text]
.L943:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 55
	ja	.L1005
	jmp	DWORD [.L1328+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1328:
	dd	.L1305
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1308
	dd	.L1005
	dd	.L1005
	dd	.L1311
	dd	.L1314
	dd	.L1005
	dd	.L1005
	dd	.L1317
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1320
	dd	.L1323
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1326
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1305
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1308
	dd	.L1005
	dd	.L1005
	dd	.L1311
	dd	.L1314
	dd	.L1005
	dd	.L1005
	dd	.L1317
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1320
	dd	.L1323
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1326
	[section .text]
.L967:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 52
	ja	.L1005
	jmp	DWORD [.L1349+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1349:
	dd	.L1332
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1335
	dd	.L1005
	dd	.L1005
	dd	.L1338
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1341
	dd	.L1005
	dd	.L1344
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1347
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1332
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1335
	dd	.L1005
	dd	.L1005
	dd	.L1338
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1341
	dd	.L1005
	dd	.L1344
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1347
	[section .text]
.L997:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 46
	ja	.L1005
	jmp	DWORD [.L1370+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1370:
	dd	.L1353
	dd	.L1356
	dd	.L1359
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1362
	dd	.L1365
	dd	.L1005
	dd	.L1368
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1353
	dd	.L1356
	dd	.L1359
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1362
	dd	.L1365
	dd	.L1005
	dd	.L1368
	[section .text]
.L946:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 50
	ja	.L1005
	jmp	DWORD [.L1394+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1394:
	dd	.L1374
	dd	.L1377
	dd	.L1380
	dd	.L1383
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1386
	dd	.L1389
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1392
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1374
	dd	.L1377
	dd	.L1380
	dd	.L1383
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1386
	dd	.L1389
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1392
	[section .text]
.L937:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 72
	cmp	eax, 48
	ja	.L1005
	jmp	DWORD [.L1418+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1418:
	dd	.L1398
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1401
	dd	.L1005
	dd	.L1005
	dd	.L1404
	dd	.L1407
	dd	.L1005
	dd	.L1005
	dd	.L1410
	dd	.L1413
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1416
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1398
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1401
	dd	.L1005
	dd	.L1005
	dd	.L1404
	dd	.L1407
	dd	.L1005
	dd	.L1005
	dd	.L1410
	dd	.L1413
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1416
	[section .text]
.L952:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L1420
	cmp	dl, 115
	je	.L1420
	jmp	.L1005
.L976:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 55
	ja	.L1005
	jmp	DWORD [.L1457+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1457:
	dd	.L1425
	dd	.L1005
	dd	.L1005
	dd	.L1428
	dd	.L1431
	dd	.L1434
	dd	.L1005
	dd	.L1005
	dd	.L1437
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1440
	dd	.L1005
	dd	.L1443
	dd	.L1005
	dd	.L1005
	dd	.L1446
	dd	.L1449
	dd	.L1005
	dd	.L1452
	dd	.L1005
	dd	.L1005
	dd	.L1455
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1425
	dd	.L1005
	dd	.L1005
	dd	.L1428
	dd	.L1431
	dd	.L1434
	dd	.L1005
	dd	.L1005
	dd	.L1437
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1440
	dd	.L1005
	dd	.L1443
	dd	.L1005
	dd	.L1005
	dd	.L1446
	dd	.L1449
	dd	.L1005
	dd	.L1452
	dd	.L1005
	dd	.L1005
	dd	.L1455
	[section .text]
.L958:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 66
	cmp	eax, 48
	ja	.L1005
	jmp	DWORD [.L1475+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1475:
	dd	.L1461
	dd	.L1005
	dd	.L1464
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1467
	dd	.L1470
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1473
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1461
	dd	.L1005
	dd	.L1464
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1467
	dd	.L1470
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1473
	[section .text]
.L961:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 57
	ja	.L1005
	jmp	DWORD [.L1517+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1517:
	dd	.L1479
	dd	.L1482
	dd	.L1485
	dd	.L1005
	dd	.L1488
	dd	.L1005
	dd	.L1491
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1494
	dd	.L1497
	dd	.L1500
	dd	.L1503
	dd	.L1506
	dd	.L1005
	dd	.L1509
	dd	.L1512
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1515
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1479
	dd	.L1482
	dd	.L1485
	dd	.L1005
	dd	.L1488
	dd	.L1005
	dd	.L1491
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1494
	dd	.L1497
	dd	.L1500
	dd	.L1503
	dd	.L1506
	dd	.L1005
	dd	.L1509
	dd	.L1512
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1515
	[section .text]
.L955:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L1519
	cmp	dl, 108
	je	.L1519
	jmp	.L1005
.L991:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L1522
	cmp	dl, 101
	je	.L1522
	jmp	.L1005
.L994:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L1524
	cmp	dl, 65
	jg	.L1525
	cmp	dl, 64
	jle	.L1005
	jmp	.L1527
.L1525:
	cmp	dl, 66
	jle	.L1530
	cmp	dl, 81
	jle	.L1005
	jmp	.L1532
.L1524:
	cmp	dl, 98
	jg	.L1534
	cmp	dl, 96
	jle	.L1005
	cmp	dl, 97
	jle	.L1527
	jmp	.L1530
.L1534:
	cmp	dl, 114
	je	.L1532
	jmp	.L1005
.L988:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	jg	.L1539
	cmp	dl, 68
	jg	.L1540
	cmp	dl, 66
	jle	.L1005
	cmp	dl, 67
	jmp	.L8675
.L1540:
	cmp	dl, 76
	jle	.L1005
	cmp	dl, 77
	jle	.L1548
	jmp	.L1549
.L1539:
	cmp	dl, 100
	jg	.L1551
	cmp	dl, 98
	jle	.L1005
	cmp	dl, 99
.L8675:
	jle	.L1543
	jmp	.L1544
.L1551:
	cmp	dl, 108
	jle	.L1005
	cmp	dl, 109
	jle	.L1548
	cmp	dl, 110
	jle	.L1549
	jmp	.L1005
.L999:
	inc	ecx
	mov	dl, BYTE [ecx]
.L1005:
	mov	eax, 0
	test	dl, dl
	jle	.L926
	jmp	.L999
.L931:
.L1543:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	je	.L1561
	cmp	dl, 111
	je	.L1561
	jmp	.L1005
.L1544:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 48
	jle	.L1005
	cmp	dl, 49
	jle	.L1565
	cmp	dl, 50
	jle	.L1567
	jmp	.L1005
.L1548:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	je	.L1569
	cmp	dl, 111
	je	.L1569
	jmp	.L1005
.L1549:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L1572
	cmp	dl, 112
	jne	.L1005
.L1572:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L1575
	cmp	dl, 99
	jne	.L1005
.L1575:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 75
	je	.L1578
	cmp	dl, 107
	jne	.L1005
.L1578:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	jg	.L1580
	cmp	dl, 72
	je	.L1582
	cmp	dl, 75
	jle	.L1005
	jmp	.L1584
.L1580:
	cmp	dl, 104
	jg	.L1586
	cmp	dl, 103
	jle	.L1005
	jmp	.L1582
.L1586:
	cmp	dl, 108
	je	.L1584
	jmp	.L1005
.L1582:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L1591
	cmp	dl, 112
	je	.L1591
	jmp	.L1005
.L1584:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L1594
	cmp	dl, 112
	jne	.L1005
.L1594:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L1596
	cmp	dl, 68
	je	.L1598
	cmp	dl, 82
	jle	.L1005
	jmp	.L1600
.L1596:
	cmp	dl, 100
	jg	.L1602
	cmp	dl, 99
	jle	.L1005
	jmp	.L1598
.L1602:
	cmp	dl, 115
	je	.L1600
	jmp	.L1005
.L1598:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6689793
	jmp	.L9151
.L1600:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 5121
	jmp	.L9152
.L1591:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L1612
	cmp	dl, 68
	je	.L1614
	cmp	dl, 82
	jle	.L1005
	jmp	.L1616
.L1612:
	cmp	dl, 100
	jg	.L1618
	cmp	dl, 99
	jle	.L1005
	jmp	.L1614
.L1618:
	cmp	dl, 115
	je	.L1616
	jmp	.L1005
.L1614:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6690049
	jmp	.L9151
.L1616:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 5377
	jmp	.L9152
.L1569:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L1629
	cmp	dl, 118
	jne	.L1005
.L1629:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], umov_insn
	mov	DWORD [edi+4], 6
	jmp	.L9016
.L1567:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 985857
	jmp	.L9153
.L1565:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 1030401
	jmp	.L8709
.L1561:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L1641
	cmp	dl, 109
	jne	.L1005
.L1641:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L1644
	cmp	dl, 105
	jne	.L1005
.L1644:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L1647
	cmp	dl, 115
	jne	.L1005
.L1647:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L1649
	cmp	dl, 68
	je	.L1651
	cmp	dl, 82
	jle	.L1005
	jmp	.L1653
.L1649:
	cmp	dl, 100
	jg	.L1654
	cmp	dl, 99
	jle	.L1005
	jmp	.L1651
.L1654:
	cmp	dl, 115
	jne	.L1005
.L1653:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15937025
	jmp	.L9152
.L1651:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15871489
	jmp	.L9151
.L1527:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L1665
	cmp	dl, 105
	je	.L1665
	jmp	.L1005
.L1530:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L1668
	cmp	dl, 105
	je	.L1668
	jmp	.L1005
.L1532:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L1670
	cmp	dl, 77
	je	.L1672
	cmp	dl, 82
	jle	.L1005
	jmp	.L1674
.L1670:
	cmp	dl, 109
	jg	.L1676
	cmp	dl, 108
	jle	.L1005
	jmp	.L1672
.L1676:
	cmp	dl, 115
	je	.L1674
	jmp	.L1005
.L1672:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L1681
	cmp	dl, 115
	je	.L1681
	jmp	.L1005
.L1674:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 72
	je	.L1684
	cmp	dl, 104
	jne	.L1005
.L1684:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L1687
	cmp	dl, 114
	jne	.L1005
.L1687:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 997121
	jmp	.L8712
.L1681:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L1693
	cmp	dl, 114
	jne	.L1005
.L1693:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 995329
	jmp	.L8713
.L1668:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L1699
	cmp	dl, 110
	jne	.L1005
.L1699:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L1702
	cmp	dl, 118
	jne	.L1005
.L1702:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L1705
	cmp	dl, 100
	jne	.L1005
.L1705:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 985345
	jmp	.L8721
.L1665:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L1711
	cmp	dl, 116
	jne	.L1005
.L1711:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 39681
	jmp	.L8695
.L1522:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L1717
	cmp	dl, 114
	jne	.L1005
.L1717:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L1719
	cmp	dl, 82
	je	.L1721
	cmp	dl, 86
	jle	.L1005
	jmp	.L1723
.L1719:
	cmp	dl, 114
	jg	.L1725
	cmp	dl, 113
	jle	.L1005
	jmp	.L1721
.L1725:
	cmp	dl, 119
	je	.L1723
	jmp	.L1005
.L1721:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], prot286_insn
	mov	DWORD [edi+4], 262145
	jmp	.L9031
.L1723:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], prot286_insn
	mov	DWORD [edi+4], 327681
	jmp	.L9031
.L1519:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L1736
	cmp	dl, 116
	jne	.L1005
.L1736:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 62465
	mov	DWORD [edi+8], 8388608
	jmp	.L8696
.L1479:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L1741
	test	dl, dl
	jle	.L9238
	cmp	dl, 68
	jle	.L999
	jmp	.L1745
.L1741:
	cmp	dl, 101
	je	.L1745
	jmp	.L999
.L1743:
.L1482:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L1750
	test	dl, dl
	jle	.L9237
	cmp	dl, 68
	jle	.L999
	jmp	.L1754
.L1750:
	cmp	dl, 101
	je	.L1754
	jmp	.L999
.L1752:
.L1485:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	jg	.L1759
	test	dl, dl
	jle	.L9237
	cmp	dl, 87
	jle	.L999
	jmp	.L1763
.L1759:
	cmp	dl, 120
	je	.L1763
	jmp	.L999
.L1761:
.L1488:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	jg	.L1768
	test	dl, dl
	jle	.L9236
	cmp	dl, 66
	jle	.L999
	jmp	.L1772
.L1768:
	cmp	dl, 99
	je	.L1772
	jmp	.L999
.L1770:
.L1491:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L1777
	test	dl, dl
	jle	.L9235
	cmp	dl, 68
	jle	.L999
	jmp	.L1781
.L1777:
	cmp	dl, 101
	je	.L1781
	jmp	.L999
.L1779:
.L1494:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L1786
	test	dl, dl
	jle	.L9234
	cmp	dl, 68
	jle	.L999
	jmp	.L1790
.L1786:
	cmp	dl, 101
	je	.L1790
	jmp	.L999
.L1788:
.L1497:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L1796
	cmp	dl, 112
	je	.L1796
	jmp	.L1005
.L1500:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 57
	ja	.L1005
	jmp	DWORD [.L1830+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L1830:
	dd	.L1801
	dd	.L1804
	dd	.L1745
	dd	.L1005
	dd	.L1828
	dd	.L1005
	dd	.L1813
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1816
	dd	.L1005
	dd	.L1005
	dd	.L1819
	dd	.L1822
	dd	.L1005
	dd	.L1005
	dd	.L1825
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1828
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1801
	dd	.L1804
	dd	.L1745
	dd	.L1005
	dd	.L1828
	dd	.L1005
	dd	.L1813
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1816
	dd	.L1005
	dd	.L1005
	dd	.L1819
	dd	.L1822
	dd	.L1005
	dd	.L1005
	dd	.L1825
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1828
	[section .text]
.L1503:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 7
	jmp	.L8695
.L1506:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	jg	.L1834
	cmp	dl, 68
	jg	.L1835
	test	dl, dl
	jmp	.L9239
.L1835:
	cmp	dl, 69
	jle	.L1839
	cmp	dl, 78
	jle	.L999
	jmp	.L1822
.L1834:
	cmp	dl, 101
	jg	.L1843
	cmp	dl, 100
	jle	.L999
	jmp	.L1839
.L1843:
	cmp	dl, 111
	je	.L1822
	jmp	.L999
.L1509:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L1850
	cmp	dl, 99
	je	.L1850
	jmp	.L1005
.L1512:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 2055
	jmp	.L8695
.L1515:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9236:
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 1031
	jmp	.L8695
.L1850:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L1859
	cmp	dl, 120
	jne	.L1005
.L1859:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	je	.L1862
	cmp	dl, 122
	jne	.L1005
.L1862:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], jcxz_insn
	mov	DWORD [edi+4], 16386
	jmp	.L9148
.L1839:
	inc	ecx
	cmp	BYTE [ecx], 0
.L9239:
	jg	.L999
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 2567
	jmp	.L8695
.L1841:
.L1819:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 263
	jmp	.L8695
.L1801:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L1877
	test	dl, dl
	jle	.L9231
	cmp	dl, 68
	jle	.L999
	jmp	.L1881
.L1877:
	cmp	dl, 101
	je	.L1881
	jmp	.L999
.L1879:
.L1804:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L1886
	test	dl, dl
	jle	.L9230
	cmp	dl, 68
	jle	.L999
	jmp	.L1890
.L1886:
	cmp	dl, 101
	je	.L1890
	jmp	.L999
.L1888:
.L1807:
.L1810:
.L1828:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 1287
	jmp	.L8695
.L1825:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 2311
	jmp	.L8695
.L1822:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 2823
	jmp	.L8695
.L1813:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L1910
	test	dl, dl
	jle	.L9228
	cmp	dl, 68
	jle	.L999
	jmp	.L1914
.L1910:
	cmp	dl, 101
	je	.L1914
	jmp	.L999
.L1912:
.L1816:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L1919
	test	dl, dl
	jle	.L9227
	cmp	dl, 68
	jle	.L999
	jmp	.L1923
.L1919:
	cmp	dl, 101
	jne	.L999
.L1921:
.L1923:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9235:
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 3847
	jmp	.L8695
.L1914:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9234:
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 3079
	jmp	.L8695
.L1890:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9238:
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 1799
	jmp	.L8695
.L1881:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9237:
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 519
	jmp	.L8695
.L1796:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], jmp_insn
	jmp	.L9226
.L1790:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9228:
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 3591
	jmp	.L8695
.L1781:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9227:
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 3335
	jmp	.L8695
.L1772:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L1950
	cmp	dl, 120
	jne	.L1005
.L1950:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	je	.L1953
	cmp	dl, 122
	jne	.L1005
.L1953:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], jcxz_insn
	mov	DWORD [edi+4], 8194
	jmp	.L9186
.L1763:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	je	.L1959
	cmp	dl, 122
	jne	.L1005
.L1959:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], jcxz_insn
	mov	DWORD [edi+4], 4098
	jmp	.L8695
.L1754:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9231:
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 1543
	jmp	.L8695
.L1745:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9230:
	mov	DWORD [edi], jcc_insn
	mov	DWORD [edi+4], 775
	jmp	.L8695
.L1461:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L1971
	cmp	dl, 116
	je	.L1971
	jmp	.L1005
.L1464:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L1974
	cmp	dl, 105
	je	.L1974
	jmp	.L1005
.L1467:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	je	.L1977
	cmp	dl, 117
	je	.L1977
	jmp	.L1005
.L1470:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	jg	.L1979
	cmp	dl, 82
	jg	.L1980
	test	dl, dl
	jle	.L1982
	cmp	dl, 67
	je	.L1984
	jmp	.L999
.L1980:
	cmp	dl, 83
	jle	.L1987
	cmp	dl, 84
	jle	.L1989
	cmp	dl, 85
	jle	.L999
	jmp	.L1991
.L1979:
	cmp	dl, 115
	jg	.L1993
	cmp	dl, 99
	je	.L1984
	cmp	dl, 114
	jle	.L999
	jmp	.L1987
.L1993:
	cmp	dl, 116
	jle	.L1989
	cmp	dl, 118
	je	.L1991
	jmp	.L999
.L1982:
	mov	DWORD [edi], in_insn
	jmp	.L9225
.L1473:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L2002
	cmp	dl, 101
	jne	.L1005
.L2002:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L2005
	cmp	dl, 116
	jne	.L1005
.L2005:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2007
	cmp	dl, 68
	jg	.L2008
	test	dl, dl
	jle	.L2010
	cmp	dl, 67
	jle	.L999
	jmp	.L2012
.L2008:
	cmp	dl, 81
	je	.L2015
	cmp	dl, 86
	jle	.L999
	jmp	.L2017
.L2007:
	cmp	dl, 112
	jg	.L2019
	cmp	dl, 100
	je	.L2012
	jmp	.L999
.L2019:
	cmp	dl, 113
	jle	.L2015
	cmp	dl, 119
	je	.L2017
	jmp	.L999
.L2010:
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 52993
	jmp	.L8695
.L2017:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1101569
	jmp	.L8695
.L2012:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2150145
	jmp	.L9186
.L2015:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 4247297
	jmp	.L9148
.L1984:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], incdec_insn
	mov	DWORD [edi+4], 16390
	jmp	.L8695
.L1987:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2039
	cmp	dl, 67
	jg	.L2040
	cmp	dl, 66
	jmp	.L8672
.L2040:
	cmp	dl, 68
	jle	.L2045
	cmp	dl, 86
	jle	.L1005
	jmp	.L2047
.L2039:
	cmp	dl, 99
	jg	.L2049
	cmp	dl, 98
.L8672:
	je	.L2042
	jmp	.L1005
.L2049:
	cmp	dl, 100
	jle	.L2045
	cmp	dl, 119
	je	.L2047
	jmp	.L1005
.L1989:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 51
	jg	.L2054
	cmp	dl, 47
	jg	.L2055
	test	dl, dl
	jg	.L999
	jmp	.L2062
.L2055:
	cmp	dl, 48
	jle	.L2059
	cmp	dl, 50
	jle	.L999
	jmp	.L2093
.L2054:
	cmp	dl, 79
	jg	.L2063
	cmp	dl, 78
	jle	.L999
	jmp	.L2065
.L2063:
	cmp	dl, 111
	je	.L2065
	jmp	.L999
.L2062:
	mov	DWORD [edi], int_insn
	mov	DWORD [edi+4], 1
	jmp	.L8695
.L1991:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	jg	.L2070
	cmp	dl, 68
	je	.L2072
	cmp	dl, 75
	jle	.L1005
	jmp	.L2074
.L2070:
	cmp	dl, 100
	jg	.L2076
	cmp	dl, 99
	jle	.L1005
	jmp	.L2072
.L2076:
	cmp	dl, 108
	je	.L2074
	jmp	.L1005
.L2072:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 985089
	jmp	.L8721
.L2074:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L2084
	cmp	dl, 112
	jne	.L1005
.L2084:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 71
	je	.L2087
	cmp	dl, 103
	jne	.L1005
.L2087:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 118423809
.L8721:
	mov	DWORD [edi+8], 8388616
	jmp	.L8696
.L2059:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 51
	je	.L2093
	jmp	.L1005
.L2061:
.L2065:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 52737
	jmp	.L8695
.L2093:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 52225
	jmp	.L8695
.L2042:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 27649
	jmp	.L8695
.L2045:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2125057
	jmp	.L9186
.L2047:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1076481
	jmp	.L8695
.L1977:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L2116
	cmp	dl, 108
	jne	.L1005
.L2116:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], imul_insn
	mov	DWORD [edi+4], 19
	jmp	.L8695
.L1974:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L2122
	cmp	dl, 118
	jne	.L1005
.L2122:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], f6_insn
	mov	DWORD [edi+4], 1796
	jmp	.L8695
.L1971:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L2128
	cmp	dl, 115
	jne	.L1005
.L2128:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ibts_insn
	jmp	.L8724
.L1425:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	jg	.L2133
	cmp	dl, 77
	jg	.L2134
	cmp	dl, 66
	jle	.L1005
	cmp	dl, 67
	jle	.L2137
	cmp	dl, 68
	jmp	.L8671
.L2134:
	cmp	dl, 78
	jle	.L2142
	cmp	dl, 84
	jle	.L1005
	cmp	dl, 85
	jle	.L2145
	jmp	.L2146
.L2133:
	cmp	dl, 109
	jg	.L2148
	cmp	dl, 98
	jle	.L1005
	cmp	dl, 99
	jle	.L2137
	cmp	dl, 100
.L8671:
	jle	.L2139
	jmp	.L1005
.L2148:
	cmp	dl, 116
	jg	.L2153
	cmp	dl, 110
	jle	.L2142
	jmp	.L1005
.L2153:
	cmp	dl, 117
	jle	.L2145
	cmp	dl, 118
	jle	.L2146
	jmp	.L1005
.L1428:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L2159
	cmp	dl, 105
	je	.L2159
	jmp	.L1005
.L1431:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L2162
	cmp	dl, 120
	je	.L2162
	jmp	.L1005
.L1434:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 50
	cmp	eax, 65
	ja	.L1005
	jmp	DWORD [.L2189+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L2189:
	dd	.L2166
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2169
	dd	.L1005
	dd	.L2172
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2175
	dd	.L2178
	dd	.L1005
	dd	.L2181
	dd	.L1005
	dd	.L2184
	dd	.L2187
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2169
	dd	.L1005
	dd	.L2172
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2175
	dd	.L2178
	dd	.L1005
	dd	.L2181
	dd	.L1005
	dd	.L2184
	dd	.L2187
	[section .text]
.L1437:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	jg	.L2190
	cmp	dl, 50
	je	.L2192
	jmp	.L1005
.L2190:
	cmp	dl, 78
	jle	.L2195
	cmp	dl, 110
	je	.L2195
	jmp	.L1005
.L1440:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	jg	.L2197
	cmp	dl, 73
	jg	.L2198
	cmp	dl, 65
	je	.L2200
	cmp	dl, 72
	jle	.L1005
	jmp	.L2202
.L2198:
	cmp	dl, 79
	jg	.L2204
	cmp	dl, 78
	jle	.L1005
	jmp	.L2206
.L2204:
	cmp	dl, 84
	jle	.L1005
	cmp	dl, 85
	jle	.L2210
	jmp	.L2211
.L2197:
	cmp	dl, 110
	jg	.L2213
	cmp	dl, 97
	jg	.L2214
	cmp	dl, 96
	jle	.L1005
	jmp	.L2200
.L2214:
	cmp	dl, 105
	je	.L2202
	jmp	.L1005
.L2213:
	cmp	dl, 116
	jg	.L2219
	cmp	dl, 111
	jle	.L2206
	jmp	.L1005
.L2219:
	cmp	dl, 117
	jle	.L2210
	cmp	dl, 118
	jle	.L2211
	jmp	.L1005
.L1443:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L2224
	cmp	dl, 80
	je	.L2226
	cmp	dl, 81
	jle	.L1005
	jmp	.L2228
.L2224:
	cmp	dl, 112
	jg	.L2230
	cmp	dl, 111
	jle	.L1005
	jmp	.L2226
.L2230:
	cmp	dl, 114
	je	.L2228
	jmp	.L1005
.L1446:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L2235
	cmp	dl, 101
	je	.L2235
	jmp	.L1005
.L1449:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 54
	ja	.L1005
	jmp	DWORD [.L2257+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L2257:
	dd	.L2240
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2243
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2246
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2249
	dd	.L1005
	dd	.L1005
	dd	.L2252
	dd	.L1005
	dd	.L2255
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2240
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2243
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2246
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2249
	dd	.L1005
	dd	.L1005
	dd	.L2252
	dd	.L1005
	dd	.L2255
	[section .text]
.L1452:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L2258
	cmp	dl, 78
	je	.L2260
	cmp	dl, 82
	jle	.L1005
	jmp	.L2262
.L2258:
	cmp	dl, 110
	jg	.L2264
	cmp	dl, 109
	jle	.L1005
	jmp	.L2260
.L2264:
	cmp	dl, 115
	je	.L2262
	jmp	.L1005
.L1455:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	je	.L2269
	cmp	dl, 111
	jne	.L1005
.L2269:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L2272
	cmp	dl, 114
	jne	.L1005
.L2272:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 61186
	jmp	.L8866
.L2262:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 72
	je	.L2278
	cmp	dl, 104
	je	.L2278
	jmp	.L1005
.L2260:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L2281
	cmp	dl, 112
	jne	.L1005
.L2281:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L2284
	cmp	dl, 99
	jne	.L1005
.L2284:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 75
	je	.L2287
	cmp	dl, 107
	jne	.L1005
.L2287:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	jg	.L2289
	cmp	dl, 72
	je	.L2291
	cmp	dl, 75
	jle	.L1005
	jmp	.L2293
.L2289:
	cmp	dl, 104
	jg	.L2295
	cmp	dl, 103
	jle	.L1005
	jmp	.L2291
.L2295:
	cmp	dl, 108
	je	.L2293
	jmp	.L1005
.L2291:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2299
	cmp	dl, 68
	jg	.L2300
	cmp	dl, 66
	je	.L2302
	cmp	dl, 67
	jmp	.L8677
.L2300:
	cmp	dl, 81
	je	.L2307
	cmp	dl, 86
	jle	.L1005
	jmp	.L2309
.L2299:
	cmp	dl, 100
	jg	.L2311
	cmp	dl, 98
	je	.L2302
	cmp	dl, 99
.L8677:
	jle	.L1005
	jmp	.L2304
.L2311:
	cmp	dl, 113
	jg	.L2315
	cmp	dl, 112
	jle	.L1005
	jmp	.L2307
.L2315:
	cmp	dl, 119
	je	.L2309
	jmp	.L1005
.L2293:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2319
	cmp	dl, 68
	jg	.L2320
	cmp	dl, 66
	je	.L2322
	cmp	dl, 67
	jmp	.L8676
.L2320:
	cmp	dl, 81
	je	.L2327
	cmp	dl, 86
	jle	.L1005
	jmp	.L2329
.L2319:
	cmp	dl, 100
	jg	.L2331
	cmp	dl, 98
	je	.L2322
	cmp	dl, 99
.L8676:
	jle	.L1005
	jmp	.L2324
.L2331:
	cmp	dl, 113
	jg	.L2335
	cmp	dl, 112
	jle	.L1005
	jmp	.L2327
.L2335:
	cmp	dl, 119
	je	.L2329
	jmp	.L1005
.L2322:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L2340
	cmp	dl, 119
	je	.L2340
	jmp	.L1005
.L2324:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L2343
	cmp	dl, 113
	je	.L2343
	jmp	.L1005
.L2327:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L2346
	cmp	dl, 100
	je	.L2346
	jmp	.L1005
.L2329:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L2349
	cmp	dl, 100
	jne	.L1005
.L2349:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 24834
	jmp	.L8866
.L2346:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L2355
	cmp	dl, 113
	jne	.L1005
.L2355:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6712321
	jmp	.L9151
.L2343:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 25090
	jmp	.L8866
.L2340:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 24578
	jmp	.L8866
.L2302:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L2367
	cmp	dl, 119
	je	.L2367
	jmp	.L1005
.L2304:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L2370
	cmp	dl, 113
	je	.L2370
	jmp	.L1005
.L2307:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L2373
	cmp	dl, 100
	je	.L2373
	jmp	.L1005
.L2309:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L2376
	cmp	dl, 100
	jne	.L1005
.L2376:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 26882
	jmp	.L8866
.L2373:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L2382
	cmp	dl, 113
	jne	.L1005
.L2382:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6712577
	jmp	.L9151
.L2370:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 27138
	jmp	.L8866
.L2367:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 26626
	jmp	.L8866
.L2278:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 70
	jg	.L2393
	cmp	dl, 64
	jg	.L2394
	test	dl, dl
	jg	.L999
	jmp	.L2401
.L2394:
	cmp	dl, 65
	jle	.L2398
	cmp	dl, 69
	jle	.L999
	jmp	.L2400
.L2393:
	cmp	dl, 97
	jg	.L2402
	cmp	dl, 96
	jle	.L999
	jmp	.L2398
.L2402:
	cmp	dl, 102
	je	.L2400
	jmp	.L999
.L2401:
	mov	DWORD [edi], push_insn
	mov	DWORD [edi+4], 28
	jmp	.L8695
.L2398:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2408
	cmp	dl, 67
	jg	.L2409
	test	dl, dl
	jg	.L999
	jmp	.L2416
.L2409:
	cmp	dl, 68
	jle	.L2413
	cmp	dl, 86
	jle	.L999
	jmp	.L2415
.L2408:
	cmp	dl, 100
	jg	.L2417
	cmp	dl, 99
	jle	.L999
	jmp	.L2413
.L2417:
	cmp	dl, 119
	je	.L2415
	jmp	.L999
.L2416:
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 24577
	jmp	.L8857
.L2400:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2426
	cmp	dl, 68
	jg	.L2427
	test	dl, dl
	jle	.L2429
	cmp	dl, 67
	jle	.L999
	jmp	.L2431
.L2427:
	cmp	dl, 81
	je	.L2434
	cmp	dl, 86
	jle	.L999
	jmp	.L2436
.L2426:
	cmp	dl, 112
	jg	.L2438
	cmp	dl, 100
	je	.L2431
	jmp	.L999
.L2438:
	cmp	dl, 113
	jle	.L2434
	cmp	dl, 119
	je	.L2436
	jmp	.L999
.L2429:
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 39937
	jmp	.L8695
.L2431:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2137089
	jmp	.L9186
.L2434:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 4234241
	jmp	.L9148
.L2436:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1088513
	jmp	.L8695
.L2413:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2121729
	jmp	.L9186
.L2415:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1073153
	jmp	.L8857
.L2246:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L2468
	cmp	dl, 108
	je	.L2468
	jmp	.L1005
.L2249:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	jg	.L2470
	cmp	dl, 65
	je	.L2472
	cmp	dl, 75
	jle	.L1005
	jmp	.L2474
.L2470:
	cmp	dl, 97
	jg	.L2476
	cmp	dl, 96
	jle	.L1005
	jmp	.L2472
.L2476:
	cmp	dl, 108
	je	.L2474
	jmp	.L1005
.L2252:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L2481
	cmp	dl, 98
	je	.L2481
	jmp	.L1005
.L2240:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L2484
	cmp	dl, 100
	je	.L2484
	jmp	.L1005
.L2243:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	je	.L2487
	cmp	dl, 117
	je	.L2487
	jmp	.L1005
.L2255:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L2490
	cmp	dl, 97
	jne	.L1005
.L2490:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L2493
	cmp	dl, 112
	jne	.L1005
.L2493:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L2496
	cmp	dl, 100
	jne	.L1005
.L2496:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 47873
	jmp	.L8812
.L2487:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 70
	je	.L2502
	cmp	dl, 102
	jne	.L1005
.L2502:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2504
	cmp	dl, 72
	jg	.L2505
	cmp	dl, 68
	je	.L2507
	cmp	dl, 71
	jmp	.L8678
.L2505:
	cmp	dl, 76
	je	.L2512
	cmp	dl, 86
	jle	.L1005
	jmp	.L2514
.L2504:
	cmp	dl, 104
	jg	.L2516
	cmp	dl, 100
	je	.L2507
	cmp	dl, 103
.L8678:
	jle	.L1005
	jmp	.L2509
.L2516:
	cmp	dl, 108
	jg	.L2520
	cmp	dl, 107
	jle	.L1005
	jmp	.L2512
.L2520:
	cmp	dl, 119
	je	.L2514
	jmp	.L1005
.L2507:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssessimm_insn
	mov	DWORD [edi+4], 6713345
	jmp	.L9151
.L2509:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L2528
	cmp	dl, 119
	je	.L2528
	jmp	.L1005
.L2512:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L2531
	cmp	dl, 119
	je	.L2531
	jmp	.L1005
.L2514:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pshufw_insn
	jmp	.L9240
.L2531:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssessimm_insn
	mov	DWORD [edi+4], 15888385
	jmp	.L9151
.L2528:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssessimm_insn
	mov	DWORD [edi+4], 15953921
	jmp	.L9151
.L2484:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L2543
	cmp	dl, 98
	jne	.L1005
.L2543:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L2546
	cmp	dl, 119
	jne	.L1005
.L2546:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 62978
	jmp	.L8825
.L2481:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 66
	cmp	eax, 53
	ja	.L1005
	jmp	DWORD [.L2571+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L2571:
	dd	.L2554
	dd	.L1005
	dd	.L2557
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2560
	dd	.L1005
	dd	.L2563
	dd	.L1005
	dd	.L2566
	dd	.L1005
	dd	.L2569
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2554
	dd	.L1005
	dd	.L2557
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L2560
	dd	.L1005
	dd	.L2563
	dd	.L1005
	dd	.L2566
	dd	.L1005
	dd	.L2569
	[section .text]
.L2554:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 63490
	jmp	.L8866
.L2569:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 63746
	jmp	.L8866
.L2557:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 64002
	jmp	.L8866
.L2560:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 64258
	jmp	.L8866
.L2563:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2584
	cmp	dl, 72
	jg	.L2585
	cmp	dl, 66
	jmp	.L8670
.L2585:
	cmp	dl, 73
	jle	.L2590
	cmp	dl, 86
	jle	.L1005
	jmp	.L2592
.L2584:
	cmp	dl, 104
	jg	.L2594
	cmp	dl, 98
.L8670:
	je	.L2587
	jmp	.L1005
.L2594:
	cmp	dl, 105
	jle	.L2590
	cmp	dl, 119
	je	.L2592
	jmp	.L1005
.L2566:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L2600
	cmp	dl, 115
	jne	.L1005
.L2600:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2602
	cmp	dl, 66
	je	.L2604
	cmp	dl, 86
	jle	.L1005
	jmp	.L2606
.L2602:
	cmp	dl, 98
	jg	.L2608
	cmp	dl, 97
	jle	.L1005
	jmp	.L2604
.L2608:
	cmp	dl, 119
	je	.L2606
	jmp	.L1005
.L2604:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 55298
	jmp	.L8866
.L2606:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 55554
	jmp	.L8866
.L2587:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 59394
	jmp	.L8866
.L2590:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L2622
	cmp	dl, 119
	je	.L2622
	jmp	.L1005
.L2592:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 59650
	jmp	.L8866
.L2622:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixmmx_insn
	mov	DWORD [edi+4], 21761
	jmp	.L8827
.L2472:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2630
	cmp	dl, 68
	je	.L2632
	cmp	dl, 86
	jle	.L1005
	jmp	.L2634
.L2630:
	cmp	dl, 100
	jg	.L2636
	cmp	dl, 99
	jle	.L1005
	jmp	.L2632
.L2636:
	cmp	dl, 119
	je	.L2634
	jmp	.L1005
.L2474:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2640
	cmp	dl, 80
	jg	.L2641
	cmp	dl, 68
	jmp	.L8669
.L2641:
	cmp	dl, 81
	jle	.L2646
	cmp	dl, 86
	jle	.L1005
	jmp	.L2648
.L2640:
	cmp	dl, 112
	jg	.L2649
	cmp	dl, 100
.L8669:
	je	.L2643
	jmp	.L1005
.L2649:
	cmp	dl, 113
	jle	.L2646
	cmp	dl, 119
	jne	.L1005
.L2648:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pshift_insn
	mov	DWORD [edi+4], 41013508
	jmp	.L8866
.L2643:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	jg	.L2657
	test	dl, dl
	jle	.L2659
	cmp	dl, 80
	jle	.L999
	jmp	.L2661
.L2657:
	cmp	dl, 113
	je	.L2661
	jmp	.L999
.L2659:
	mov	DWORD [edi], pshift_insn
	mov	DWORD [edi+4], 41079300
	jmp	.L8866
.L2646:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pshift_insn
	mov	DWORD [edi+4], 41145092
	jmp	.L8866
.L2661:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pslrldq_insn
	mov	DWORD [edi+4], 769
	jmp	.L9151
.L2634:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pshift_insn
	mov	DWORD [edi+4], 74572036
	jmp	.L8866
.L2632:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pshift_insn
	mov	DWORD [edi+4], 74637828
	jmp	.L8866
.L2468:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2678
	cmp	dl, 80
	jg	.L2679
	cmp	dl, 68
	jmp	.L8668
.L2679:
	cmp	dl, 81
	jle	.L2684
	cmp	dl, 86
	jle	.L1005
	jmp	.L2686
.L2678:
	cmp	dl, 112
	jg	.L2687
	cmp	dl, 100
.L8668:
	je	.L2681
	jmp	.L1005
.L2687:
	cmp	dl, 113
	jle	.L2684
	cmp	dl, 119
	jne	.L1005
.L2686:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pshift_insn
	mov	DWORD [edi+4], 108130564
	jmp	.L8866
.L2681:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	jg	.L2695
	test	dl, dl
	jle	.L2697
	cmp	dl, 80
	jle	.L999
	jmp	.L2699
.L2695:
	cmp	dl, 113
	je	.L2699
	jmp	.L999
.L2697:
	mov	DWORD [edi], pshift_insn
	mov	DWORD [edi+4], 108196356
	jmp	.L8866
.L2684:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pshift_insn
	mov	DWORD [edi+4], 108262148
	jmp	.L8866
.L2699:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pslrldq_insn
	mov	DWORD [edi+4], 1793
	jmp	.L9151
.L2235:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 70
	je	.L2711
	cmp	dl, 102
	jne	.L1005
.L2711:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L2714
	cmp	dl, 101
	jne	.L1005
.L2714:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L2717
	cmp	dl, 116
	jne	.L1005
.L2717:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L2720
	cmp	dl, 99
	jne	.L1005
.L2720:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 72
	je	.L2723
	cmp	dl, 104
	jne	.L1005
.L2723:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2725
	cmp	dl, 78
	jg	.L2726
	test	dl, dl
	jle	.L2728
	cmp	dl, 77
	jle	.L999
	jmp	.L2730
.L2726:
	cmp	dl, 84
	je	.L2733
	cmp	dl, 86
	jle	.L999
	jmp	.L2735
.L2725:
	cmp	dl, 115
	jg	.L2737
	cmp	dl, 110
	je	.L2730
	jmp	.L999
.L2737:
	cmp	dl, 116
	jle	.L2733
	cmp	dl, 119
	je	.L2735
	jmp	.L999
.L2728:
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 986369
	jmp	.L8826
.L2730:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L2745
	cmp	dl, 116
	je	.L2745
	jmp	.L1005
.L2733:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 47
	jle	.L1005
	cmp	dl, 48
	jle	.L2749
	cmp	dl, 49
	jle	.L2751
	cmp	dl, 50
	jle	.L2753
	jmp	.L1005
.L2735:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 17763585
	jmp	.L8826
.L2749:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 17766401
	jmp	.L9024
.L2751:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 34543617
	jmp	.L9024
.L2753:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 51320833
	jmp	.L9024
.L2745:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L2767
	cmp	dl, 97
	jne	.L1005
.L2767:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 989185
	jmp	.L9024
.L2226:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 70
	jg	.L2772
	cmp	dl, 64
	jg	.L2773
	test	dl, dl
	jg	.L999
	jmp	.L2780
.L2773:
	cmp	dl, 65
	jle	.L2777
	cmp	dl, 69
	jle	.L999
	jmp	.L2779
.L2772:
	cmp	dl, 97
	jg	.L2781
	cmp	dl, 96
	jle	.L999
	jmp	.L2777
.L2781:
	cmp	dl, 102
	je	.L2779
	jmp	.L999
.L2780:
	mov	DWORD [edi], pop_insn
.L9226:
	mov	DWORD [edi+4], 21
	jmp	.L8695
.L2228:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 60162
	jmp	.L8866
.L2777:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2790
	cmp	dl, 67
	jg	.L2791
	test	dl, dl
	jg	.L999
	jmp	.L2798
.L2791:
	cmp	dl, 68
	jle	.L2795
	cmp	dl, 86
	jle	.L999
	jmp	.L2797
.L2790:
	cmp	dl, 100
	jg	.L2799
	cmp	dl, 99
	jle	.L999
	jmp	.L2795
.L2799:
	cmp	dl, 119
	je	.L2797
	jmp	.L999
.L2798:
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 24833
	jmp	.L8857
.L2779:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2808
	cmp	dl, 68
	jg	.L2809
	test	dl, dl
	jle	.L2811
	cmp	dl, 67
	jle	.L999
	jmp	.L2813
.L2809:
	cmp	dl, 81
	je	.L2816
	cmp	dl, 86
	jle	.L999
	jmp	.L2818
.L2808:
	cmp	dl, 112
	jg	.L2820
	cmp	dl, 100
	je	.L2813
	jmp	.L999
.L2820:
	cmp	dl, 113
	jle	.L2816
	cmp	dl, 119
	je	.L2818
	jmp	.L999
.L2811:
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 40193
	jmp	.L8695
.L2813:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2137345
	jmp	.L9186
.L2818:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1088769
	jmp	.L8695
.L2816:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 4234497
	jmp	.L9148
.L2795:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2121985
	jmp	.L9186
.L2797:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1073409
	jmp	.L8857
.L2200:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	jg	.L2849
	cmp	dl, 68
	jg	.L2850
	cmp	dl, 66
	jle	.L1005
	cmp	dl, 67
	jle	.L2853
	jmp	.L2854
.L2850:
	cmp	dl, 71
	je	.L2857
	cmp	dl, 87
	jle	.L1005
	jmp	.L2859
.L2849:
	cmp	dl, 102
	jg	.L2861
	cmp	dl, 98
	jle	.L1005
	cmp	dl, 99
	jle	.L2853
	cmp	dl, 100
	jle	.L2854
	jmp	.L1005
.L2861:
	cmp	dl, 103
	jle	.L2857
	cmp	dl, 120
	je	.L2859
	jmp	.L1005
.L2210:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L2869
	cmp	dl, 108
	je	.L2869
	jmp	.L1005
.L2202:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L2872
	cmp	dl, 110
	je	.L2872
	jmp	.L1005
.L2206:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L2875
	cmp	dl, 118
	je	.L2875
	jmp	.L1005
.L2211:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	jg	.L2877
	cmp	dl, 76
	jg	.L2878
	cmp	dl, 71
	je	.L2880
	cmp	dl, 75
	jmp	.L8679
.L2878:
	cmp	dl, 78
	je	.L2885
	cmp	dl, 89
	jle	.L1005
	jmp	.L2887
.L2877:
	cmp	dl, 108
	jg	.L2889
	cmp	dl, 103
	je	.L2880
	cmp	dl, 107
.L8679:
	jle	.L1005
	jmp	.L2882
.L2889:
	cmp	dl, 110
	jg	.L2893
	cmp	dl, 109
	jle	.L1005
	jmp	.L2885
.L2893:
	cmp	dl, 122
	je	.L2887
	jmp	.L1005
.L2880:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L2898
	cmp	dl, 101
	je	.L2898
	jmp	.L1005
.L2882:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	je	.L2901
	cmp	dl, 122
	je	.L2901
	jmp	.L1005
.L2885:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	je	.L2904
	cmp	dl, 122
	je	.L2904
	jmp	.L1005
.L2887:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L2907
	cmp	dl, 98
	jne	.L1005
.L2907:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixmmx_insn
	mov	DWORD [edi+4], 22529
	jmp	.L8827
.L2904:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L2913
	cmp	dl, 98
	jne	.L1005
.L2913:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixmmx_insn
	mov	DWORD [edi+4], 23041
	jmp	.L8827
.L2901:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L2919
	cmp	dl, 98
	jne	.L1005
.L2919:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixmmx_insn
	mov	DWORD [edi+4], 23297
	jmp	.L8827
.L2898:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	je	.L2925
	cmp	dl, 122
	jne	.L1005
.L2925:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L2928
	cmp	dl, 98
	jne	.L1005
.L2928:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixmmx_insn
	mov	DWORD [edi+4], 23553
	jmp	.L8827
.L2875:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L2934
	cmp	dl, 109
	jne	.L1005
.L2934:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L2937
	cmp	dl, 115
	jne	.L1005
.L2937:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 75
	je	.L2940
	cmp	dl, 107
	jne	.L1005
.L2940:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L2943
	cmp	dl, 98
	jne	.L1005
.L2943:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pmovmskb_insn
	mov	DWORD [edi+4], 2
	jmp	.L9152
.L2872:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	jg	.L2948
	cmp	dl, 83
	je	.L2950
	cmp	dl, 84
	jle	.L1005
	jmp	.L2952
.L2948:
	cmp	dl, 115
	jg	.L2954
	cmp	dl, 114
	jle	.L1005
	jmp	.L2950
.L2954:
	cmp	dl, 117
	je	.L2952
	jmp	.L1005
.L2950:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L2959
	cmp	dl, 119
	je	.L2959
	jmp	.L1005
.L2952:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L2962
	cmp	dl, 98
	jne	.L1005
.L2962:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 55810
	jmp	.L8825
.L2959:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 59906
	jmp	.L8825
.L2869:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	jg	.L2970
	cmp	dl, 75
	jg	.L2971
	cmp	dl, 72
	jmp	.L8680
.L2971:
	cmp	dl, 76
	jle	.L2975
	cmp	dl, 84
	jle	.L1005
	jmp	.L2977
.L2970:
	cmp	dl, 107
	jg	.L2979
	cmp	dl, 104
.L8680:
	jne	.L1005
	jmp	.L2978
.L2979:
	cmp	dl, 108
	jle	.L2975
	cmp	dl, 117
	je	.L2977
	jmp	.L1005
.L2978:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L2984
	cmp	dl, 84
	jg	.L2985
	cmp	dl, 82
	jmp	.L8667
.L2985:
	cmp	dl, 85
	jle	.L2990
	cmp	dl, 86
	jle	.L1005
	jmp	.L2992
.L2984:
	cmp	dl, 116
	jg	.L2994
	cmp	dl, 114
.L8667:
	je	.L2987
	jmp	.L1005
.L2994:
	cmp	dl, 117
	jle	.L2990
	cmp	dl, 119
	je	.L2992
	jmp	.L1005
.L2975:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3000
	cmp	dl, 119
	je	.L3000
	jmp	.L1005
.L2977:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L3003
	cmp	dl, 100
	jne	.L1005
.L3003:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L3006
	cmp	dl, 113
	jne	.L1005
.L3006:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 62466
	jmp	.L9151
.L3000:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 54530
	jmp	.L8866
.L2987:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L3014
	cmp	dl, 73
	je	.L3016
	cmp	dl, 86
	jle	.L1005
	jmp	.L3018
.L3014:
	cmp	dl, 105
	jg	.L3020
	cmp	dl, 104
	jle	.L1005
	jmp	.L3016
.L3020:
	cmp	dl, 119
	je	.L3018
	jmp	.L1005
.L2990:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3025
	cmp	dl, 119
	je	.L3025
	jmp	.L1005
.L2992:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 58626
	jmp	.L8866
.L3025:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 58370
	jmp	.L8825
.L3018:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	jg	.L3033
	cmp	dl, 65
	je	.L3035
	cmp	dl, 66
	jle	.L1005
	jmp	.L3037
.L3033:
	cmp	dl, 97
	jg	.L3039
	cmp	dl, 96
	jle	.L1005
	jmp	.L3035
.L3039:
	cmp	dl, 99
	je	.L3037
	jmp	.L1005
.L3016:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3044
	cmp	dl, 119
	jne	.L1005
.L3044:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixmmx_insn
	mov	DWORD [edi+4], 23809
	jmp	.L8827
.L3035:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 46849
	jmp	.L8826
.L3037:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixmmx_insn
	mov	DWORD [edi+4], 22785
	jmp	.L8827
.L2853:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 72
	je	.L3056
	cmp	dl, 104
	je	.L3056
	jmp	.L1005
.L2854:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L3059
	cmp	dl, 100
	je	.L3059
	jmp	.L1005
.L2857:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3062
	cmp	dl, 119
	je	.L3062
	jmp	.L1005
.L2859:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	jg	.L3064
	cmp	dl, 83
	je	.L3066
	cmp	dl, 84
	jle	.L1005
	jmp	.L3068
.L3064:
	cmp	dl, 115
	jg	.L3070
	cmp	dl, 114
	jle	.L1005
	jmp	.L3066
.L3070:
	cmp	dl, 117
	je	.L3068
	jmp	.L1005
.L3066:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3075
	cmp	dl, 119
	je	.L3075
	jmp	.L1005
.L3068:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L3078
	cmp	dl, 98
	jne	.L1005
.L3078:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 56834
	jmp	.L8825
.L3075:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 60930
	jmp	.L8825
.L3062:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixmmx_insn
	mov	DWORD [edi+4], 20993
	jmp	.L8827
.L3059:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3090
	cmp	dl, 119
	jne	.L1005
.L3090:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L3093
	cmp	dl, 100
	jne	.L1005
.L3093:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 62722
	jmp	.L8866
.L3056:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L3099
	cmp	dl, 114
	jne	.L1005
.L3099:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L3102
	cmp	dl, 105
	jne	.L1005
.L3102:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3105
	cmp	dl, 119
	jne	.L1005
.L3105:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pmachriw_insn
	mov	DWORD [edi+4], 1
	jmp	.L8827
.L2195:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L3111
	cmp	dl, 115
	je	.L3111
	jmp	.L1005
.L2192:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 70
	je	.L3114
	cmp	dl, 102
	jne	.L1005
.L3114:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L3116
	cmp	dl, 68
	je	.L3118
	cmp	dl, 86
	jle	.L1005
	jmp	.L3120
.L3116:
	cmp	dl, 100
	jg	.L3122
	cmp	dl, 99
	jle	.L1005
	jmp	.L3118
.L3122:
	cmp	dl, 119
	je	.L3120
	jmp	.L1005
.L3118:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 3329
	jmp	.L8826
.L3120:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 3073
	jmp	.L8812
.L3111:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L3133
	cmp	dl, 114
	jne	.L1005
.L3133:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3136
	cmp	dl, 119
	jne	.L1005
.L3136:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pinsrw_insn
	mov	DWORD [edi+4], 4
	jmp	.L8825
.L2166:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L3142
	cmp	dl, 105
	je	.L3142
	jmp	.L1005
.L2169:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	jg	.L3144
	cmp	dl, 66
	jle	.L1005
	cmp	dl, 67
	jle	.L3147
	jmp	.L3148
.L3144:
	cmp	dl, 98
	jle	.L1005
	cmp	dl, 99
	jle	.L3147
	cmp	dl, 100
	jle	.L3148
	jmp	.L1005
.L2172:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L3154
	cmp	dl, 109
	je	.L3154
	jmp	.L1005
.L2175:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	jg	.L3156
	cmp	dl, 72
	jg	.L3157
	cmp	dl, 65
	jmp	.L8666
.L3157:
	cmp	dl, 73
	jle	.L3162
	cmp	dl, 84
	jle	.L1005
	jmp	.L3164
.L3156:
	cmp	dl, 104
	jg	.L3166
	cmp	dl, 97
.L8666:
	je	.L3159
	jmp	.L1005
.L3166:
	cmp	dl, 105
	jle	.L3162
	cmp	dl, 117
	je	.L3164
	jmp	.L1005
.L2178:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L3172
	cmp	dl, 97
	je	.L3172
	jmp	.L1005
.L2181:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L3175
	cmp	dl, 110
	je	.L3175
	jmp	.L1005
.L2184:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L3177
	cmp	dl, 67
	je	.L3179
	cmp	dl, 82
	jle	.L1005
	jmp	.L3181
.L3177:
	cmp	dl, 99
	jg	.L3183
	cmp	dl, 98
	jle	.L1005
	jmp	.L3179
.L3183:
	cmp	dl, 115
	je	.L3181
	jmp	.L1005
.L2187:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	je	.L3188
	cmp	dl, 117
	jne	.L1005
.L3188:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L3191
	cmp	dl, 98
	jne	.L1005
.L3191:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L3193
	test	dl, dl
	jle	.L3195
	cmp	dl, 81
	jle	.L999
	jmp	.L3197
.L3193:
	cmp	dl, 114
	je	.L3197
	jmp	.L999
.L3195:
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 39425
	jmp	.L8826
.L3197:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 43521
	jmp	.L8826
.L3179:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L3206
	cmp	dl, 112
	je	.L3206
	jmp	.L1005
.L3181:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L3209
	cmp	dl, 113
	jne	.L1005
.L3209:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L3211
	cmp	dl, 73
	je	.L3213
	cmp	dl, 81
	jle	.L1005
	jmp	.L3215
.L3211:
	cmp	dl, 105
	jg	.L3217
	cmp	dl, 104
	jle	.L1005
	jmp	.L3213
.L3217:
	cmp	dl, 114
	je	.L3215
	jmp	.L1005
.L3213:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L3222
	cmp	dl, 116
	je	.L3222
	jmp	.L1005
.L3215:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L3225
	cmp	dl, 116
	jne	.L1005
.L3225:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 38657
	jmp	.L8826
.L3222:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 49
	jne	.L1005
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 42753
	jmp	.L8826
.L3206:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	jg	.L3234
	test	dl, dl
	jle	.L3236
	cmp	dl, 72
	jle	.L999
	jmp	.L3238
.L3234:
	cmp	dl, 105
	je	.L3238
	jmp	.L999
.L3236:
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 38401
	jmp	.L8826
.L3238:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L3244
	cmp	dl, 116
	jne	.L1005
.L3244:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 48
	jle	.L1005
	cmp	dl, 49
	jle	.L3248
	cmp	dl, 50
	jle	.L3250
	jmp	.L1005
.L3248:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 42497
	jmp	.L8826
.L3250:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 46593
	jmp	.L8826
.L3175:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L3258
	cmp	dl, 97
	jne	.L1005
.L3258:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L3261
	cmp	dl, 99
	jne	.L1005
.L3261:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L3264
	cmp	dl, 99
	jne	.L1005
.L3264:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 36353
	jmp	.L8812
.L3172:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L3270
	cmp	dl, 99
	jne	.L1005
.L3270:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L3273
	cmp	dl, 99
	jne	.L1005
.L3273:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 35329
	jmp	.L8812
.L3159:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L3279
	cmp	dl, 120
	je	.L3279
	jmp	.L1005
.L3162:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L3282
	cmp	dl, 110
	je	.L3282
	jmp	.L1005
.L3164:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L3285
	cmp	dl, 108
	jne	.L1005
.L3285:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 46081
	jmp	.L8826
.L3282:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 37889
	jmp	.L8826
.L3279:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 41985
	jmp	.L8826
.L3154:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L3297
	cmp	dl, 112
	jne	.L1005
.L3297:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 71
	jg	.L3299
	cmp	dl, 69
	je	.L3301
	cmp	dl, 70
	jle	.L1005
	jmp	.L3303
.L3299:
	cmp	dl, 101
	jg	.L3305
	cmp	dl, 100
	jle	.L1005
	jmp	.L3301
.L3305:
	cmp	dl, 103
	je	.L3303
	jmp	.L1005
.L3301:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L3310
	cmp	dl, 113
	je	.L3310
	jmp	.L1005
.L3303:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L3312
	cmp	dl, 69
	je	.L3314
	cmp	dl, 83
	jle	.L1005
	jmp	.L3316
.L3312:
	cmp	dl, 101
	jg	.L3318
	cmp	dl, 100
	jle	.L1005
	jmp	.L3314
.L3318:
	cmp	dl, 116
	je	.L3316
	jmp	.L1005
.L3314:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 36865
	jmp	.L8826
.L3316:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 40961
	jmp	.L8826
.L3310:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 45057
	jmp	.L8826
.L3147:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L3332
	cmp	dl, 99
	je	.L3332
	jmp	.L1005
.L3148:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L3335
	cmp	dl, 100
	jne	.L1005
.L3335:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 40449
	jmp	.L8826
.L3332:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 44545
	jmp	.L8826
.L3142:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L3343
	cmp	dl, 68
	je	.L3345
	cmp	dl, 86
	jle	.L1005
	jmp	.L3347
.L3343:
	cmp	dl, 100
	jg	.L3349
	cmp	dl, 99
	jle	.L1005
	jmp	.L3345
.L3349:
	cmp	dl, 119
	je	.L3347
	jmp	.L1005
.L3345:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 7425
	jmp	.L8826
.L3347:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 7169
.L8812:
	mov	DWORD [edi+8], 66560
	jmp	.L8696
.L2162:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L3360
	cmp	dl, 116
	jne	.L1005
.L3360:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L3363
	cmp	dl, 114
	jne	.L1005
.L3363:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3366
	cmp	dl, 119
	jne	.L1005
.L3366:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], pextrw_insn
	mov	DWORD [edi+4], 2
	jmp	.L8825
.L2159:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L3372
	cmp	dl, 115
	jne	.L1005
.L3372:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L3375
	cmp	dl, 116
	jne	.L1005
.L3375:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L3378
	cmp	dl, 105
	jne	.L1005
.L3378:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L3381
	cmp	dl, 98
	jne	.L1005
.L3381:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixmmx_insn
	mov	DWORD [edi+4], 21505
	jmp	.L8827
.L2145:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L3387
	cmp	dl, 115
	je	.L3387
	jmp	.L1005
.L2137:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	jg	.L3389
	cmp	dl, 75
	je	.L3391
	cmp	dl, 76
	jle	.L1005
	jmp	.L3393
.L3389:
	cmp	dl, 107
	jg	.L3395
	cmp	dl, 106
	jle	.L1005
	jmp	.L3391
.L3395:
	cmp	dl, 109
	je	.L3393
	jmp	.L1005
.L2139:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L3400
	cmp	dl, 100
	je	.L3400
	jmp	.L1005
.L2142:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L3403
	cmp	dl, 100
	je	.L3403
	jmp	.L1005
.L2146:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 71
	jg	.L3405
	cmp	dl, 69
	je	.L3407
	cmp	dl, 70
	jle	.L1005
	jmp	.L3409
.L3405:
	cmp	dl, 101
	jg	.L3411
	cmp	dl, 100
	jle	.L1005
	jmp	.L3407
.L3411:
	cmp	dl, 103
	je	.L3409
	jmp	.L1005
.L3407:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L3416
	cmp	dl, 98
	je	.L3416
	jmp	.L1005
.L3409:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L3418
	cmp	dl, 84
	jg	.L3419
	cmp	dl, 66
	jmp	.L8681
.L3419:
	cmp	dl, 85
	jle	.L3423
	cmp	dl, 86
	jle	.L1005
	jmp	.L3425
.L3418:
	cmp	dl, 116
	jg	.L3427
	cmp	dl, 98
.L8681:
	jne	.L1005
	jmp	.L3426
.L3427:
	cmp	dl, 117
	jle	.L3423
	cmp	dl, 119
	je	.L3425
	jmp	.L1005
.L3426:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 57346
	jmp	.L8825
.L3425:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 58114
	jmp	.L8825
.L3423:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L3439
	cmp	dl, 115
	jne	.L1005
.L3439:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L3442
	cmp	dl, 98
	jne	.L1005
.L3442:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], now3d_insn
	mov	DWORD [edi+4], 48897
	jmp	.L8826
.L3416:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixmmx_insn
	mov	DWORD [edi+4], 20481
	jmp	.L8827
.L3403:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	jg	.L3450
	test	dl, dl
	jle	.L3452
	cmp	dl, 77
	jle	.L999
	jmp	.L3454
.L3450:
	cmp	dl, 110
	je	.L3454
	jmp	.L999
.L3452:
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 56066
	jmp	.L8866
.L3454:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 57090
	jmp	.L8866
.L3400:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 66
	cmp	eax, 53
	ja	.L1005
	jmp	DWORD [.L3482+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L3482:
	dd	.L3465
	dd	.L1005
	dd	.L3468
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L3471
	dd	.L1005
	dd	.L3474
	dd	.L1005
	dd	.L3477
	dd	.L1005
	dd	.L3480
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L3465
	dd	.L1005
	dd	.L3468
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L3471
	dd	.L1005
	dd	.L3474
	dd	.L1005
	dd	.L3477
	dd	.L1005
	dd	.L3480
	[section .text]
.L3465:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 64514
	jmp	.L8866
.L3480:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 64770
	jmp	.L8866
.L3468:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 65026
	jmp	.L8866
.L3471:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 54274
	jmp	.L8866
.L3474:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L3495
	cmp	dl, 72
	jg	.L3496
	cmp	dl, 66
	jmp	.L8665
.L3496:
	cmp	dl, 73
	jle	.L3501
	cmp	dl, 86
	jle	.L1005
	jmp	.L3503
.L3495:
	cmp	dl, 104
	jg	.L3505
	cmp	dl, 98
.L8665:
	je	.L3498
	jmp	.L1005
.L3505:
	cmp	dl, 105
	jle	.L3501
	cmp	dl, 119
	je	.L3503
	jmp	.L1005
.L3477:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L3511
	cmp	dl, 115
	jne	.L1005
.L3511:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L3513
	cmp	dl, 66
	je	.L3515
	cmp	dl, 86
	jle	.L1005
	jmp	.L3517
.L3513:
	cmp	dl, 98
	jg	.L3519
	cmp	dl, 97
	jle	.L1005
	jmp	.L3515
.L3519:
	cmp	dl, 119
	je	.L3517
	jmp	.L1005
.L3515:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 56322
	jmp	.L8866
.L3517:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 56578
	jmp	.L8866
.L3498:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 60418
	jmp	.L8866
.L3501:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3533
	cmp	dl, 119
	je	.L3533
	jmp	.L1005
.L3503:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 60674
	jmp	.L8866
.L3533:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixmmx_insn
	mov	DWORD [edi+4], 20737
.L8827:
	mov	DWORD [edi+8], 139264
	jmp	.L8696
.L3391:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	jg	.L3541
	cmp	dl, 83
	je	.L3543
	cmp	dl, 84
	jle	.L1005
	jmp	.L3545
.L3541:
	cmp	dl, 115
	jg	.L3547
	cmp	dl, 114
	jle	.L1005
	jmp	.L3543
.L3547:
	cmp	dl, 117
	je	.L3545
	jmp	.L1005
.L3393:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L3552
	cmp	dl, 112
	jne	.L1005
.L3552:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 71
	jg	.L3554
	cmp	dl, 69
	je	.L3556
	cmp	dl, 70
	jle	.L1005
	jmp	.L3558
.L3554:
	cmp	dl, 101
	jg	.L3560
	cmp	dl, 100
	jle	.L1005
	jmp	.L3556
.L3560:
	cmp	dl, 103
	je	.L3558
	jmp	.L1005
.L3556:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L3565
	cmp	dl, 113
	je	.L3565
	jmp	.L1005
.L3558:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L3568
	cmp	dl, 116
	jne	.L1005
.L3568:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L3570
	cmp	dl, 67
	jg	.L3571
	cmp	dl, 66
	jmp	.L8682
.L3571:
	cmp	dl, 68
	jle	.L3575
	cmp	dl, 86
	jle	.L1005
	jmp	.L3577
.L3570:
	cmp	dl, 99
	jg	.L3579
	cmp	dl, 98
.L8682:
	jne	.L1005
	jmp	.L3578
.L3579:
	cmp	dl, 100
	jle	.L3575
	cmp	dl, 119
	je	.L3577
	jmp	.L1005
.L3578:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 25602
	jmp	.L8866
.L3575:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 26114
	jmp	.L8866
.L3577:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 25858
	jmp	.L8866
.L3565:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L3593
	cmp	dl, 67
	jg	.L3594
	cmp	dl, 66
	jmp	.L8683
.L3594:
	cmp	dl, 68
	jle	.L3598
	cmp	dl, 86
	jle	.L1005
	jmp	.L3600
.L3593:
	cmp	dl, 99
	jg	.L3602
	cmp	dl, 98
.L8683:
	jne	.L1005
	jmp	.L3601
.L3602:
	cmp	dl, 100
	jle	.L3598
	cmp	dl, 119
	je	.L3600
	jmp	.L1005
.L3601:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 29698
	jmp	.L8866
.L3598:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 30210
	jmp	.L8866
.L3600:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 29954
	jmp	.L8866
.L3543:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L3617
	cmp	dl, 115
	je	.L3617
	jmp	.L1005
.L3545:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L3620
	cmp	dl, 115
	jne	.L1005
.L3620:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3623
	cmp	dl, 119
	jne	.L1005
.L3623:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L3626
	cmp	dl, 98
	jne	.L1005
.L3626:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 26370
	jmp	.L8866
.L3617:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L3631
	cmp	dl, 68
	je	.L3633
	cmp	dl, 86
	jle	.L1005
	jmp	.L3635
.L3631:
	cmp	dl, 100
	jg	.L3637
	cmp	dl, 99
	jle	.L1005
	jmp	.L3633
.L3637:
	cmp	dl, 119
	je	.L3635
	jmp	.L1005
.L3633:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L3642
	cmp	dl, 119
	je	.L3642
	jmp	.L1005
.L3635:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L3645
	cmp	dl, 98
	jne	.L1005
.L3645:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 25346
	jmp	.L8866
.L3642:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], mmxsse2_insn
	mov	DWORD [edi+4], 27394
	jmp	.L8866
.L3387:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L3654
	cmp	dl, 101
	jne	.L1005
.L3654:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 15962113
	jmp	.L8847
.L1420:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 25861
	jmp	.L8700
.L1398:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 23
	jmp	.L8699
.L1401:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 19
	jmp	.L8699
.L1404:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	je	.L3663
	cmp	dl, 117
	je	.L3663
	jmp	.L1005
.L1407:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	jg	.L3665
	test	dl, dl
	jle	.L3667
	cmp	dl, 75
	jle	.L999
	jmp	.L3669
.L3665:
	cmp	dl, 108
	je	.L3669
	jmp	.L999
.L3667:
	mov	DWORD [edi], 53
	jmp	.L8699
.L1410:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L3672
	cmp	dl, 81
	jg	.L3673
	cmp	dl, 70
	jmp	.L8664
.L3673:
	cmp	dl, 82
	jle	.L3678
	cmp	dl, 86
	jle	.L1005
	jmp	.L3680
.L3672:
	cmp	dl, 113
	jg	.L3682
	cmp	dl, 102
.L8664:
	je	.L3675
	jmp	.L1005
.L3682:
	cmp	dl, 114
	jle	.L3678
	cmp	dl, 119
	je	.L3680
	jmp	.L1005
.L1413:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L3687
	cmp	dl, 67
	jg	.L3688
	test	dl, dl
	jle	.L3690
	cmp	dl, 66
	jle	.L999
	jmp	.L3692
.L3688:
	cmp	dl, 81
	jle	.L999
	cmp	dl, 82
	jle	.L3696
	jmp	.L3697
.L3687:
	cmp	dl, 113
	jg	.L3699
	cmp	dl, 99
	je	.L3692
	jmp	.L999
.L3699:
	cmp	dl, 114
	jle	.L3696
	cmp	dl, 115
	jle	.L3697
	jmp	.L999
.L3690:
	mov	DWORD [edi], bittest_insn
	mov	DWORD [edi+4], 303878
	jmp	.L9186
.L1416:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 51
	jmp	.L8699
.L3692:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], bittest_insn
	mov	DWORD [edi+4], 506630
	jmp	.L9186
.L3696:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], bittest_insn
	mov	DWORD [edi+4], 439046
	jmp	.L9186
.L3697:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], bittest_insn
	mov	DWORD [edi+4], 371462
	jmp	.L9186
.L3675:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], bsfr_insn
	mov	DWORD [edi+4], 48131
	jmp	.L9186
.L3678:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], bsfr_insn
	mov	DWORD [edi+4], 48387
	jmp	.L9186
.L3680:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L3723
	cmp	dl, 97
	jne	.L1005
.L3723:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L3726
	cmp	dl, 112
	jne	.L1005
.L3726:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], bswap_insn
	mov	DWORD [edi+4], 2
	jmp	.L8930
.L3669:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 37
	jmp	.L8699
.L3663:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L3734
	cmp	dl, 110
	jne	.L1005
.L3734:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L3737
	cmp	dl, 100
	jne	.L1005
.L3737:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], bound_insn
	mov	DWORD [edi+4], 2
	jmp	.L8857
.L1374:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L3746
	cmp	dl, 120
	je	.L3746
	jmp	.L1005
.L1377:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	jg	.L3748
	cmp	dl, 80
	je	.L3750
	cmp	dl, 87
	jle	.L1005
	jmp	.L3752
.L3748:
	cmp	dl, 112
	jg	.L3754
	cmp	dl, 111
	jle	.L1005
	jmp	.L3750
.L3754:
	cmp	dl, 120
	je	.L3752
	jmp	.L1005
.L1380:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L3759
	cmp	dl, 120
	je	.L3759
	jmp	.L1005
.L1383:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	jg	.L3761
	cmp	dl, 73
	je	.L3763
	cmp	dl, 87
	jle	.L1005
	jmp	.L3765
.L3761:
	cmp	dl, 105
	jg	.L3767
	cmp	dl, 104
	jle	.L1005
	jmp	.L3763
.L3767:
	cmp	dl, 120
	je	.L3765
	jmp	.L1005
.L1386:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L3772
	cmp	dl, 109
	je	.L3772
	jmp	.L1005
.L1389:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L3775
	cmp	dl, 116
	je	.L3775
	jmp	.L1005
.L1392:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L3777
	cmp	dl, 72
	jg	.L3778
	test	dl, dl
	jg	.L999
	jmp	.L3785
.L3778:
	cmp	dl, 73
	jle	.L3782
	cmp	dl, 79
	jle	.L999
	jmp	.L3784
.L3777:
	cmp	dl, 105
	jg	.L3786
	cmp	dl, 104
	jle	.L999
	jmp	.L3782
.L3786:
	cmp	dl, 112
	je	.L3784
	jmp	.L999
.L3785:
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L3790
	push	ebx
	push	DWORD LC16
	push	esi
	push	DWORD 0
	call	yasm__warning
	add	esp, 16
.L3790:
	mov	DWORD [edi], 9728
	jmp	.L8700
.L3784:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 68
	jmp	.L8699
.L3782:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 70
	jmp	.L8699
.L3775:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L3794
	cmp	dl, 101
	jne	.L1005
.L3794:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L3797
	cmp	dl, 114
	jne	.L1005
.L3797:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], enter_insn
	mov	DWORD [edi+4], 1
	jmp	.L8857
.L3772:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L3803
	cmp	dl, 115
	jne	.L1005
.L3803:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 1013505
	jmp	.L8866
.L3765:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 66
	jmp	.L8699
.L3763:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 71
	jmp	.L8699
.L3759:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 65
	jmp	.L8699
.L3752:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 67
	jmp	.L8699
.L3750:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 69
	jmp	.L8699
.L3746:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 64
	jmp	.L8699
.L1353:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L3815
	cmp	dl, 100
	je	.L3815
	jmp	.L1005
.L1356:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L3818
	cmp	dl, 116
	je	.L3818
	jmp	.L1005
.L1359:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 72
	je	.L3821
	cmp	dl, 104
	je	.L3821
	jmp	.L1005
.L1362:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L3824
	cmp	dl, 97
	je	.L3824
	jmp	.L1005
.L1365:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L3827
	cmp	dl, 109
	je	.L3827
	jmp	.L1005
.L1368:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L3830
	cmp	dl, 114
	jne	.L1005
.L3830:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L3832
	test	dl, dl
	jle	.L3834
	cmp	dl, 79
	jle	.L999
	jmp	.L3836
.L3832:
	cmp	dl, 112
	je	.L3836
	jmp	.L999
.L3834:
	mov	DWORD [edi], arith_insn
	mov	DWORD [edi+4], 405527
	jmp	.L8695
.L3836:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L3841
	cmp	dl, 68
	je	.L3843
	cmp	dl, 82
	jle	.L1005
	jmp	.L3845
.L3841:
	cmp	dl, 100
	jg	.L3846
	cmp	dl, 99
	jle	.L1005
	jmp	.L3843
.L3846:
	cmp	dl, 115
	jne	.L1005
.L3845:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 22273
	jmp	.L9152
.L3843:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6706945
	jmp	.L9151
.L3827:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 47
	jle	.L1005
	cmp	dl, 49
	je	.L3858
	cmp	dl, 57
	jg	.L1005
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L3861:
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L3862
	mov	al, BYTE [ebx+3]
	sub	eax, 56
	cmp	al, 1
	jbe	.L9215
.L3862:
	movsx	eax, BYTE [ebx+3]
	sub	eax, 48
	or	eax, 128
	jmp	.L9201
.L3858:
	inc	ecx
	mov	dl, BYTE [ecx]
	test	dl, dl
	jle	.L3861
	cmp	dl, 47
	jle	.L999
	cmp	dl, 53
	jg	.L999
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	movsx	eax, BYTE [ebx+4]
	jmp	.L9269
.L3824:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L3869
	cmp	dl, 116
	jne	.L1005
.L3869:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	jg	.L3871
	test	dl, dl
	jle	.L3873
	cmp	dl, 65
	jle	.L999
	jmp	.L3875
.L3871:
	cmp	dl, 98
	je	.L3875
	jmp	.L999
.L3873:
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 55041
	jmp	.L8695
.L3875:
	inc	ecx
	cmp	BYTE [ecx], 0
	jle	.L3873
	jmp	.L999
.L3821:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 71
	je	.L3882
	cmp	dl, 103
	jne	.L1005
.L3882:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], xchg_insn
	mov	DWORD [edi+4], 14
	jmp	.L8695
.L3818:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L3888
	cmp	dl, 115
	jne	.L1005
.L3888:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], xbts_insn
.L8724:
	mov	DWORD [edi+4], 2
	mov	DWORD [edi+8], 6291460
	jmp	.L8696
.L3815:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L3894
	cmp	dl, 100
	jne	.L1005
.L3894:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cmpxchgxadd_insn
	mov	DWORD [edi+4], 49156
	jmp	.L8930
.L1332:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	jg	.L3899
	cmp	dl, 83
	je	.L3901
	cmp	dl, 87
	jle	.L1005
	jmp	.L3903
.L3899:
	cmp	dl, 115
	jg	.L3905
	cmp	dl, 114
	jle	.L1005
	jmp	.L3901
.L3905:
	cmp	dl, 120
	je	.L3903
	jmp	.L1005
.L1335:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L3910
	cmp	dl, 101
	je	.L3910
	jmp	.L1005
.L1338:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L3913
	cmp	dl, 110
	je	.L3913
	jmp	.L1005
.L1341:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 47
	jle	.L1005
	cmp	dl, 55
	jle	.L3917
	jmp	.L1005
.L1344:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L3919
	cmp	dl, 118
	je	.L3919
	jmp	.L1005
.L1347:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L3922
	cmp	dl, 108
	jne	.L1005
.L3922:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L3924
	cmp	dl, 79
	jg	.L3925
	test	dl, dl
	jg	.L999
	jmp	.L3932
.L3925:
	cmp	dl, 80
	jle	.L3929
	cmp	dl, 82
	jle	.L999
	jmp	.L3931
.L3924:
	cmp	dl, 112
	jg	.L3933
	cmp	dl, 111
	jle	.L999
	jmp	.L3929
.L3933:
	cmp	dl, 115
	je	.L3931
	jmp	.L999
.L3932:
	mov	DWORD [edi], f6_insn
	mov	DWORD [edi+4], 1028
	jmp	.L8695
.L3929:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L3939
	cmp	dl, 68
	je	.L3941
	cmp	dl, 82
	jle	.L1005
	jmp	.L3943
.L3939:
	cmp	dl, 100
	jg	.L3945
	cmp	dl, 99
	jle	.L1005
	jmp	.L3941
.L3945:
	cmp	dl, 115
	je	.L3943
	jmp	.L1005
.L3931:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L3949
	cmp	dl, 68
	je	.L3951
	cmp	dl, 82
	jle	.L1005
	jmp	.L3953
.L3949:
	cmp	dl, 100
	jg	.L3954
	cmp	dl, 99
	jle	.L1005
	jmp	.L3951
.L3954:
	cmp	dl, 115
	jne	.L1005
.L3953:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15948033
	jmp	.L9152
.L3951:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15882497
	jmp	.L9151
.L3943:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 22785
	jmp	.L9152
.L3941:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6707457
	jmp	.L9151
.L3919:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	jg	.L3970
	cmp	dl, 76
	jg	.L3971
	cmp	dl, 67
	jg	.L3972
	test	dl, dl
	jle	.L3974
	cmp	dl, 65
	je	.L3976
	jmp	.L999
.L3972:
	cmp	dl, 71
	jg	.L3978
	cmp	dl, 68
	jle	.L3980
	jmp	.L999
.L3978:
	cmp	dl, 72
	jle	.L3983
	cmp	dl, 75
	jle	.L999
	jmp	.L3985
.L3971:
	cmp	dl, 82
	jg	.L3987
	cmp	dl, 78
	jg	.L3988
	cmp	dl, 77
	jle	.L3990
	jmp	.L3991
.L3988:
	cmp	dl, 81
	je	.L3994
	jmp	.L999
.L3987:
	cmp	dl, 84
	jg	.L3996
	cmp	dl, 83
	jle	.L3998
	jmp	.L999
.L3996:
	cmp	dl, 85
	jle	.L4001
	cmp	dl, 89
	jle	.L999
	jmp	.L4003
.L3970:
	cmp	dl, 109
	jg	.L4005
	cmp	dl, 100
	jg	.L4006
	cmp	dl, 97
	je	.L3976
	cmp	dl, 99
	jle	.L999
	jmp	.L3980
.L4006:
	cmp	dl, 104
	jg	.L4010
	cmp	dl, 103
	jle	.L999
	jmp	.L3983
.L4010:
	cmp	dl, 107
	jle	.L999
	cmp	dl, 108
	jle	.L3985
	jmp	.L3990
.L4005:
	cmp	dl, 115
	jg	.L4016
	cmp	dl, 112
	jg	.L4017
	cmp	dl, 110
	jle	.L3991
	jmp	.L999
.L4017:
	cmp	dl, 113
	jle	.L3994
	cmp	dl, 114
	jle	.L999
	jmp	.L3998
.L4016:
	cmp	dl, 117
	jg	.L4023
	cmp	dl, 116
	jle	.L999
	jmp	.L4001
.L4023:
	cmp	dl, 122
	je	.L4003
	jmp	.L999
.L3974:
	mov	DWORD [edi], mov_insn
	mov	DWORD [edi+4], 45
	jmp	.L8695
.L3976:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L4030
	cmp	dl, 112
	je	.L4030
	jmp	.L1005
.L3980:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	jg	.L4032
	test	dl, dl
	jle	.L4034
	cmp	dl, 80
	jle	.L999
	jmp	.L4036
.L4032:
	cmp	dl, 113
	je	.L4036
	jmp	.L999
.L4034:
	mov	DWORD [edi], movd_insn
	mov	DWORD [edi+4], 8
	jmp	.L8866
.L3983:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L4041
	cmp	dl, 76
	je	.L4043
	cmp	dl, 79
	jle	.L1005
	jmp	.L4045
.L4041:
	cmp	dl, 108
	jg	.L4047
	cmp	dl, 107
	jle	.L1005
	jmp	.L4043
.L4047:
	cmp	dl, 112
	je	.L4045
	jmp	.L1005
.L3985:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L4051
	cmp	dl, 72
	je	.L4053
	cmp	dl, 79
	jle	.L1005
	jmp	.L4055
.L4051:
	cmp	dl, 104
	jg	.L4057
	cmp	dl, 103
	jle	.L1005
	jmp	.L4053
.L4057:
	cmp	dl, 112
	je	.L4055
	jmp	.L1005
.L3990:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L4062
	cmp	dl, 115
	je	.L4062
	jmp	.L1005
.L3991:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L4065
	cmp	dl, 116
	je	.L4065
	jmp	.L1005
.L3994:
	inc	ecx
	mov	dl, BYTE [ecx]
	test	dl, dl
	jle	.L4068
	cmp	dl, 50
	je	.L4070
	jmp	.L999
.L4068:
	mov	DWORD [edi], movq_insn
	mov	DWORD [edi+4], 5
.L8866:
	mov	DWORD [edi+8], 8192
	jmp	.L8696
.L3998:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 66
	cmp	eax, 54
	ja	.L1005
	jmp	DWORD [.L4093+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L4093:
	dd	.L4076
	dd	.L1005
	dd	.L4079
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L4082
	dd	.L1005
	dd	.L4085
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L4088
	dd	.L4091
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L4076
	dd	.L1005
	dd	.L4079
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L4082
	dd	.L1005
	dd	.L4085
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L4088
	dd	.L4091
	[section .text]
.L4001:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L4095
	cmp	dl, 112
	je	.L4095
	jmp	.L1005
.L4003:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L4098
	cmp	dl, 120
	jne	.L1005
.L4098:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movszx_insn
	mov	DWORD [edi+4], 46597
	jmp	.L9186
.L4095:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4103
	cmp	dl, 68
	je	.L4105
	cmp	dl, 82
	jle	.L1005
	jmp	.L4107
.L4103:
	cmp	dl, 100
	jg	.L4109
	cmp	dl, 99
	jle	.L1005
	jmp	.L4105
.L4109:
	cmp	dl, 115
	je	.L4107
	jmp	.L1005
.L4105:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movaupd_insn
	mov	DWORD [edi+4], 4098
	jmp	.L9151
.L4107:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movaups_insn
	mov	DWORD [edi+4], 4098
	jmp	.L9152
.L4091:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	jg	.L4119
	test	dl, dl
	jle	.L4121
	cmp	dl, 67
	jle	.L999
	jmp	.L4123
.L4119:
	cmp	dl, 100
	je	.L4123
	jmp	.L999
.L4121:
	mov	DWORD [edi], movszx_insn
	mov	DWORD [edi+4], 48645
	jmp	.L9186
.L4076:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 41985
	jmp	.L8695
.L4088:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1090817
	jmp	.L8695
.L4079:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movsd_insn
	mov	DWORD [edi+4], 4
	jmp	.L8695
.L4082:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 4236545
	jmp	.L8695
.L4085:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movss_insn
	mov	DWORD [edi+4], 3
	jmp	.L9152
.L4123:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], movsxd_insn
	mov	DWORD [edi+4], 1
	jmp	.L9148
.L4070:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L4149
	cmp	dl, 100
	jne	.L1005
.L4149:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L4152
	cmp	dl, 113
	jne	.L1005
.L4152:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movq2dq_insn
	jmp	.L9262
.L4065:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	jg	.L4157
	cmp	dl, 72
	jg	.L4158
	cmp	dl, 68
	jne	.L1005
	jmp	.L4170
.L4158:
	cmp	dl, 73
	jle	.L4162
	cmp	dl, 79
	jle	.L1005
	cmp	dl, 80
	jle	.L4165
	jmp	.L4166
.L4157:
	cmp	dl, 105
	jg	.L4168
	cmp	dl, 100
	je	.L4170
	cmp	dl, 104
	jle	.L1005
	jmp	.L4162
.L4168:
	cmp	dl, 111
	jle	.L1005
	cmp	dl, 112
	jle	.L4165
	cmp	dl, 113
	jle	.L4166
	jmp	.L1005
.L4170:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L4177
	cmp	dl, 113
	je	.L4177
	jmp	.L1005
.L4162:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movnti_insn
	mov	DWORD [edi+4], 2
.L8847:
	mov	DWORD [edi+8], 128
	jmp	.L8696
.L4165:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4182
	cmp	dl, 68
	je	.L4184
	cmp	dl, 82
	jle	.L1005
	jmp	.L4186
.L4182:
	cmp	dl, 100
	jg	.L4188
	cmp	dl, 99
	jle	.L1005
	jmp	.L4184
.L4188:
	cmp	dl, 115
	je	.L4186
	jmp	.L1005
.L4166:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movntq_insn
	jmp	.L9265
.L4186:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movntps_insn
	jmp	.L9265
.L4184:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movntpddq_insn
	mov	DWORD [edi+4], 11009
	jmp	.L9151
.L4177:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movntpddq_insn
	mov	DWORD [edi+4], 59137
	jmp	.L9151
.L4062:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 75
	je	.L4205
	cmp	dl, 107
	jne	.L1005
.L4205:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L4208
	cmp	dl, 112
	jne	.L1005
.L4208:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4210
	cmp	dl, 68
	je	.L4212
	cmp	dl, 82
	jle	.L1005
	jmp	.L4214
.L4210:
	cmp	dl, 100
	jg	.L4216
	cmp	dl, 99
	jle	.L1005
	jmp	.L4212
.L4216:
	cmp	dl, 115
	je	.L4214
	jmp	.L1005
.L4212:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movmskpd_insn
	jmp	.L9262
.L4214:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movmskps_insn
	jmp	.L9265
.L4053:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L4227
	cmp	dl, 112
	je	.L4227
	jmp	.L1005
.L4055:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4229
	cmp	dl, 68
	je	.L4231
	cmp	dl, 82
	jle	.L1005
	jmp	.L4233
.L4229:
	cmp	dl, 100
	jg	.L4235
	cmp	dl, 99
	jle	.L1005
	jmp	.L4231
.L4235:
	cmp	dl, 115
	je	.L4233
	jmp	.L1005
.L4231:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movhlpd_insn
	mov	DWORD [edi+4], 4610
	jmp	.L9151
.L4233:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movhlps_insn
	mov	DWORD [edi+4], 4610
	jmp	.L9152
.L4227:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L4246
	cmp	dl, 115
	jne	.L1005
.L4246:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movhllhps_insn
	mov	DWORD [edi+4], 5633
	jmp	.L9152
.L4043:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L4252
	cmp	dl, 112
	je	.L4252
	jmp	.L1005
.L4045:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4254
	cmp	dl, 68
	je	.L4256
	cmp	dl, 82
	jle	.L1005
	jmp	.L4258
.L4254:
	cmp	dl, 100
	jg	.L4260
	cmp	dl, 99
	jle	.L1005
	jmp	.L4256
.L4260:
	cmp	dl, 115
	je	.L4258
	jmp	.L1005
.L4256:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movhlpd_insn
	mov	DWORD [edi+4], 5634
	jmp	.L9151
.L4258:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movhlps_insn
	mov	DWORD [edi+4], 5634
	jmp	.L9152
.L4252:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L4271
	cmp	dl, 115
	jne	.L1005
.L4271:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movhllhps_insn
	mov	DWORD [edi+4], 4609
	jmp	.L9152
.L4036:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L4276
	cmp	dl, 50
	jg	.L4277
	cmp	dl, 49
	jle	.L1005
	jmp	.L4282
.L4277:
	cmp	dl, 65
	je	.L4281
	jmp	.L1005
.L4276:
	cmp	dl, 97
	jg	.L4283
	cmp	dl, 85
	jle	.L4285
	cmp	dl, 96
	jle	.L1005
	jmp	.L4281
.L4283:
	cmp	dl, 117
	je	.L4285
	jmp	.L1005
.L4282:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L4290
	cmp	dl, 113
	je	.L4290
	jmp	.L1005
.L4281:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movdqau_insn
	mov	DWORD [edi+4], 26114
	jmp	.L9151
.L4285:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movdqau_insn
	mov	DWORD [edi+4], 62210
	jmp	.L9151
.L4290:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movdq2q_insn
	jmp	.L9262
.L4030:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4301
	cmp	dl, 68
	je	.L4303
	cmp	dl, 82
	jle	.L1005
	jmp	.L4305
.L4301:
	cmp	dl, 100
	jg	.L4307
	cmp	dl, 99
	jle	.L1005
	jmp	.L4303
.L4307:
	cmp	dl, 115
	je	.L4305
	jmp	.L1005
.L4303:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movaupd_insn
	mov	DWORD [edi+4], 10242
	jmp	.L9151
.L4305:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], movaups_insn
	mov	DWORD [edi+4], 10242
	jmp	.L9152
.L3917:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	movsx	eax, BYTE [ebx+2]
	sub	eax, 48
	or	eax, 112
	jmp	.L9201
.L3913:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4318
	cmp	dl, 80
	je	.L4320
	cmp	dl, 82
	jle	.L1005
	jmp	.L4322
.L4318:
	cmp	dl, 112
	jg	.L4324
	cmp	dl, 111
	jle	.L1005
	jmp	.L4320
.L4324:
	cmp	dl, 115
	je	.L4322
	jmp	.L1005
.L4320:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4328
	cmp	dl, 68
	je	.L4330
	cmp	dl, 82
	jle	.L1005
	jmp	.L4332
.L4328:
	cmp	dl, 100
	jg	.L4334
	cmp	dl, 99
	jle	.L1005
	jmp	.L4330
.L4334:
	cmp	dl, 115
	je	.L4332
	jmp	.L1005
.L4322:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4338
	cmp	dl, 68
	je	.L4340
	cmp	dl, 82
	jle	.L1005
	jmp	.L4342
.L4338:
	cmp	dl, 100
	jg	.L4343
	cmp	dl, 99
	jle	.L1005
	jmp	.L4340
.L4343:
	cmp	dl, 115
	jne	.L1005
.L4342:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15949057
	jmp	.L9152
.L4340:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15883521
	jmp	.L9151
.L4332:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 23809
	jmp	.L9152
.L4330:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6708481
	jmp	.L9151
.L3910:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L4360
	cmp	dl, 110
	jne	.L1005
.L4360:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L4363
	cmp	dl, 99
	jne	.L1005
.L4363:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L4366
	cmp	dl, 101
	jne	.L1005
.L4366:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], threebyte_insn
	mov	DWORD [edi+4], 263122945
	jmp	.L9024
.L3901:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 75
	je	.L4372
	cmp	dl, 107
	je	.L4372
	jmp	.L1005
.L3903:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4374
	cmp	dl, 80
	je	.L4376
	cmp	dl, 82
	jle	.L1005
	jmp	.L4378
.L4374:
	cmp	dl, 112
	jg	.L4380
	cmp	dl, 111
	jle	.L1005
	jmp	.L4376
.L4380:
	cmp	dl, 115
	je	.L4378
	jmp	.L1005
.L4376:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4384
	cmp	dl, 68
	je	.L4386
	cmp	dl, 82
	jle	.L1005
	jmp	.L4388
.L4384:
	cmp	dl, 100
	jg	.L4390
	cmp	dl, 99
	jle	.L1005
	jmp	.L4386
.L4390:
	cmp	dl, 115
	je	.L4388
	jmp	.L1005
.L4378:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4394
	cmp	dl, 68
	je	.L4396
	cmp	dl, 82
	jle	.L1005
	jmp	.L4398
.L4394:
	cmp	dl, 100
	jg	.L4399
	cmp	dl, 99
	jle	.L1005
	jmp	.L4396
.L4399:
	cmp	dl, 115
	jne	.L1005
.L4398:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15949569
	jmp	.L9152
.L4396:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15884033
	jmp	.L9151
.L4388:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 24321
	jmp	.L9152
.L4386:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6708993
	jmp	.L9151
.L4372:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L4416
	cmp	dl, 109
	jne	.L1005
.L4416:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	je	.L4419
	cmp	dl, 111
	jne	.L1005
.L4419:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L4422
	cmp	dl, 118
	jne	.L1005
.L4422:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	jg	.L4424
	cmp	dl, 68
	je	.L4426
	cmp	dl, 80
	jle	.L1005
	jmp	.L4428
.L4424:
	cmp	dl, 100
	jg	.L4430
	cmp	dl, 99
	jle	.L1005
	jmp	.L4426
.L4430:
	cmp	dl, 113
	je	.L4428
	jmp	.L1005
.L4426:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L4435
	cmp	dl, 113
	je	.L4435
	jmp	.L1005
.L4428:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], maskmovq_insn
.L9240:
	mov	DWORD [edi+4], 1
.L8825:
	mov	DWORD [edi+8], 8256
	jmp	.L8696
.L4435:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	je	.L4441
	cmp	dl, 117
	jne	.L1005
.L4441:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], maskmovdqu_insn
.L9262:
	mov	DWORD [edi+4], 1
	jmp	.L9151
.L1305:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4446
	cmp	dl, 65
	je	.L4448
	cmp	dl, 82
	jle	.L1005
	jmp	.L4450
.L4446:
	cmp	dl, 97
	jg	.L4452
	cmp	dl, 96
	jle	.L1005
	jmp	.L4448
.L4452:
	cmp	dl, 115
	je	.L4450
	jmp	.L1005
.L1308:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L4457
	cmp	dl, 99
	je	.L4457
	jmp	.L1005
.L1311:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 22
	jmp	.L8699
.L1314:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	jg	.L4460
	cmp	dl, 75
	jg	.L4461
	test	dl, dl
	jg	.L999
	jmp	.L4468
.L4461:
	cmp	dl, 76
	jle	.L4465
	cmp	dl, 85
	jle	.L999
	jmp	.L4467
.L4460:
	cmp	dl, 108
	jg	.L4469
	cmp	dl, 107
	jle	.L999
	jmp	.L4465
.L4469:
	cmp	dl, 118
	je	.L4467
	jmp	.L999
.L4468:
	mov	DWORD [edi], 55
	jmp	.L8699
.L1317:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 18
	jmp	.L8699
.L1320:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 47
	jle	.L1005
	cmp	dl, 55
	jle	.L4476
	jmp	.L1005
.L1323:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L4478
	push	ebx
	push	DWORD LC16
	push	esi
	push	DWORD 0
	call	yasm__warning
	add	esp, 16
.L4478:
	mov	DWORD [edi], 15875
	jmp	.L8700
.L1326:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 50
	jmp	.L8699
.L4476:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	movsx	eax, BYTE [ebx+2]
	sub	eax, 48
	or	eax, 160
	jmp	.L9201
.L4465:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 39
	jmp	.L8699
.L4467:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4483
	cmp	dl, 79
	jg	.L4484
	test	dl, dl
	jg	.L999
	jmp	.L4491
.L4484:
	cmp	dl, 80
	jle	.L4488
	cmp	dl, 82
	jle	.L999
	jmp	.L4490
.L4483:
	cmp	dl, 112
	jg	.L4492
	cmp	dl, 111
	jle	.L999
	jmp	.L4488
.L4492:
	cmp	dl, 115
	je	.L4490
	jmp	.L999
.L4491:
	mov	DWORD [edi], f6_insn
	mov	DWORD [edi+4], 1540
	jmp	.L8695
.L4488:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4498
	cmp	dl, 68
	je	.L4500
	cmp	dl, 82
	jle	.L1005
	jmp	.L4502
.L4498:
	cmp	dl, 100
	jg	.L4504
	cmp	dl, 99
	jle	.L1005
	jmp	.L4500
.L4504:
	cmp	dl, 115
	je	.L4502
	jmp	.L1005
.L4490:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4508
	cmp	dl, 68
	je	.L4510
	cmp	dl, 82
	jle	.L1005
	jmp	.L4512
.L4508:
	cmp	dl, 100
	jg	.L4513
	cmp	dl, 99
	jle	.L1005
	jmp	.L4510
.L4513:
	cmp	dl, 115
	jne	.L1005
.L4512:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15949313
	jmp	.L9152
.L4510:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15883777
	jmp	.L9151
.L4502:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 24065
	jmp	.L9152
.L4500:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6708737
	jmp	.L9151
.L4457:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], incdec_insn
	mov	DWORD [edi+4], 83974
	jmp	.L8695
.L4448:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 9985
	jmp	.L8695
.L4450:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 12033
	jmp	.L8695
.L1263:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L4545
	cmp	dl, 108
	je	.L4545
	jmp	.L1005
.L1266:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L4548
	cmp	dl, 119
	je	.L4548
	jmp	.L1005
.L1269:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	jg	.L4550
	cmp	dl, 79
	je	.L4552
	cmp	dl, 80
	jle	.L1005
	jmp	.L4554
.L4550:
	cmp	dl, 111
	jg	.L4556
	cmp	dl, 110
	jle	.L1005
	jmp	.L4552
.L4556:
	cmp	dl, 113
	je	.L4554
	jmp	.L1005
.L1272:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 21
	jmp	.L8699
.L1275:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L4561
	cmp	dl, 69
	jg	.L4562
	cmp	dl, 66
	jg	.L4563
	test	dl, dl
	jg	.L999
	jmp	.L4579
.L4563:
	cmp	dl, 67
	jle	.L4567
	cmp	dl, 68
	jle	.L4569
	jmp	.L999
.L4562:
	cmp	dl, 72
	jg	.L4571
	cmp	dl, 70
	jle	.L4573
	jmp	.L999
.L4571:
	cmp	dl, 73
	jle	.L4576
	cmp	dl, 83
	jle	.L999
	jmp	.L4578
.L4561:
	cmp	dl, 102
	jg	.L4580
	cmp	dl, 99
	jg	.L4581
	cmp	dl, 98
	jle	.L999
	jmp	.L4567
.L4581:
	cmp	dl, 100
	jle	.L4569
	cmp	dl, 101
	jle	.L999
	jmp	.L4573
.L4580:
	cmp	dl, 105
	jg	.L4587
	cmp	dl, 104
	jle	.L999
	jmp	.L4576
.L4587:
	cmp	dl, 116
	je	.L4578
	jmp	.L999
.L4579:
	mov	DWORD [edi], 17
	jmp	.L8699
.L1278:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L4591
	cmp	dl, 67
	jg	.L4592
	cmp	dl, 66
	jle	.L1005
	jmp	.L4594
.L4592:
	cmp	dl, 78
	jle	.L1005
	cmp	dl, 79
	jle	.L4598
	jmp	.L4599
.L4591:
	cmp	dl, 110
	jg	.L4601
	cmp	dl, 99
	je	.L4594
	jmp	.L1005
.L4601:
	cmp	dl, 111
	jle	.L4598
	cmp	dl, 112
	jle	.L4599
	jmp	.L1005
.L1281:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L4607
	cmp	dl, 109
	je	.L4607
	jmp	.L1005
.L1284:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	je	.L4610
	cmp	dl, 117
	je	.L4610
	jmp	.L1005
.L1287:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 49
	jg	.L4612
	cmp	dl, 48
	jmp	.L8663
.L4612:
	cmp	dl, 52
	jle	.L4614
	cmp	dl, 56
.L8663:
	je	.L4614
	jmp	.L1005
.L1290:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 11777
	jmp	.L8700
.L1293:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L4620
	cmp	dl, 116
	je	.L4620
	jmp	.L1005
.L1296:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L4623
	cmp	dl, 100
	je	.L4623
	jmp	.L1005
.L1299:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 49
	jmp	.L8699
.L4623:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L4626
	test	dl, dl
	jle	.L4628
	cmp	dl, 68
	jle	.L999
	jmp	.L4630
.L4626:
	cmp	dl, 101
	je	.L4630
	jmp	.L999
.L4628:
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1087745
	jmp	.L8695
.L4630:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2136065
	jmp	.L9186
.L4620:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L4638
	cmp	dl, 79
	jg	.L4639
	cmp	dl, 68
	jne	.L1005
	jmp	.L4651
.L4639:
	cmp	dl, 80
	jle	.L4643
	cmp	dl, 82
	jle	.L1005
	cmp	dl, 83
	jle	.L4646
	jmp	.L4647
.L4638:
	cmp	dl, 112
	jg	.L4649
	cmp	dl, 100
	je	.L4651
	cmp	dl, 111
	jle	.L1005
	jmp	.L4643
.L4649:
	cmp	dl, 114
	jle	.L1005
	cmp	dl, 115
	jle	.L4646
	cmp	dl, 116
	jle	.L4647
	jmp	.L1005
.L4651:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L4658
	cmp	dl, 113
	je	.L4658
	jmp	.L1005
.L4643:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4660
	cmp	dl, 72
	jg	.L4661
	cmp	dl, 68
	jmp	.L8662
.L4661:
	cmp	dl, 73
	jle	.L4666
	cmp	dl, 82
	jle	.L1005
	jmp	.L4668
.L4660:
	cmp	dl, 104
	jg	.L4670
	cmp	dl, 100
.L8662:
	je	.L4663
	jmp	.L1005
.L4670:
	cmp	dl, 105
	jle	.L4666
	cmp	dl, 115
	je	.L4668
	jmp	.L1005
.L4646:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4675
	cmp	dl, 72
	jg	.L4676
	cmp	dl, 68
	jmp	.L8661
.L4676:
	cmp	dl, 73
	jle	.L4681
	cmp	dl, 82
	jle	.L1005
	jmp	.L4683
.L4675:
	cmp	dl, 104
	jg	.L4685
	cmp	dl, 100
.L8661:
	je	.L4678
	jmp	.L1005
.L4685:
	cmp	dl, 105
	jle	.L4681
	cmp	dl, 115
	je	.L4683
	jmp	.L1005
.L4647:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4690
	cmp	dl, 80
	je	.L4692
	cmp	dl, 82
	jle	.L1005
	jmp	.L4694
.L4690:
	cmp	dl, 112
	jg	.L4696
	cmp	dl, 111
	jle	.L1005
	jmp	.L4692
.L4696:
	cmp	dl, 115
	je	.L4694
	jmp	.L1005
.L4692:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4700
	cmp	dl, 68
	je	.L4702
	cmp	dl, 82
	jle	.L1005
	jmp	.L4704
.L4700:
	cmp	dl, 100
	jg	.L4706
	cmp	dl, 99
	jle	.L1005
	jmp	.L4702
.L4706:
	cmp	dl, 115
	je	.L4704
	jmp	.L1005
.L4694:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4710
	cmp	dl, 68
	je	.L4712
	cmp	dl, 82
	jle	.L1005
	jmp	.L4714
.L4710:
	cmp	dl, 100
	jg	.L4716
	cmp	dl, 99
	jle	.L1005
	jmp	.L4712
.L4716:
	cmp	dl, 115
	je	.L4714
	jmp	.L1005
.L4712:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	je	.L4721
	jmp	.L1005
.L4714:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	jne	.L1005
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L4724
	cmp	dl, 115
	jne	.L1005
.L4724:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L4727
	cmp	dl, 105
	jne	.L1005
.L4727:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15936513
	jmp	.L9152
.L4721:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L4733
	cmp	dl, 115
	jne	.L1005
.L4733:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L4736
	cmp	dl, 105
	jne	.L1005
.L4736:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15870977
	jmp	.L9151
.L4702:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	je	.L4742
	jmp	.L1005
.L4704:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	jne	.L1005
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L4744
	cmp	dl, 68
	je	.L4746
	cmp	dl, 79
	jle	.L1005
	jmp	.L4748
.L4744:
	cmp	dl, 100
	jg	.L4750
	cmp	dl, 99
	jle	.L1005
	jmp	.L4746
.L4750:
	cmp	dl, 112
	je	.L4748
	jmp	.L1005
.L4746:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L4755
	cmp	dl, 113
	je	.L4755
	jmp	.L1005
.L4748:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L4758
	cmp	dl, 105
	jne	.L1005
.L4758:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 11265
	jmp	.L9152
.L4755:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15948545
	jmp	.L9151
.L4742:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L4766
	cmp	dl, 68
	je	.L4768
	cmp	dl, 79
	jle	.L1005
	jmp	.L4770
.L4766:
	cmp	dl, 100
	jg	.L4772
	cmp	dl, 99
	jle	.L1005
	jmp	.L4768
.L4772:
	cmp	dl, 112
	je	.L4770
	jmp	.L1005
.L4768:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L4777
	cmp	dl, 113
	je	.L4777
	jmp	.L1005
.L4770:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L4780
	cmp	dl, 105
	jne	.L1005
.L4780:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6695937
	jmp	.L9151
.L4777:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6743553
	jmp	.L9151
.L4681:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	je	.L4789
	jmp	.L1005
.L4683:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	je	.L4791
	jmp	.L1005
.L4678:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	jne	.L1005
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L4794
	cmp	dl, 115
	jne	.L1005
.L4794:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4796
	cmp	dl, 73
	je	.L4798
	cmp	dl, 82
	jle	.L1005
	jmp	.L4800
.L4796:
	cmp	dl, 105
	jg	.L4802
	cmp	dl, 104
	jle	.L1005
	jmp	.L4798
.L4802:
	cmp	dl, 115
	je	.L4800
	jmp	.L1005
.L4798:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15871233
	jmp	.L9151
.L4800:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15882753
	jmp	.L9151
.L4791:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L4813
	cmp	dl, 115
	jne	.L1005
.L4813:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	jg	.L4815
	cmp	dl, 68
	je	.L4817
	cmp	dl, 72
	jle	.L1005
	jmp	.L4819
.L4815:
	cmp	dl, 100
	jg	.L4821
	cmp	dl, 99
	jle	.L1005
	jmp	.L4817
.L4821:
	cmp	dl, 105
	je	.L4819
	jmp	.L1005
.L4817:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15948289
	jmp	.L9151
.L4819:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15936769
	jmp	.L9152
.L4789:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L4832
	cmp	dl, 115
	jne	.L1005
.L4832:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4834
	cmp	dl, 68
	je	.L4836
	cmp	dl, 82
	jle	.L1005
	jmp	.L4838
.L4834:
	cmp	dl, 100
	jg	.L4840
	cmp	dl, 99
	jle	.L1005
	jmp	.L4836
.L4840:
	cmp	dl, 115
	je	.L4838
	jmp	.L1005
.L4836:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15870465
	jmp	.L9151
.L4838:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15936001
	jmp	.L9152
.L4666:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	je	.L4851
	jmp	.L1005
.L4668:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	je	.L4853
	jmp	.L1005
.L4663:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	jne	.L1005
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L4855
	cmp	dl, 68
	je	.L4857
	cmp	dl, 79
	jle	.L1005
	jmp	.L4859
.L4855:
	cmp	dl, 100
	jg	.L4861
	cmp	dl, 99
	jle	.L1005
	jmp	.L4857
.L4861:
	cmp	dl, 112
	je	.L4859
	jmp	.L1005
.L4857:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L4866
	cmp	dl, 113
	je	.L4866
	jmp	.L1005
.L4859:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4868
	cmp	dl, 73
	je	.L4870
	cmp	dl, 82
	jle	.L1005
	jmp	.L4872
.L4868:
	cmp	dl, 105
	jg	.L4874
	cmp	dl, 104
	jle	.L1005
	jmp	.L4870
.L4874:
	cmp	dl, 115
	je	.L4872
	jmp	.L1005
.L4870:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6696193
	jmp	.L9151
.L4872:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6707713
	jmp	.L9151
.L4866:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15918593
	jmp	.L9151
.L4853:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L4887
	cmp	dl, 68
	je	.L4889
	cmp	dl, 79
	jle	.L1005
	jmp	.L4891
.L4887:
	cmp	dl, 100
	jg	.L4892
	cmp	dl, 99
	jle	.L1005
	jmp	.L4889
.L4892:
	cmp	dl, 112
	jne	.L1005
.L4891:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	jg	.L4896
	cmp	dl, 68
	je	.L4898
	cmp	dl, 72
	jle	.L1005
	jmp	.L4900
.L4896:
	cmp	dl, 100
	jg	.L4902
	cmp	dl, 99
	jle	.L1005
	jmp	.L4898
.L4902:
	cmp	dl, 105
	je	.L4900
	jmp	.L1005
.L4889:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L4907
	cmp	dl, 113
	jne	.L1005
.L4907:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6707969
	jmp	.L9151
.L4898:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 23041
	jmp	.L9151
.L4900:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 11521
	jmp	.L9152
.L4851:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L4919
	cmp	dl, 112
	jne	.L1005
.L4919:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4921
	cmp	dl, 68
	je	.L4923
	cmp	dl, 82
	jle	.L1005
	jmp	.L4925
.L4921:
	cmp	dl, 100
	jg	.L4927
	cmp	dl, 99
	jle	.L1005
	jmp	.L4923
.L4927:
	cmp	dl, 115
	je	.L4925
	jmp	.L1005
.L4923:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6695425
	jmp	.L9151
.L4925:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 10753
	jmp	.L9152
.L4658:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	jne	.L1005
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L4939
	cmp	dl, 112
	jne	.L1005
.L4939:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4941
	cmp	dl, 68
	je	.L4943
	cmp	dl, 82
	jle	.L1005
	jmp	.L4945
.L4941:
	cmp	dl, 100
	jg	.L4947
	cmp	dl, 99
	jle	.L1005
	jmp	.L4943
.L4947:
	cmp	dl, 115
	je	.L4945
	jmp	.L1005
.L4943:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15984129
	jmp	.L9151
.L4945:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 23297
	jmp	.L9151
.L4614:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L4958
	cmp	BYTE [ebx+2], 56
	je	.L9215
.L4958:
	movsx	eax, BYTE [ebx+2]
	sub	eax, 48
	or	eax, 144
	jmp	.L9201
.L4610:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L4960
	cmp	dl, 105
	jne	.L1005
.L4960:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L4963
	cmp	dl, 100
	jne	.L1005
.L4963:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 1024513
	jmp	.L8930
.L4607:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L4969
	cmp	dl, 105
	jne	.L1005
.L4969:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L4972
	cmp	dl, 115
	jne	.L1005
.L4972:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L4974
	cmp	dl, 68
	je	.L4976
	cmp	dl, 82
	jle	.L1005
	jmp	.L4978
.L4974:
	cmp	dl, 100
	jg	.L4980
	cmp	dl, 99
	jle	.L1005
	jmp	.L4976
.L4980:
	cmp	dl, 115
	je	.L4978
	jmp	.L1005
.L4976:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6696705
	jmp	.L9151
.L4978:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 12033
	jmp	.L9152
.L4594:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 62721
	jmp	.L8695
.L4599:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	jg	.L4993
	cmp	dl, 78
	jg	.L4994
	cmp	dl, 69
	jg	.L4995
	test	dl, dl
	jle	.L4997
	cmp	dl, 68
	jle	.L999
	jmp	.L4999
.L4995:
	cmp	dl, 76
	je	.L5002
	cmp	dl, 77
	jle	.L999
	jmp	.L5004
.L4994:
	cmp	dl, 83
	jg	.L5006
	cmp	dl, 79
	jle	.L5008
	cmp	dl, 80
	jle	.L5010
	cmp	dl, 82
	jle	.L999
	jmp	.L5012
.L5006:
	cmp	dl, 85
	je	.L5015
	cmp	dl, 87
	jle	.L999
	jmp	.L5017
.L4993:
	cmp	dl, 111
	jg	.L5019
	cmp	dl, 107
	jg	.L5020
	cmp	dl, 101
	je	.L4999
	jmp	.L999
.L5020:
	cmp	dl, 108
	jle	.L5002
	cmp	dl, 109
	jle	.L999
	cmp	dl, 110
	jle	.L5004
	jmp	.L5008
.L5019:
	cmp	dl, 116
	jg	.L5027
	cmp	dl, 112
	jle	.L5010
	cmp	dl, 115
	je	.L5012
	jmp	.L999
.L5027:
	cmp	dl, 117
	jle	.L5015
	cmp	dl, 120
	je	.L5017
	jmp	.L999
.L4997:
	mov	DWORD [edi], arith_insn
	mov	DWORD [edi+4], 473111
	jmp	.L8695
.L4598:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L5036
	cmp	dl, 118
	jne	.L1005
.L5036:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 57
	ja	.L1005
	jmp	DWORD [.L5073+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L5073:
	dd	.L5041
	dd	.L5044
	dd	.L5195
	dd	.L1005
	dd	.L5071
	dd	.L1005
	dd	.L5053
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5056
	dd	.L1005
	dd	.L5059
	dd	.L5062
	dd	.L5065
	dd	.L1005
	dd	.L1005
	dd	.L5068
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5071
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5041
	dd	.L5044
	dd	.L5195
	dd	.L1005
	dd	.L5071
	dd	.L1005
	dd	.L5053
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5056
	dd	.L1005
	dd	.L5059
	dd	.L5062
	dd	.L5065
	dd	.L1005
	dd	.L1005
	dd	.L5068
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5071
	[section .text]
.L5062:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 3
	jmp	.L9010
.L5059:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 57
	ja	.L1005
	jmp	DWORD [.L5109+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L5109:
	dd	.L5080
	dd	.L5083
	dd	.L5086
	dd	.L1005
	dd	.L5107
	dd	.L1005
	dd	.L5092
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5095
	dd	.L1005
	dd	.L1005
	dd	.L5098
	dd	.L5101
	dd	.L1005
	dd	.L1005
	dd	.L5104
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5107
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5080
	dd	.L5083
	dd	.L5086
	dd	.L1005
	dd	.L5107
	dd	.L1005
	dd	.L5092
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5095
	dd	.L1005
	dd	.L1005
	dd	.L5098
	dd	.L5101
	dd	.L1005
	dd	.L1005
	dd	.L5104
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5107
	[section .text]
.L5044:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L5110
	test	dl, dl
	jle	.L9256
	cmp	dl, 68
	jle	.L999
	jmp	.L5114
.L5110:
	cmp	dl, 101
	je	.L5114
	jmp	.L999
.L5112:
.L5047:
.L5041:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L5122
	test	dl, dl
	jle	.L9255
	cmp	dl, 68
	jle	.L999
	jmp	.L5086
.L5122:
	cmp	dl, 101
	je	.L5086
	jmp	.L999
.L5124:
.L5050:
.L5071:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 1027
	jmp	.L9010
.L5068:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 2051
	jmp	.L9010
.L5065:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	jg	.L5140
	cmp	dl, 68
	jg	.L5141
	test	dl, dl
	jmp	.L9268
.L5141:
	cmp	dl, 69
	jle	.L5145
	cmp	dl, 78
	jle	.L999
	jmp	.L5101
.L5140:
	cmp	dl, 101
	jg	.L5149
	cmp	dl, 100
	jle	.L999
	jmp	.L5145
.L5149:
	cmp	dl, 111
	je	.L5101
	jmp	.L999
.L5056:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L5155
	test	dl, dl
	jle	.L9252
	cmp	dl, 68
	jle	.L999
	jmp	.L5159
.L5155:
	cmp	dl, 101
	je	.L5159
	jmp	.L999
.L5157:
.L5053:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L5164
	test	dl, dl
	jle	.L9251
	cmp	dl, 68
	jle	.L999
	jmp	.L5168
.L5164:
	cmp	dl, 101
	jne	.L999
.L5166:
.L5168:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	jmp	.L5226
.L5159:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	jmp	.L5217
.L5145:
	inc	ecx
	cmp	BYTE [ecx], 0
.L9268:
	jg	.L999
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 2563
	jmp	.L9010
.L5147:
.L5126:
.L5114:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	jmp	.L5193
.L5080:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L5191
	test	dl, dl
	jle	.L5193
	cmp	dl, 68
	jle	.L999
	jmp	.L5195
.L5191:
	cmp	dl, 101
	je	.L5195
	jmp	.L999
.L5193:
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 1539
	jmp	.L9010
.L5083:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L5200
	test	dl, dl
	jle	.L9249
	cmp	dl, 68
	jle	.L999
	jmp	.L5204
.L5200:
	cmp	dl, 101
	je	.L5204
	jmp	.L999
.L5202:
.L5086:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9249:
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 771
	jmp	.L9010
.L5089:
.L5092:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L5215
	test	dl, dl
	jle	.L5217
	cmp	dl, 68
	jle	.L999
	jmp	.L5219
.L5215:
	cmp	dl, 101
	je	.L5219
	jmp	.L999
.L5217:
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 3587
	jmp	.L9010
.L5095:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L5224
	test	dl, dl
	jle	.L5226
	cmp	dl, 68
	jle	.L999
	jmp	.L5228
.L5224:
	cmp	dl, 101
	je	.L5228
	jmp	.L999
.L5226:
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 3331
	jmp	.L9010
.L5098:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 259
	jmp	.L9010
.L5101:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 2819
	jmp	.L9010
.L5104:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 2307
	jmp	.L9010
.L5107:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 1283
	jmp	.L9010
.L5228:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9251:
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 3843
	jmp	.L9010
.L5219:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9252:
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 3075
	jmp	.L9010
.L5204:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9255:
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 1795
	jmp	.L9010
.L5195:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9256:
	mov	DWORD [edi], cmovcc_insn
	mov	DWORD [edi+4], 515
	jmp	.L9010
.L4999:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L5258
	cmp	dl, 113
	je	.L5258
	jmp	.L1005
.L5002:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L5260
	cmp	dl, 69
	je	.L5262
	cmp	dl, 83
	jle	.L1005
	jmp	.L5264
.L5260:
	cmp	dl, 101
	jg	.L5266
	cmp	dl, 100
	jle	.L1005
	jmp	.L5262
.L5266:
	cmp	dl, 116
	je	.L5264
	jmp	.L1005
.L5004:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	jg	.L5270
	cmp	dl, 69
	je	.L5272
	cmp	dl, 75
	jle	.L1005
	jmp	.L5274
.L5270:
	cmp	dl, 101
	jg	.L5276
	cmp	dl, 100
	jle	.L1005
	jmp	.L5272
.L5276:
	cmp	dl, 108
	je	.L5274
	jmp	.L1005
.L5008:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L5281
	cmp	dl, 114
	je	.L5281
	jmp	.L1005
.L5010:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5283
	cmp	dl, 68
	je	.L5285
	cmp	dl, 82
	jle	.L1005
	jmp	.L5287
.L5283:
	cmp	dl, 100
	jg	.L5289
	cmp	dl, 99
	jle	.L1005
	jmp	.L5285
.L5289:
	cmp	dl, 115
	je	.L5287
	jmp	.L1005
.L5012:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 66
	cmp	eax, 53
	ja	.L1005
	jmp	DWORD [.L5310+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L5310:
	dd	.L5296
	dd	.L1005
	dd	.L5299
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5302
	dd	.L1005
	dd	.L5305
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5308
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5296
	dd	.L1005
	dd	.L5299
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5302
	dd	.L1005
	dd	.L5305
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5308
	[section .text]
.L5015:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L5312
	cmp	dl, 110
	je	.L5312
	jmp	.L1005
.L5017:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L5315
	cmp	dl, 99
	jne	.L1005
.L5315:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 72
	je	.L5318
	cmp	dl, 104
	jne	.L1005
.L5318:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 71
	je	.L5321
	cmp	dl, 103
	jne	.L1005
.L5321:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 52
	jg	.L5323
	test	dl, dl
	jle	.L5325
	cmp	dl, 51
	jle	.L999
	jmp	.L5327
.L5323:
	cmp	dl, 56
	je	.L5330
	jmp	.L999
.L5325:
	mov	DWORD [edi], cmpxchgxadd_insn
	mov	DWORD [edi+4], 45060
.L8930:
	mov	DWORD [edi+8], 8
	jmp	.L8696
.L5327:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 56
	je	.L5334
	jmp	.L1005
.L5330:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L5336
	cmp	dl, 98
	jne	.L1005
.L5336:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cmpxchg8b_insn
	mov	DWORD [edi+4], 1
	jmp	.L8963
.L5334:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 54
	jne	.L1005
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cmpxchgxadd_insn
	mov	DWORD [edi+4], 42500
	mov	DWORD [edi+8], 2097160
	jmp	.L8696
.L5312:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	je	.L5346
	cmp	dl, 111
	jne	.L1005
.L5346:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L5349
	cmp	dl, 114
	jne	.L1005
.L5349:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L5352
	cmp	dl, 100
	jne	.L1005
.L5352:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5354
	cmp	dl, 80
	je	.L5356
	cmp	dl, 82
	jle	.L1005
	jmp	.L5358
.L5354:
	cmp	dl, 112
	jg	.L5360
	cmp	dl, 111
	jle	.L1005
	jmp	.L5356
.L5360:
	cmp	dl, 115
	je	.L5358
	jmp	.L1005
.L5356:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5364
	cmp	dl, 68
	je	.L5366
	cmp	dl, 82
	jle	.L1005
	jmp	.L5368
.L5364:
	cmp	dl, 100
	jg	.L5370
	cmp	dl, 99
	jle	.L1005
	jmp	.L5366
.L5370:
	cmp	dl, 115
	je	.L5368
	jmp	.L1005
.L5358:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5374
	cmp	dl, 68
	je	.L5376
	cmp	dl, 82
	jle	.L1005
	jmp	.L5378
.L5374:
	cmp	dl, 100
	jg	.L5380
	cmp	dl, 99
	jle	.L1005
	jmp	.L5376
.L5380:
	cmp	dl, 115
	je	.L5378
	jmp	.L1005
.L5376:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 258561
	jmp	.L9151
.L5378:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 258817
	jmp	.L9152
.L5366:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 222721
	jmp	.L9151
.L5368:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpps_insn
	jmp	.L9264
.L5296:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 42497
	jmp	.L8695
.L5308:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1091329
	jmp	.L8695
.L5299:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cmpsd_insn
	jmp	.L9223
.L5302:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 4237057
	jmp	.L9148
.L5305:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssessimm_insn
	mov	DWORD [edi+4], 15974913
	jmp	.L9152
.L5287:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssepsimm_insn
	mov	DWORD [edi+4], 49665
	jmp	.L9152
.L5285:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssessimm_insn
	mov	DWORD [edi+4], 6734337
	jmp	.L9151
.L5281:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L5419
	cmp	dl, 100
	jne	.L1005
.L5419:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5421
	cmp	dl, 80
	je	.L5423
	cmp	dl, 82
	jle	.L1005
	jmp	.L5425
.L5421:
	cmp	dl, 112
	jg	.L5427
	cmp	dl, 111
	jle	.L1005
	jmp	.L5423
.L5427:
	cmp	dl, 115
	je	.L5425
	jmp	.L1005
.L5423:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5431
	cmp	dl, 68
	je	.L5433
	cmp	dl, 82
	jle	.L1005
	jmp	.L5435
.L5431:
	cmp	dl, 100
	jg	.L5437
	cmp	dl, 99
	jle	.L1005
	jmp	.L5433
.L5437:
	cmp	dl, 115
	je	.L5435
	jmp	.L1005
.L5425:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5441
	cmp	dl, 68
	je	.L5443
	cmp	dl, 82
	jle	.L1005
	jmp	.L5445
.L5441:
	cmp	dl, 100
	jg	.L5447
	cmp	dl, 99
	jle	.L1005
	jmp	.L5443
.L5447:
	cmp	dl, 115
	je	.L5445
	jmp	.L1005
.L5443:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 520705
	jmp	.L9151
.L5445:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 520961
	jmp	.L9152
.L5433:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 484865
	jmp	.L9151
.L5435:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpps_insn
	mov	DWORD [edi+4], 1793
	jmp	.L9152
.L5272:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 81
	je	.L5464
	cmp	dl, 113
	je	.L5464
	jmp	.L1005
.L5274:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L5466
	cmp	dl, 69
	je	.L5468
	cmp	dl, 83
	jle	.L1005
	jmp	.L5470
.L5466:
	cmp	dl, 101
	jg	.L5472
	cmp	dl, 100
	jle	.L1005
	jmp	.L5468
.L5472:
	cmp	dl, 116
	je	.L5470
	jmp	.L1005
.L5468:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5476
	cmp	dl, 80
	je	.L5478
	cmp	dl, 82
	jle	.L1005
	jmp	.L5480
.L5476:
	cmp	dl, 112
	jg	.L5482
	cmp	dl, 111
	jle	.L1005
	jmp	.L5478
.L5482:
	cmp	dl, 115
	je	.L5480
	jmp	.L1005
.L5470:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5486
	cmp	dl, 80
	je	.L5488
	cmp	dl, 82
	jle	.L1005
	jmp	.L5490
.L5486:
	cmp	dl, 112
	jg	.L5492
	cmp	dl, 111
	jle	.L1005
	jmp	.L5488
.L5492:
	cmp	dl, 115
	je	.L5490
	jmp	.L1005
.L5488:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5496
	cmp	dl, 68
	je	.L5498
	cmp	dl, 82
	jle	.L1005
	jmp	.L5500
.L5496:
	cmp	dl, 100
	jg	.L5502
	cmp	dl, 99
	jle	.L1005
	jmp	.L5498
.L5502:
	cmp	dl, 115
	je	.L5500
	jmp	.L1005
.L5490:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5506
	cmp	dl, 68
	je	.L5508
	cmp	dl, 82
	jle	.L1005
	jmp	.L5510
.L5506:
	cmp	dl, 100
	jg	.L5512
	cmp	dl, 99
	jle	.L1005
	jmp	.L5508
.L5512:
	cmp	dl, 115
	je	.L5510
	jmp	.L1005
.L5508:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 389633
	jmp	.L9151
.L5510:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 389889
	jmp	.L9152
.L5498:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 353793
	jmp	.L9151
.L5500:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpps_insn
	mov	DWORD [edi+4], 1281
	jmp	.L9152
.L5478:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5528
	cmp	dl, 68
	je	.L5530
	cmp	dl, 82
	jle	.L1005
	jmp	.L5532
.L5528:
	cmp	dl, 100
	jg	.L5534
	cmp	dl, 99
	jle	.L1005
	jmp	.L5530
.L5534:
	cmp	dl, 115
	je	.L5532
	jmp	.L1005
.L5480:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5538
	cmp	dl, 68
	je	.L5540
	cmp	dl, 82
	jle	.L1005
	jmp	.L5542
.L5538:
	cmp	dl, 100
	jg	.L5544
	cmp	dl, 99
	jle	.L1005
	jmp	.L5540
.L5544:
	cmp	dl, 115
	je	.L5542
	jmp	.L1005
.L5540:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 455169
	jmp	.L9151
.L5542:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 455425
	jmp	.L9152
.L5530:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 419329
	jmp	.L9151
.L5532:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpps_insn
	mov	DWORD [edi+4], 1537
	jmp	.L9152
.L5464:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5560
	cmp	dl, 80
	je	.L5562
	cmp	dl, 82
	jle	.L1005
	jmp	.L5564
.L5560:
	cmp	dl, 112
	jg	.L5566
	cmp	dl, 111
	jle	.L1005
	jmp	.L5562
.L5566:
	cmp	dl, 115
	je	.L5564
	jmp	.L1005
.L5562:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5570
	cmp	dl, 68
	je	.L5572
	cmp	dl, 82
	jle	.L1005
	jmp	.L5574
.L5570:
	cmp	dl, 100
	jg	.L5576
	cmp	dl, 99
	jle	.L1005
	jmp	.L5572
.L5576:
	cmp	dl, 115
	je	.L5574
	jmp	.L1005
.L5564:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5580
	cmp	dl, 68
	je	.L5582
	cmp	dl, 82
	jle	.L1005
	jmp	.L5584
.L5580:
	cmp	dl, 100
	jg	.L5586
	cmp	dl, 99
	jle	.L1005
	jmp	.L5582
.L5586:
	cmp	dl, 115
	je	.L5584
	jmp	.L1005
.L5582:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 324097
	jmp	.L9151
.L5584:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 324353
	jmp	.L9152
.L5572:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 288257
	jmp	.L9151
.L5574:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpps_insn
	mov	DWORD [edi+4], 1025
	jmp	.L9152
.L5262:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5602
	cmp	dl, 80
	je	.L5604
	cmp	dl, 82
	jle	.L1005
	jmp	.L5606
.L5602:
	cmp	dl, 112
	jg	.L5608
	cmp	dl, 111
	jle	.L1005
	jmp	.L5604
.L5608:
	cmp	dl, 115
	je	.L5606
	jmp	.L1005
.L5264:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5612
	cmp	dl, 80
	je	.L5614
	cmp	dl, 82
	jle	.L1005
	jmp	.L5616
.L5612:
	cmp	dl, 112
	jg	.L5618
	cmp	dl, 111
	jle	.L1005
	jmp	.L5614
.L5618:
	cmp	dl, 115
	je	.L5616
	jmp	.L1005
.L5614:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5622
	cmp	dl, 68
	je	.L5624
	cmp	dl, 82
	jle	.L1005
	jmp	.L5626
.L5622:
	cmp	dl, 100
	jg	.L5628
	cmp	dl, 99
	jle	.L1005
	jmp	.L5624
.L5628:
	cmp	dl, 115
	je	.L5626
	jmp	.L1005
.L5616:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5632
	cmp	dl, 68
	je	.L5634
	cmp	dl, 82
	jle	.L1005
	jmp	.L5636
.L5632:
	cmp	dl, 100
	jg	.L5637
	cmp	dl, 99
	jle	.L1005
	jmp	.L5634
.L5637:
	cmp	dl, 115
	jne	.L1005
.L5636:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 127745
	jmp	.L9152
.L5634:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 127489
	jmp	.L9151
.L5626:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpps_insn
	mov	DWORD [edi+4], 257
	jmp	.L9152
.L5624:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 91649
	jmp	.L9151
.L5604:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5653
	cmp	dl, 68
	je	.L5655
	cmp	dl, 82
	jle	.L1005
	jmp	.L5657
.L5653:
	cmp	dl, 100
	jg	.L5659
	cmp	dl, 99
	jle	.L1005
	jmp	.L5655
.L5659:
	cmp	dl, 115
	je	.L5657
	jmp	.L1005
.L5606:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5663
	cmp	dl, 68
	je	.L5665
	cmp	dl, 82
	jle	.L1005
	jmp	.L5667
.L5663:
	cmp	dl, 100
	jg	.L5668
	cmp	dl, 99
	jle	.L1005
	jmp	.L5665
.L5668:
	cmp	dl, 115
	jne	.L1005
.L5667:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 193281
	jmp	.L9152
.L5665:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 193025
	jmp	.L9151
.L5657:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpps_insn
	jmp	.L9263
.L5655:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 157185
	jmp	.L9151
.L5258:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5684
	cmp	dl, 80
	je	.L5686
	cmp	dl, 82
	jle	.L1005
	jmp	.L5688
.L5684:
	cmp	dl, 112
	jg	.L5690
	cmp	dl, 111
	jle	.L1005
	jmp	.L5686
.L5690:
	cmp	dl, 115
	je	.L5688
	jmp	.L1005
.L5686:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5694
	cmp	dl, 68
	je	.L5696
	cmp	dl, 82
	jle	.L1005
	jmp	.L5698
.L5694:
	cmp	dl, 100
	jg	.L5700
	cmp	dl, 99
	jle	.L1005
	jmp	.L5696
.L5700:
	cmp	dl, 115
	je	.L5698
	jmp	.L1005
.L5688:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5704
	cmp	dl, 68
	je	.L5706
	cmp	dl, 82
	jle	.L1005
	jmp	.L5708
.L5704:
	cmp	dl, 100
	jg	.L5709
	cmp	dl, 99
	jle	.L1005
	jmp	.L5706
.L5709:
	cmp	dl, 115
	jne	.L1005
.L5708:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 62209
	jmp	.L9152
.L5706:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 61953
	jmp	.L9151
.L5698:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpps_insn
.L9265:
	mov	DWORD [edi+4], 1
	jmp	.L9152
.L5696:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssecmpss_insn
	mov	DWORD [edi+4], 26113
	jmp	.L9151
.L4567:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 63489
	jmp	.L8695
.L4569:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 64513
	jmp	.L8695
.L4576:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 64001
	jmp	.L8695
.L4578:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L5735
	cmp	dl, 115
	je	.L5735
	jmp	.L1005
.L4573:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L5738
	cmp	dl, 108
	jne	.L1005
.L5738:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	je	.L5741
	cmp	dl, 117
	jne	.L1005
.L5741:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L5744
	cmp	dl, 115
	jne	.L1005
.L5744:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 72
	je	.L5747
	cmp	dl, 104
	jne	.L1005
.L5747:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], clflush_insn
	mov	DWORD [edi+4], 1
	jmp	.L9024
.L5735:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 984577
	jmp	.L9154
.L4554:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L5755
	test	dl, dl
	jle	.L5757
	cmp	dl, 68
	jle	.L999
	jmp	.L5759
.L5755:
	cmp	dl, 101
	je	.L5759
	jmp	.L999
.L5757:
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2136321
	jmp	.L9186
.L4552:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 4233473
	jmp	.L9148
.L5759:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 4233217
	jmp	.L9148
.L4548:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1087489
	jmp	.L8695
.L4545:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L5776
	cmp	dl, 108
	jne	.L1005
.L5776:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], call_insn
	jmp	.L9222
.L1230:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 47
	jle	.L1005
	cmp	dl, 53
	jle	.L5783
	jmp	.L1005
.L1233:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L5784
	cmp	dl, 66
	jg	.L5785
	test	dl, dl
	jle	.L5787
	cmp	dl, 65
	jle	.L999
	jmp	.L5789
.L5785:
	cmp	dl, 68
	je	.L5792
	cmp	dl, 86
	jle	.L999
	jmp	.L5794
.L5784:
	cmp	dl, 99
	jg	.L5796
	cmp	dl, 98
	je	.L5789
	jmp	.L999
.L5796:
	cmp	dl, 100
	jle	.L5792
	cmp	dl, 119
	je	.L5794
	jmp	.L999
.L5787:
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	movsx	eax, BYTE [ebx+1]
	sub	eax, 48
	jmp	.L9261
.L1236:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L5803
	cmp	dl, 120
	je	.L5803
	jmp	.L1005
.L1239:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	jg	.L5805
	cmp	dl, 80
	je	.L5807
	cmp	dl, 87
	jle	.L1005
	jmp	.L5809
.L5805:
	cmp	dl, 112
	jg	.L5811
	cmp	dl, 111
	jle	.L1005
	jmp	.L5807
.L5811:
	cmp	dl, 120
	je	.L5809
	jmp	.L1005
.L1242:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	jg	.L5815
	cmp	dl, 80
	jg	.L5816
	cmp	dl, 76
	je	.L5818
	cmp	dl, 79
	jmp	.L8684
.L5816:
	cmp	dl, 82
	je	.L5823
	cmp	dl, 87
	jle	.L1005
	jmp	.L5825
.L5815:
	cmp	dl, 112
	jg	.L5827
	cmp	dl, 108
	je	.L5818
	cmp	dl, 111
.L8684:
	jle	.L1005
	jmp	.L5820
.L5827:
	cmp	dl, 114
	jg	.L5831
	cmp	dl, 113
	jle	.L1005
	jmp	.L5823
.L5831:
	cmp	dl, 120
	je	.L5825
	jmp	.L1005
.L1245:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 73
	cmp	eax, 47
	ja	.L1005
	jmp	DWORD [.L5855+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L5855:
	dd	.L5838
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5841
	dd	.L1005
	dd	.L1005
	dd	.L5844
	dd	.L1005
	dd	.L1005
	dd	.L5847
	dd	.L5850
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5853
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5838
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5841
	dd	.L1005
	dd	.L1005
	dd	.L5844
	dd	.L1005
	dd	.L1005
	dd	.L5847
	dd	.L5850
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5853
	[section .text]
.L1248:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L5856
	cmp	dl, 80
	je	.L5858
	cmp	dl, 83
	jle	.L1005
	jmp	.L5860
.L5856:
	cmp	dl, 112
	jg	.L5862
	cmp	dl, 111
	jle	.L1005
	jmp	.L5858
.L5862:
	cmp	dl, 116
	je	.L5860
	jmp	.L1005
.L1251:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L5867
	cmp	dl, 112
	je	.L5867
	jmp	.L1005
.L1254:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L5869
	cmp	dl, 76
	je	.L5871
	cmp	dl, 81
	jle	.L1005
	jmp	.L5873
.L5869:
	cmp	dl, 108
	jg	.L5875
	cmp	dl, 107
	jle	.L1005
	jmp	.L5871
.L5875:
	cmp	dl, 114
	je	.L5873
	jmp	.L1005
.L1257:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 68
	cmp	eax, 48
	ja	.L1005
	jmp	DWORD [.L5902+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L5902:
	dd	.L5882
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5885
	dd	.L1005
	dd	.L1005
	dd	.L5888
	dd	.L5891
	dd	.L1005
	dd	.L1005
	dd	.L5894
	dd	.L5897
	dd	.L1005
	dd	.L1005
	dd	.L5900
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5882
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L5885
	dd	.L1005
	dd	.L1005
	dd	.L5888
	dd	.L5891
	dd	.L1005
	dd	.L1005
	dd	.L5894
	dd	.L5897
	dd	.L1005
	dd	.L1005
	dd	.L5900
	[section .text]
.L5894:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 84
	jmp	.L8699
.L5885:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 86
	jmp	.L8699
.L5891:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 1026561
	mov	DWORD [edi+8], 524304
	jmp	.L8696
.L5897:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L5911
	cmp	dl, 114
	je	.L5911
	jmp	.L1005
.L5882:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L5914
	cmp	dl, 99
	je	.L5914
	jmp	.L1005
.L5888:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L5917
	cmp	dl, 100
	je	.L5917
	jmp	.L1005
.L5900:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L5920
	cmp	dl, 115
	jne	.L1005
.L5920:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixsmm_insn
	mov	DWORD [edi+4], 32001
	jmp	.L9141
.L5917:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L5926
	cmp	dl, 116
	jne	.L1005
.L5926:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixsmm_insn
	mov	DWORD [edi+4], 31489
	jmp	.L9141
.L5914:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], rsdc_insn
	jmp	.L9257
.L5911:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L5935
	cmp	dl, 116
	jne	.L1005
.L5935:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L5937
	cmp	dl, 80
	je	.L5939
	cmp	dl, 82
	jle	.L1005
	jmp	.L5941
.L5937:
	cmp	dl, 112
	jg	.L5943
	cmp	dl, 111
	jle	.L1005
	jmp	.L5939
.L5943:
	cmp	dl, 115
	je	.L5941
	jmp	.L1005
.L5939:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L5948
	cmp	dl, 115
	je	.L5948
	jmp	.L1005
.L5941:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L5951
	cmp	dl, 115
	jne	.L1005
.L5951:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15946241
	jmp	.L9152
.L5948:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 20993
	jmp	.L9152
.L5871:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], shift_insn
	jmp	.L9221
.L5873:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], shift_insn
	jmp	.L9220
.L5867:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 192
	jmp	.L8699
.L5858:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	jg	.L5967
	cmp	dl, 69
	jg	.L5968
	test	dl, dl
	jle	.L5970
	cmp	dl, 68
	jmp	.L9273
.L5968:
	cmp	dl, 78
	je	.L5975
	cmp	dl, 89
.L9273:
	jle	.L999
	jmp	.L5977
.L5967:
	cmp	dl, 109
	jg	.L5979
	cmp	dl, 101
	jmp	.L9275
.L5979:
	cmp	dl, 110
	jle	.L5975
	cmp	dl, 122
.L9275:
	je	.L5977
	jmp	.L999
.L5970:
	mov	DWORD [edi], 1
	mov	DWORD [edi+4], 243
	jmp	.L8701
.L5860:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	jg	.L5984
	cmp	dl, 69
	jg	.L5985
	test	dl, dl
	jmp	.L9241
.L5985:
	cmp	dl, 70
	jle	.L5989
	cmp	dl, 77
	jle	.L999
	jmp	.L5991
.L5984:
	cmp	dl, 102
	jg	.L5993
	cmp	dl, 101
	jle	.L999
	jmp	.L5989
.L5993:
	cmp	dl, 110
	je	.L5991
	jmp	.L999
.L5989:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], retnf_insn
	mov	DWORD [edi+4], 51714
	jmp	.L8695
.L5991:
	inc	ecx
	cmp	BYTE [ecx], 0
.L9241:
	jg	.L999
	mov	DWORD [edi], retnf_insn
	mov	DWORD [edi+4], 49666
	jmp	.L8695
.L5972:
.L5975:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	jg	.L6006
	cmp	dl, 69
	je	.L6010
	cmp	dl, 89
	jmp	.L9274
.L6006:
	cmp	dl, 101
	jg	.L6012
	cmp	dl, 100
.L9274:
	jle	.L1005
	jmp	.L6010
.L6012:
	cmp	dl, 122
	je	.L6010
	jmp	.L1005
.L5977:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 1
	mov	DWORD [edi+4], 244
	jmp	.L8701
.L6008:
.L6010:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 1
	mov	DWORD [edi+4], 242
	jmp	.L8701
.L5853:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 82
	jmp	.L8699
.L5838:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 87
	jmp	.L8699
.L5850:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L6024
	cmp	dl, 115
	je	.L6024
	jmp	.L1005
.L5841:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L6027
	cmp	dl, 115
	je	.L6027
	jmp	.L1005
.L5844:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L6030
	cmp	dl, 109
	je	.L6030
	jmp	.L1005
.L5847:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 72
	je	.L6033
	cmp	dl, 104
	jne	.L1005
.L6033:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L6036
	cmp	dl, 114
	jne	.L1005
.L6036:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 996865
.L8712:
	mov	DWORD [edi+8], 655392
	jmp	.L8696
.L6030:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L6042
	cmp	dl, 99
	jne	.L1005
.L6042:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 996097
	jmp	.L9010
.L6027:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L6048
	cmp	dl, 114
	jne	.L1005
.L6048:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 995841
.L8713:
	mov	DWORD [edi+8], 8388624
	jmp	.L8696
.L6024:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L6054
	cmp	dl, 99
	jne	.L1005
.L6054:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 995585
.L8963:
	mov	DWORD [edi+8], 16
	jmp	.L8696
.L5825:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 81
	jmp	.L8699
.L5818:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], shift_insn
	jmp	.L9218
.L5823:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], shift_insn
	mov	DWORD [edi+4], 776
	jmp	.L8695
.L5820:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6067
	cmp	dl, 80
	je	.L6069
	cmp	dl, 82
	jle	.L1005
	jmp	.L6071
.L6067:
	cmp	dl, 112
	jg	.L6073
	cmp	dl, 111
	jle	.L1005
	jmp	.L6069
.L6073:
	cmp	dl, 115
	je	.L6071
	jmp	.L1005
.L6069:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L6078
	cmp	dl, 115
	je	.L6078
	jmp	.L1005
.L6071:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L6081
	cmp	dl, 115
	jne	.L1005
.L6081:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15946497
	jmp	.L9152
.L6078:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 21249
	jmp	.L9152
.L5809:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 83
	jmp	.L8699
.L5807:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 85
	jmp	.L8699
.L5803:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 80
	jmp	.L8699
.L5792:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	movsx	eax, BYTE [ebx+1]
	sub	eax, 48
	jmp	.L9260
.L5794:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	movsx	eax, BYTE [ebx+1]
	sub	eax, 48
	jmp	.L9259
.L5789:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	movsx	eax, BYTE [ebx+1]
	sub	eax, 48
	jmp	.L9258
.L5783:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L6101
	cmp	dl, 66
	jg	.L6102
	test	dl, dl
	jle	.L6104
	cmp	dl, 65
	jle	.L999
	jmp	.L6106
.L6102:
	cmp	dl, 68
	je	.L6109
	cmp	dl, 86
	jle	.L999
	jmp	.L6111
.L6101:
	cmp	dl, 99
	jg	.L6113
	cmp	dl, 98
	je	.L6106
	jmp	.L999
.L6113:
	cmp	dl, 100
	jle	.L6109
	cmp	dl, 119
	je	.L6111
	jmp	.L999
.L6104:
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	movsx	eax, BYTE [ebx+2]
.L9269:
	sub	eax, 38
.L9261:
	or	eax, 80
	jmp	.L9201
.L6106:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	movsx	eax, BYTE [ebx+2]
	sub	eax, 38
.L9258:
	or	eax, 16
	jmp	.L9201
.L6109:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	movsx	eax, BYTE [ebx+2]
	sub	eax, 38
.L9260:
	or	eax, 64
	jmp	.L9201
.L6111:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	movsx	eax, BYTE [ebx+2]
	sub	eax, 38
.L9259:
	or	eax, 48
	jmp	.L9201
.L1195:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L6125
	cmp	dl, 72
	je	.L6127
	cmp	dl, 81
	jle	.L1005
	jmp	.L6129
.L6125:
	cmp	dl, 104
	jg	.L6131
	cmp	dl, 103
	jle	.L1005
	jmp	.L6127
.L6131:
	cmp	dl, 114
	je	.L6129
	jmp	.L1005
.L1198:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6135
	cmp	dl, 77
	je	.L6137
	cmp	dl, 82
	jle	.L1005
	jmp	.L6139
.L6135:
	cmp	dl, 109
	jg	.L6141
	cmp	dl, 108
	jle	.L1005
	jmp	.L6137
.L6141:
	cmp	dl, 115
	je	.L6139
	jmp	.L1005
.L1201:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6145
	cmp	dl, 65
	je	.L6147
	cmp	dl, 82
	jle	.L1005
	jmp	.L6149
.L6145:
	cmp	dl, 97
	jg	.L6151
	cmp	dl, 96
	jle	.L1005
	jmp	.L6147
.L6151:
	cmp	dl, 115
	je	.L6149
	jmp	.L1005
.L1204:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6155
	cmp	dl, 69
	je	.L6157
	cmp	dl, 82
	jle	.L1005
	jmp	.L6159
.L6155:
	cmp	dl, 101
	jg	.L6161
	cmp	dl, 100
	jle	.L1005
	jmp	.L6157
.L6161:
	cmp	dl, 115
	je	.L6159
	jmp	.L1005
.L1207:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6165
	cmp	dl, 68
	je	.L6167
	cmp	dl, 82
	jle	.L1005
	jmp	.L6169
.L6165:
	cmp	dl, 100
	jg	.L6171
	cmp	dl, 99
	jle	.L1005
	jmp	.L6167
.L6171:
	cmp	dl, 115
	je	.L6169
	jmp	.L1005
.L1210:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L6176
	cmp	dl, 100
	je	.L6176
	jmp	.L1005
.L1213:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L6179
	cmp	dl, 100
	je	.L6179
	jmp	.L1005
.L1216:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L6182
	cmp	dl, 115
	je	.L6182
	jmp	.L1005
.L1219:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	jg	.L6184
	cmp	dl, 66
	jg	.L6185
	cmp	dl, 65
	je	.L6187
	jmp	.L1005
.L6185:
	cmp	dl, 67
	jle	.L6190
	cmp	dl, 68
	jle	.L6192
	cmp	dl, 78
	jle	.L1005
	jmp	.L6194
.L6184:
	cmp	dl, 99
	jg	.L6196
	cmp	dl, 97
	je	.L6187
	cmp	dl, 98
	jle	.L1005
	jmp	.L6190
.L6196:
	cmp	dl, 100
	jle	.L6192
	cmp	dl, 111
	je	.L6194
	jmp	.L1005
.L1222:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6202
	cmp	dl, 76
	je	.L6204
	cmp	dl, 82
	jle	.L1005
	jmp	.L6206
.L6202:
	cmp	dl, 108
	jg	.L6208
	cmp	dl, 107
	jle	.L1005
	jmp	.L6204
.L6208:
	cmp	dl, 115
	je	.L6206
	jmp	.L1005
.L1225:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L6213
	cmp	dl, 114
	jne	.L1005
.L6213:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], prot286_insn
	mov	DWORD [edi+4], 196609
	jmp	.L9013
.L6206:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], lfgss_insn
	mov	DWORD [edi+4], 46594
	jmp	.L9186
.L6204:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], bsfr_insn
	mov	DWORD [edi+4], 771
	jmp	.L9031
.L6190:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 75
	je	.L6225
	cmp	dl, 107
	je	.L6225
	jmp	.L1005
.L6194:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L6228
	cmp	dl, 112
	je	.L6228
	jmp	.L1005
.L6192:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L6231
	cmp	dl, 115
	je	.L6231
	jmp	.L1005
.L6187:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L6234
	cmp	dl, 100
	jne	.L1005
.L6234:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L6237
	cmp	dl, 97
	jne	.L1005
.L6237:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L6240
	cmp	dl, 108
	jne	.L1005
.L6240:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L6243
	cmp	dl, 108
	jne	.L1005
.L6243:
	inc	ecx
	mov	dl, BYTE [ecx]
	test	dl, dl
	jle	.L6246
	cmp	dl, 50
	je	.L6248
	jmp	.L999
.L6246:
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 984833
	jmp	.L9016
.L6248:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 56
	jne	.L1005
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 54
	jne	.L1005
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 984321
.L8709:
	mov	DWORD [edi+8], 2097154
	jmp	.L8696
.L6231:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L6256
	cmp	dl, 68
	jg	.L6257
	cmp	dl, 66
	je	.L6259
	cmp	dl, 67
	jmp	.L8685
.L6257:
	cmp	dl, 81
	je	.L6264
	cmp	dl, 86
	jle	.L1005
	jmp	.L6266
.L6256:
	cmp	dl, 100
	jg	.L6268
	cmp	dl, 98
	je	.L6259
	cmp	dl, 99
.L8685:
	jle	.L1005
	jmp	.L6261
.L6268:
	cmp	dl, 113
	jg	.L6272
	cmp	dl, 112
	jle	.L1005
	jmp	.L6264
.L6272:
	cmp	dl, 119
	je	.L6266
	jmp	.L1005
.L6259:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 44033
	jmp	.L8695
.L6266:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1092865
	jmp	.L8695
.L6261:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2141441
	jmp	.L9186
.L6264:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 4238593
	jmp	.L9148
.L6228:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	jg	.L6289
	cmp	dl, 69
	jg	.L6290
	test	dl, dl
	jle	.L6292
	cmp	dl, 68
	jmp	.L9270
.L6290:
	cmp	dl, 78
	je	.L6297
	cmp	dl, 89
.L9270:
	jle	.L999
	jmp	.L6294
.L6289:
	cmp	dl, 109
	jg	.L6301
	cmp	dl, 101
	jmp	.L9272
.L6301:
	cmp	dl, 110
	jle	.L6297
	cmp	dl, 122
.L9272:
	je	.L6294
	jmp	.L999
.L6292:
	mov	DWORD [edi], loop_insn
.L9218:
	mov	DWORD [edi+4], 520
	jmp	.L8695
.L6299:
.L6294:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], loop_insn
.L9220:
	mov	DWORD [edi+4], 264
	jmp	.L8695
.L6297:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 90
	jg	.L6314
	cmp	dl, 69
	je	.L6318
	cmp	dl, 89
	jmp	.L9271
.L6314:
	cmp	dl, 101
	jg	.L6320
	cmp	dl, 100
.L9271:
	jle	.L1005
	jmp	.L6318
.L6320:
	cmp	dl, 122
	jne	.L1005
.L6316:
.L6318:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], loop_insn
.L9221:
	mov	DWORD [edi+4], 8
	jmp	.L8695
.L6225:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 1
	mov	DWORD [edi+4], 240
	jmp	.L8701
.L6182:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L6332
	cmp	dl, 119
	jne	.L1005
.L6332:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], prot286_insn
	mov	DWORD [edi+4], 393473
	jmp	.L9154
.L6179:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L6338
	cmp	dl, 116
	jne	.L1005
.L6338:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], prot286_insn
	mov	DWORD [edi+4], 131073
.L9013:
	mov	DWORD [edi+8], 9437186
	jmp	.L8696
.L6176:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L6344
	cmp	dl, 116
	jne	.L1005
.L6344:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 51314945
	jmp	.L9154
.L6169:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], lfgss_insn
	mov	DWORD [edi+4], 46338
	jmp	.L9186
.L6167:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L6353
	cmp	dl, 116
	jne	.L1005
.L6353:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 34537729
	jmp	.L9154
.L6159:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], lfgss_insn
	mov	DWORD [edi+4], 46082
	jmp	.L9186
.L6157:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L6362
	cmp	dl, 110
	jne	.L1005
.L6362:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L6365
	cmp	dl, 99
	jne	.L1005
.L6365:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L6368
	cmp	dl, 101
	jne	.L1005
.L6368:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], threebyte_insn
	mov	DWORD [edi+4], 263120897
	jmp	.L9024
.L6147:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	jg	.L6373
	test	dl, dl
	jle	.L6375
	cmp	dl, 85
	jle	.L999
	jmp	.L6377
.L6373:
	cmp	dl, 118
	je	.L6377
	jmp	.L999
.L6375:
	mov	DWORD [edi], lea_insn
	mov	DWORD [edi+4], 3
	jmp	.L8695
.L6149:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], ldes_insn
	mov	DWORD [edi+4], 50178
	jmp	.L8695
.L6377:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L6389
	cmp	dl, 101
	jne	.L1005
.L6389:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 51457
.L8857:
	mov	DWORD [edi+8], 1
	jmp	.L8696
.L6139:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], ldes_insn
	mov	DWORD [edi+4], 50434
	jmp	.L8695
.L6137:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L6401
	cmp	dl, 120
	jne	.L1005
.L6401:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L6404
	cmp	dl, 99
	jne	.L1005
.L6404:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L6407
	cmp	dl, 115
	jne	.L1005
.L6407:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L6410
	cmp	dl, 114
	jne	.L1005
.L6410:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ldstmxcsr_insn
.L9263:
	mov	DWORD [edi+4], 513
	jmp	.L9152
.L6127:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 70
	je	.L6416
	cmp	dl, 102
	je	.L6416
	jmp	.L1005
.L6129:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], bsfr_insn
	mov	DWORD [edi+4], 515
	jmp	.L9031
.L6416:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 40705
	jmp	.L8695
.L1164:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 54
	je	.L6428
	jmp	.L1005
.L1166:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	je	.L6430
	jmp	.L1005
.L1168:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 52
	je	.L6432
	jmp	.L1005
.L1171:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6433
	cmp	dl, 68
	jg	.L6434
	cmp	dl, 65
	je	.L6436
	cmp	dl, 67
	jmp	.L8686
.L6434:
	cmp	dl, 77
	je	.L6441
	cmp	dl, 82
	jle	.L1005
	jmp	.L6443
.L6433:
	cmp	dl, 100
	jg	.L6445
	cmp	dl, 97
	je	.L6436
	cmp	dl, 99
.L8686:
	jle	.L1005
	jmp	.L6438
.L6445:
	cmp	dl, 109
	jg	.L6449
	cmp	dl, 108
	jle	.L1005
	jmp	.L6441
.L6449:
	cmp	dl, 115
	je	.L6443
	jmp	.L1005
.L1174:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	jg	.L6453
	cmp	dl, 66
	jle	.L1005
	cmp	dl, 67
	jle	.L6456
	jmp	.L6457
.L6453:
	cmp	dl, 98
	jle	.L1005
	cmp	dl, 99
	jle	.L6456
	cmp	dl, 100
	jle	.L6457
	jmp	.L1005
.L1177:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 20
	jmp	.L8699
.L1180:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 16
	jmp	.L8699
.L1183:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L6465
	cmp	dl, 100
	je	.L6465
	jmp	.L1005
.L1186:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L6468
	cmp	dl, 112
	je	.L6468
	jmp	.L1005
.L1189:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 48
	jmp	.L8699
.L6468:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L6472
	cmp	dl, 108
	jne	.L1005
.L6472:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], arpl_insn
	mov	DWORD [edi+4], 1
	jmp	.L9031
.L6465:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L6480
	cmp	dl, 77
	jg	.L6481
	test	dl, dl
	jg	.L999
	jmp	.L6488
.L6481:
	cmp	dl, 78
	jle	.L6485
	cmp	dl, 79
	jle	.L999
	jmp	.L6487
.L6480:
	cmp	dl, 110
	jg	.L6489
	cmp	dl, 109
	jle	.L999
	jmp	.L6485
.L6489:
	cmp	dl, 112
	je	.L6487
	jmp	.L999
.L6488:
	mov	DWORD [edi], arith_insn
	mov	DWORD [edi+4], 270359
	jmp	.L8695
.L6485:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L6496
	cmp	dl, 112
	je	.L6496
	jmp	.L1005
.L6487:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6498
	cmp	dl, 68
	je	.L6500
	cmp	dl, 82
	jle	.L1005
	jmp	.L6502
.L6498:
	cmp	dl, 100
	jg	.L6503
	cmp	dl, 99
	jle	.L1005
	jmp	.L6500
.L6503:
	cmp	dl, 115
	jne	.L1005
.L6502:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 21505
	jmp	.L9152
.L6500:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6706177
	jmp	.L9151
.L6496:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6513
	cmp	dl, 68
	je	.L6515
	cmp	dl, 82
	jle	.L1005
	jmp	.L6517
.L6513:
	cmp	dl, 100
	jg	.L6519
	cmp	dl, 99
	jle	.L1005
	jmp	.L6515
.L6519:
	cmp	dl, 115
	je	.L6517
	jmp	.L1005
.L6515:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6706433
	jmp	.L9151
.L6517:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 21761
	jmp	.L9152
.L6457:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6529
	cmp	dl, 79
	jg	.L6530
	test	dl, dl
	jg	.L999
	jmp	.L6537
.L6530:
	cmp	dl, 80
	jle	.L6534
	cmp	dl, 82
	jle	.L999
	jmp	.L6536
.L6529:
	cmp	dl, 112
	jg	.L6538
	cmp	dl, 111
	jle	.L999
	jmp	.L6534
.L6538:
	cmp	dl, 115
	je	.L6536
	jmp	.L999
.L6537:
	mov	DWORD [edi], arith_insn
	mov	DWORD [edi+4], 23
	jmp	.L8695
.L6456:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], arith_insn
	mov	DWORD [edi+4], 135191
	jmp	.L8695
.L6534:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6547
	cmp	dl, 68
	je	.L6549
	cmp	dl, 82
	jle	.L1005
	jmp	.L6551
.L6547:
	cmp	dl, 100
	jg	.L6553
	cmp	dl, 99
	jle	.L1005
	jmp	.L6549
.L6553:
	cmp	dl, 115
	je	.L6551
	jmp	.L1005
.L6536:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6557
	cmp	dl, 68
	je	.L6559
	cmp	dl, 82
	jle	.L1005
	jmp	.L6561
.L6557:
	cmp	dl, 100
	jg	.L6562
	cmp	dl, 99
	jle	.L1005
	jmp	.L6559
.L6562:
	cmp	dl, 115
	jne	.L1005
.L6561:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15947777
	jmp	.L9152
.L6559:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15882241
	jmp	.L9151
.L6551:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 22529
	jmp	.L9152
.L6549:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6707201
	jmp	.L9151
.L6436:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 14081
	jmp	.L8695
.L6443:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 16129
	jmp	.L8695
.L6438:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], aadm_insn
	mov	DWORD [edi+4], 258
	jmp	.L8695
.L6441:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], aadm_insn
.L9223:
	mov	DWORD [edi+4], 2
	jmp	.L8695
.L6432:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9267
	mov	DWORD [edi], 2
	jmp	.L9244
.L6430:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 2
	jmp	.L9245
.L6428:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L6606
	sub	esp, 8
	push	DWORD LC18
	push	esi
	call	yasm__error
	jmp	.L8702
.L6606:
	mov	DWORD [edi], 2
	jmp	.L9246
.L1147:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 54
	je	.L6608
	jmp	.L1005
.L1150:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	je	.L6610
	jmp	.L1005
.L1152:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 52
	je	.L6612
	jmp	.L1005
.L1156:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L6613
	test	dl, dl
	jle	.L6615
	cmp	dl, 79
	jle	.L999
	jmp	.L6617
.L6613:
	cmp	dl, 112
	je	.L6617
	jmp	.L999
.L6615:
	mov	DWORD [edi], arith_insn
	mov	DWORD [edi+4], 67607
	jmp	.L8695
.L1158:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L6623
	cmp	dl, 116
	jne	.L1005
.L6623:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6625
	test	dl, dl
	jle	.L6627
	cmp	dl, 82
	jle	.L999
	jmp	.L6629
.L6625:
	cmp	dl, 115
	je	.L6629
	jmp	.L999
.L6627:
	mov	DWORD [edi], out_insn
.L9225:
	mov	DWORD [edi+4], 6
	jmp	.L8695
.L6629:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L6634
	cmp	dl, 67
	jg	.L6635
	cmp	dl, 66
	jmp	.L8687
.L6635:
	cmp	dl, 68
	jle	.L6639
	cmp	dl, 86
	jle	.L1005
	jmp	.L6641
.L6634:
	cmp	dl, 99
	jg	.L6643
	cmp	dl, 98
.L8687:
	jne	.L1005
	jmp	.L6642
.L6643:
	cmp	dl, 100
	jle	.L6639
	cmp	dl, 119
	je	.L6641
	jmp	.L1005
.L6642:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 28161
	jmp	.L8695
.L6641:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1076993
	jmp	.L8695
.L6639:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2125569
	jmp	.L9186
.L6617:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6657
	cmp	dl, 68
	je	.L6659
	cmp	dl, 82
	jle	.L1005
	jmp	.L6661
.L6657:
	cmp	dl, 100
	jg	.L6663
	cmp	dl, 99
	jle	.L1005
	jmp	.L6659
.L6663:
	cmp	dl, 115
	je	.L6661
	jmp	.L1005
.L6659:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6706689
	jmp	.L9151
.L6661:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 22017
	jmp	.L9152
.L6612:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L6674
.L9267:
	push	ebx
	push	DWORD LC17
	jmp	.L8698
.L6674:
	mov	DWORD [edi], 3
.L9244:
	mov	DWORD [edi+4], 64
	jmp	.L8701
.L6610:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 3
.L9245:
	mov	DWORD [edi+4], 32
	jmp	.L8701
.L6608:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 3
.L9246:
	mov	DWORD [edi+4], 16
.L8701:
	mov	eax, 2
	jmp	.L926
.L1132:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L6678
	cmp	dl, 115
	je	.L6678
	jmp	.L1005
.L1135:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 4
	jmp	.L8697
.L1137:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 47
	jle	.L1005
	cmp	dl, 55
	jg	.L1005
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	movsx	eax, BYTE [ebx+2]
	sub	eax, 48
	or	eax, 176
	jmp	.L9201
.L6678:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L6685
	cmp	dl, 116
	jne	.L1005
.L6685:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], test_insn
.L9222:
	mov	DWORD [edi+4], 20
	jmp	.L8695
.L1072:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L6691
	cmp	dl, 120
	je	.L6691
	jmp	.L1005
.L1075:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L6693
	cmp	dl, 67
	jg	.L6694
	cmp	dl, 66
	jmp	.L8660
.L6694:
	cmp	dl, 68
	jle	.L6699
	cmp	dl, 81
	jle	.L1005
	jmp	.L6701
.L6693:
	cmp	dl, 99
	jg	.L6703
	cmp	dl, 98
.L8660:
	je	.L6696
	jmp	.L1005
.L6703:
	cmp	dl, 100
	jle	.L6699
	cmp	dl, 114
	je	.L6701
	jmp	.L1005
.L1078:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6708
	cmp	dl, 76
	je	.L6710
	cmp	dl, 82
	jle	.L1005
	jmp	.L6712
.L6708:
	cmp	dl, 108
	jg	.L6714
	cmp	dl, 107
	jle	.L1005
	jmp	.L6710
.L6714:
	cmp	dl, 115
	je	.L6712
	jmp	.L1005
.L1081:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	jg	.L6718
	cmp	dl, 75
	jg	.L6719
	cmp	dl, 72
	je	.L6721
	jmp	.L1005
.L6719:
	cmp	dl, 76
	jle	.L6724
	cmp	dl, 77
	jle	.L6726
	cmp	dl, 78
	jle	.L1005
	jmp	.L6728
.L6718:
	cmp	dl, 108
	jg	.L6730
	cmp	dl, 104
	je	.L6721
	cmp	dl, 107
	jle	.L1005
	jmp	.L6724
.L6730:
	cmp	dl, 109
	jle	.L6726
	cmp	dl, 111
	je	.L6728
	jmp	.L1005
.L1084:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	jg	.L6736
	cmp	dl, 69
	je	.L6738
	cmp	dl, 72
	jle	.L1005
	jmp	.L6740
.L6736:
	cmp	dl, 101
	jg	.L6742
	cmp	dl, 100
	jle	.L1005
	jmp	.L6738
.L6742:
	cmp	dl, 105
	je	.L6740
	jmp	.L1005
.L1087:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L6747
	cmp	dl, 109
	je	.L6747
	jmp	.L1005
.L1090:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L6750
	cmp	dl, 114
	je	.L6750
	jmp	.L1005
.L1093:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 50
	ja	.L1005
	jmp	DWORD [.L6775+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L6775:
	dd	.L6755
	dd	.L1005
	dd	.L6758
	dd	.L6761
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L6764
	dd	.L6767
	dd	.L6770
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L6773
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L6755
	dd	.L1005
	dd	.L6758
	dd	.L6761
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L6764
	dd	.L6767
	dd	.L6770
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L6773
	[section .text]
.L1096:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L6777
	cmp	dl, 100
	je	.L6777
	jmp	.L1005
.L1099:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	je	.L6780
	cmp	dl, 117
	je	.L6780
	jmp	.L1005
.L1102:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6782
	cmp	dl, 73
	jg	.L6783
	cmp	dl, 67
	je	.L6785
	cmp	dl, 72
	jmp	.L8689
.L6783:
	cmp	dl, 79
	je	.L6790
	cmp	dl, 82
	jle	.L1005
	jmp	.L6792
.L6782:
	cmp	dl, 105
	jg	.L6794
	cmp	dl, 99
	je	.L6785
	cmp	dl, 104
.L8689:
	jle	.L1005
	jmp	.L6787
.L6794:
	cmp	dl, 111
	jg	.L6798
	cmp	dl, 110
	jle	.L1005
	jmp	.L6790
.L6798:
	cmp	dl, 115
	je	.L6792
	jmp	.L1005
.L1105:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L6802
	cmp	dl, 81
	jg	.L6803
	cmp	dl, 65
	jmp	.L8659
.L6803:
	cmp	dl, 82
	jle	.L6808
	cmp	dl, 83
	jle	.L1005
	jmp	.L6810
.L6802:
	cmp	dl, 113
	jg	.L6812
	cmp	dl, 97
.L8659:
	je	.L6805
	jmp	.L1005
.L6812:
	cmp	dl, 114
	jle	.L6808
	cmp	dl, 116
	je	.L6810
	jmp	.L1005
.L1108:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L6817
	cmp	dl, 78
	je	.L6819
	cmp	dl, 82
	jle	.L1005
	jmp	.L6821
.L6817:
	cmp	dl, 110
	jg	.L6823
	cmp	dl, 109
	jle	.L1005
	jmp	.L6819
.L6823:
	cmp	dl, 115
	je	.L6821
	jmp	.L1005
.L1111:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	jg	.L6827
	cmp	dl, 69
	jg	.L6828
	cmp	dl, 65
	jg	.L6829
	test	dl, dl
	jle	.L6831
	cmp	dl, 64
	jle	.L999
	jmp	.L6833
.L6829:
	cmp	dl, 67
	je	.L6836
	cmp	dl, 68
	jle	.L999
	jmp	.L6838
.L6828:
	cmp	dl, 80
	jg	.L6840
	cmp	dl, 73
	je	.L6842
	jmp	.L999
.L6840:
	cmp	dl, 81
	jle	.L6845
	cmp	dl, 83
	jle	.L999
	cmp	dl, 84
	jle	.L6848
	jmp	.L6849
.L6827:
	cmp	dl, 104
	jg	.L6851
	cmp	dl, 98
	jg	.L6852
	cmp	dl, 97
	je	.L6833
	jmp	.L999
.L6852:
	cmp	dl, 99
	jle	.L6836
	cmp	dl, 101
	je	.L6838
	jmp	.L999
.L6851:
	cmp	dl, 113
	jg	.L6858
	cmp	dl, 105
	jle	.L6842
	cmp	dl, 112
	jle	.L999
	jmp	.L6845
.L6858:
	cmp	dl, 115
	jle	.L999
	cmp	dl, 116
	jle	.L6848
	cmp	dl, 117
	jle	.L6849
	jmp	.L999
.L6831:
	mov	DWORD [edi], 25604
	jmp	.L8700
.L1114:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L6866
	cmp	dl, 115
	je	.L6866
	jmp	.L1005
.L1117:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L6869
	cmp	dl, 99
	je	.L6869
	jmp	.L1005
.L1120:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L6872
	cmp	dl, 97
	je	.L6872
	jmp	.L1005
.L1123:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L6874
	cmp	dl, 67
	jg	.L6875
	cmp	dl, 65
	je	.L6877
	cmp	dl, 66
	jmp	.L8688
.L6875:
	cmp	dl, 81
	jle	.L1005
	cmp	dl, 82
	jle	.L6883
	cmp	dl, 83
	jle	.L6885
	jmp	.L6886
.L6874:
	cmp	dl, 99
	jg	.L6888
	cmp	dl, 97
	je	.L6877
	cmp	dl, 98
.L8688:
	jle	.L1005
	jmp	.L6879
.L6888:
	cmp	dl, 114
	jg	.L6892
	cmp	dl, 113
	jle	.L1005
	jmp	.L6883
.L6892:
	cmp	dl, 115
	jle	.L6885
	cmp	dl, 116
	jle	.L6886
	jmp	.L1005
.L1126:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L6898
	cmp	dl, 108
	jne	.L1005
.L6898:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	jne	.L1005
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L6902
	cmp	dl, 120
	jne	.L1005
.L6902:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L6904
	test	dl, dl
	jle	.L6906
	cmp	dl, 79
	jle	.L999
	jmp	.L6908
.L6904:
	cmp	dl, 112
	je	.L6908
	jmp	.L999
.L6906:
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14283009
	jmp	.L9136
.L6908:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 49
	jne	.L1005
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14285057
	jmp	.L9136
.L6879:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 72
	je	.L6918
	cmp	dl, 104
	je	.L6918
	jmp	.L1005
.L6877:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L6921
	cmp	dl, 109
	je	.L6921
	jmp	.L1005
.L6886:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L6924
	cmp	dl, 114
	je	.L6924
	jmp	.L1005
.L6885:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L6927
	cmp	dl, 97
	je	.L6927
	jmp	.L1005
.L6883:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L6930
	cmp	dl, 115
	jne	.L1005
.L6930:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L6933
	cmp	dl, 116
	jne	.L1005
.L6933:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	je	.L6936
	cmp	dl, 111
	jne	.L1005
.L6936:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L6939
	cmp	dl, 114
	jne	.L1005
.L6939:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 17804801
	jmp	.L9130
.L6927:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L6945
	cmp	dl, 118
	jne	.L1005
.L6945:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L6948
	cmp	dl, 101
	jne	.L1005
.L6948:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 1027585
	jmp	.L9130
.L6924:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L6954
	cmp	dl, 97
	jne	.L1005
.L6954:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L6957
	cmp	dl, 99
	jne	.L1005
.L6957:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L6960
	cmp	dl, 116
	jne	.L1005
.L6960:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14283777
	jmp	.L9136
.L6921:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14279937
	jmp	.L9136
.L6918:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fxch_insn
	mov	DWORD [edi+4], 4
	jmp	.L9136
.L6872:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L6972
	cmp	dl, 105
	jne	.L1005
.L6972:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L6975
	cmp	dl, 116
	jne	.L1005
.L6975:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 39681
	jmp	.L9136
.L6869:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	je	.L6981
	cmp	dl, 111
	jne	.L1005
.L6981:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L6984
	cmp	dl, 109
	jne	.L1005
.L6984:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L6986
	cmp	dl, 72
	jg	.L6987
	test	dl, dl
	jg	.L999
	jmp	.L6994
.L6987:
	cmp	dl, 73
	jle	.L6991
	cmp	dl, 79
	jle	.L999
	jmp	.L6993
.L6986:
	cmp	dl, 105
	jg	.L6995
	cmp	dl, 104
	jle	.L999
	jmp	.L6991
.L6995:
	cmp	dl, 112
	je	.L6993
	jmp	.L999
.L6994:
	mov	DWORD [edi], fcom2_insn
	mov	DWORD [edi+4], 14540802
	jmp	.L9078
.L6991:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7001
	test	dl, dl
	jle	.L7003
	cmp	dl, 79
	jle	.L999
	jmp	.L7005
.L7001:
	cmp	dl, 112
	je	.L7005
	jmp	.L999
.L7003:
	mov	DWORD [edi], fcom2_insn
	mov	DWORD [edi+4], 14411778
	jmp	.L9130
.L6993:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7010
	test	dl, dl
	jle	.L7012
	cmp	dl, 79
	jle	.L999
	jmp	.L7014
.L7010:
	cmp	dl, 112
	je	.L7014
	jmp	.L999
.L7012:
	mov	DWORD [edi], fcom2_insn
	mov	DWORD [edi+4], 14542850
	jmp	.L9078
.L7014:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14346497
	jmp	.L9078
.L7005:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fcom2_insn
	mov	DWORD [edi+4], 14673922
	jmp	.L9130
.L6866:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7026
	cmp	dl, 116
	jne	.L1005
.L7026:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14279681
	jmp	.L9136
.L6848:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L7031
	cmp	dl, 68
	jg	.L7032
	test	dl, dl
	jle	.L7034
	cmp	dl, 67
	je	.L7036
	jmp	.L999
.L7032:
	cmp	dl, 79
	jg	.L7038
	cmp	dl, 69
	jle	.L7040
	jmp	.L999
.L7038:
	cmp	dl, 80
	jle	.L7043
	cmp	dl, 82
	jle	.L999
	jmp	.L7045
.L7031:
	cmp	dl, 101
	jg	.L7047
	cmp	dl, 99
	je	.L7036
	cmp	dl, 100
	jle	.L999
	jmp	.L7040
.L7047:
	cmp	dl, 112
	jg	.L7051
	cmp	dl, 111
	jle	.L999
	jmp	.L7043
.L7051:
	cmp	dl, 115
	je	.L7045
	jmp	.L999
.L7034:
	mov	DWORD [edi], fst_insn
	mov	DWORD [edi+4], 3
	jmp	.L9136
.L6849:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L7058
	cmp	dl, 98
	je	.L7058
	jmp	.L1005
.L6845:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L7061
	cmp	dl, 114
	je	.L7061
	jmp	.L1005
.L6842:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L7064
	cmp	dl, 110
	je	.L7064
	jmp	.L1005
.L6836:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L7067
	cmp	dl, 97
	je	.L7067
	jmp	.L1005
.L6833:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L7070
	cmp	dl, 118
	je	.L7070
	jmp	.L1005
.L6838:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7073
	cmp	dl, 116
	jne	.L1005
.L7073:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L7076
	cmp	dl, 112
	jne	.L1005
.L7076:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L7079
	cmp	dl, 109
	jne	.L1005
.L7079:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14410753
	mov	DWORD [edi+8], 4198402
	jmp	.L8696
.L7070:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L7085
	cmp	dl, 101
	jne	.L1005
.L7085:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 110877953
	jmp	.L9136
.L7067:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L7091
	cmp	dl, 108
	jne	.L1005
.L7091:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L7094
	cmp	dl, 101
	jne	.L1005
.L7094:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14286081
	jmp	.L9136
.L7064:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	jg	.L7099
	test	dl, dl
	jle	.L7101
	cmp	dl, 66
	jle	.L999
	jmp	.L7103
.L7099:
	cmp	dl, 99
	je	.L7103
	jmp	.L999
.L7101:
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14286337
	jmp	.L9078
.L7103:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	je	.L7109
	cmp	dl, 111
	jne	.L1005
.L7109:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L7112
	cmp	dl, 115
	jne	.L1005
.L7112:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14285569
	jmp	.L9078
.L7061:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7118
	cmp	dl, 116
	jne	.L1005
.L7118:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14285313
	jmp	.L9136
.L7058:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L7123
	cmp	dl, 79
	jg	.L7124
	test	dl, dl
	jg	.L999
	jmp	.L7131
.L7124:
	cmp	dl, 80
	jle	.L7128
	cmp	dl, 81
	jle	.L999
	jmp	.L7130
.L7123:
	cmp	dl, 112
	jg	.L7132
	cmp	dl, 111
	jle	.L999
	jmp	.L7128
.L7132:
	cmp	dl, 114
	je	.L7130
	jmp	.L999
.L7131:
	mov	DWORD [edi], farith_insn
	mov	DWORD [edi+4], 81848326
	jmp	.L9136
.L7128:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], farithp_insn
	mov	DWORD [edi+4], 59395
	jmp	.L9136
.L7130:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7141
	test	dl, dl
	jle	.L7143
	cmp	dl, 79
	jle	.L999
	jmp	.L7145
.L7141:
	cmp	dl, 112
	je	.L7145
	jmp	.L999
.L7143:
	mov	DWORD [edi], farith_insn
	mov	DWORD [edi+4], 99147782
	jmp	.L9136
.L7145:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], farithp_insn
	mov	DWORD [edi+4], 57347
	jmp	.L9136
.L7036:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L7154
	cmp	dl, 119
	je	.L7154
	jmp	.L1005
.L7040:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L7157
	cmp	dl, 110
	je	.L7157
	jmp	.L1005
.L7043:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fldstp_insn
	mov	DWORD [edi+4], 117692420
	jmp	.L9136
.L7045:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L7163
	cmp	dl, 119
	jne	.L1005
.L7163:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fstsw_insn
	jmp	.L9247
.L7157:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L7169
	cmp	dl, 118
	jne	.L1005
.L7169:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 110876929
	jmp	.L9136
.L7154:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fstcw_insn
	mov	DWORD [edi+4], 1
	jmp	.L9136
.L6819:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L7178
	cmp	dl, 100
	je	.L7178
	jmp	.L1005
.L6821:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7181
	cmp	dl, 116
	jne	.L1005
.L7181:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	je	.L7184
	cmp	dl, 111
	jne	.L1005
.L7184:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L7187
	cmp	dl, 114
	jne	.L1005
.L7187:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebytemem_insn
	mov	DWORD [edi+4], 318721
	jmp	.L9136
.L7178:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L7193
	cmp	dl, 105
	jne	.L1005
.L7193:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L7196
	cmp	dl, 110
	jne	.L1005
.L7196:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7199
	cmp	dl, 116
	jne	.L1005
.L7199:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14285825
	jmp	.L9136
.L6810:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L7205
	cmp	dl, 97
	je	.L7205
	jmp	.L1005
.L6805:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7208
	cmp	dl, 116
	je	.L7208
	jmp	.L1005
.L6808:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L7211
	cmp	dl, 101
	jne	.L1005
.L7211:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L7214
	cmp	dl, 109
	jne	.L1005
.L7214:
	inc	ecx
	mov	dl, BYTE [ecx]
	test	dl, dl
	jle	.L7217
	cmp	dl, 49
	je	.L7219
	jmp	.L999
.L7217:
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14284801
	jmp	.L9136
.L7219:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14284033
	jmp	.L9078
.L7208:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L7226
	cmp	dl, 97
	jne	.L1005
.L7226:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L7229
	cmp	dl, 110
	jne	.L1005
.L7229:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14283521
	jmp	.L9136
.L7205:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L7235
	cmp	dl, 110
	jne	.L1005
.L7235:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14283265
	jmp	.L9136
.L6787:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L7241
	cmp	dl, 110
	je	.L7241
	jmp	.L1005
.L6792:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L7243
	cmp	dl, 65
	je	.L7245
	cmp	dl, 83
	jle	.L1005
	jmp	.L7247
.L7243:
	cmp	dl, 97
	jg	.L7249
	cmp	dl, 96
	jle	.L1005
	jmp	.L7245
.L7249:
	cmp	dl, 116
	je	.L7247
	jmp	.L1005
.L6785:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L7254
	cmp	dl, 108
	je	.L7254
	jmp	.L1005
.L6790:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L7257
	cmp	dl, 112
	jne	.L1005
.L7257:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14274561
	jmp	.L9136
.L7254:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L7263
	cmp	dl, 101
	jne	.L1005
.L7263:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L7266
	cmp	dl, 120
	jne	.L1005
.L7266:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14410241
	jmp	.L9136
.L7245:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L7272
	cmp	dl, 118
	je	.L7272
	jmp	.L1005
.L7247:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L7274
	cmp	dl, 68
	jg	.L7275
	cmp	dl, 67
	jmp	.L8690
.L7275:
	cmp	dl, 69
	jle	.L7279
	cmp	dl, 82
	jle	.L1005
	jmp	.L7281
.L7274:
	cmp	dl, 100
	jg	.L7283
	cmp	dl, 99
.L8690:
	jne	.L1005
	jmp	.L7282
.L7283:
	cmp	dl, 101
	jle	.L7279
	cmp	dl, 115
	je	.L7281
	jmp	.L1005
.L7282:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L7289
	cmp	dl, 119
	je	.L7289
	jmp	.L1005
.L7281:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L7292
	cmp	dl, 119
	je	.L7292
	jmp	.L1005
.L7279:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L7295
	cmp	dl, 110
	jne	.L1005
.L7295:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L7298
	cmp	dl, 118
	jne	.L1005
.L7298:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebytemem_insn
	mov	DWORD [edi+4], 448769
	jmp	.L9136
.L7292:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fnstsw_insn
.L9247:
	mov	DWORD [edi+4], 2
	jmp	.L9136
.L7289:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fldnstcw_insn
	mov	DWORD [edi+4], 1793
	jmp	.L9136
.L7272:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L7310
	cmp	dl, 101
	jne	.L1005
.L7310:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebytemem_insn
	mov	DWORD [edi+4], 449793
	jmp	.L9136
.L7241:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L7316
	cmp	dl, 105
	jne	.L1005
.L7316:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7319
	cmp	dl, 116
	jne	.L1005
.L7319:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14410497
	jmp	.L9136
.L6780:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L7325
	cmp	dl, 108
	jne	.L1005
.L7325:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7327
	test	dl, dl
	jle	.L7329
	cmp	dl, 79
	jle	.L999
	jmp	.L7331
.L7327:
	cmp	dl, 112
	je	.L7331
	jmp	.L999
.L7329:
	mov	DWORD [edi], farith_insn
	mov	DWORD [edi+4], 29935622
	jmp	.L9136
.L7331:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], farithp_insn
	mov	DWORD [edi+4], 51203
	jmp	.L9136
.L6777:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 89
	jg	.L7339
	cmp	dl, 68
	jg	.L7340
	cmp	dl, 49
	jg	.L7341
	test	dl, dl
	jle	.L7343
	cmp	dl, 48
	jle	.L999
	jmp	.L7345
.L7341:
	cmp	dl, 67
	je	.L7348
	jmp	.L999
.L7340:
	cmp	dl, 76
	jg	.L7350
	cmp	dl, 69
	jle	.L7352
	cmp	dl, 75
	jle	.L999
	jmp	.L7354
.L7350:
	cmp	dl, 80
	je	.L7357
	jmp	.L999
.L7339:
	cmp	dl, 107
	jg	.L7359
	cmp	dl, 99
	jg	.L7360
	cmp	dl, 90
	jle	.L7362
	cmp	dl, 98
	jle	.L999
	jmp	.L7348
.L7360:
	cmp	dl, 101
	je	.L7352
	jmp	.L999
.L7359:
	cmp	dl, 112
	jg	.L7367
	cmp	dl, 108
	jle	.L7354
	cmp	dl, 111
	jle	.L999
	jmp	.L7357
.L7367:
	cmp	dl, 122
	je	.L7362
	jmp	.L999
.L7343:
	mov	DWORD [edi], fldstp_insn
	mov	DWORD [edi+4], 83935236
	jmp	.L9136
.L7345:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14280705
	jmp	.L9136
.L7348:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L7378
	cmp	dl, 119
	je	.L7378
	jmp	.L1005
.L7352:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L7381
	cmp	dl, 110
	je	.L7381
	jmp	.L1005
.L7354:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	jg	.L7383
	cmp	dl, 50
	jg	.L7384
	cmp	dl, 49
	jle	.L1005
	jmp	.L7386
.L7384:
	cmp	dl, 71
	je	.L7389
	jmp	.L1005
.L7383:
	cmp	dl, 103
	jg	.L7391
	cmp	dl, 78
	jle	.L7393
	cmp	dl, 102
	jle	.L1005
	jmp	.L7389
.L7391:
	cmp	dl, 110
	je	.L7393
	jmp	.L1005
.L7357:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L7398
	cmp	dl, 105
	je	.L7398
	jmp	.L1005
.L7362:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14282241
	jmp	.L9136
.L7398:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14281473
	jmp	.L9136
.L7386:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L7406
	cmp	dl, 69
	je	.L7408
	cmp	dl, 83
	jle	.L1005
	jmp	.L7410
.L7406:
	cmp	dl, 101
	jg	.L7412
	cmp	dl, 100
	jle	.L1005
	jmp	.L7408
.L7412:
	cmp	dl, 116
	je	.L7410
	jmp	.L1005
.L7389:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	je	.L7417
	jmp	.L1005
.L7393:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 50
	jne	.L1005
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14281985
	jmp	.L9136
.L7417:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14281729
	jmp	.L9136
.L7408:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14281217
	jmp	.L9136
.L7410:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14280961
	jmp	.L9136
.L7381:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L7432
	cmp	dl, 118
	jne	.L1005
.L7432:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebytemem_insn
	mov	DWORD [edi+4], 317697
	jmp	.L9136
.L7378:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fldnstcw_insn
	mov	DWORD [edi+4], 1281
	jmp	.L9136
.L6764:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L7441
	cmp	dl, 100
	je	.L7441
	jmp	.L1005
.L6773:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	jg	.L7443
	cmp	dl, 83
	jle	.L1005
	cmp	dl, 84
	jle	.L7446
	jmp	.L7447
.L7443:
	cmp	dl, 115
	jle	.L1005
	cmp	dl, 116
	jle	.L7446
	cmp	dl, 117
	jle	.L7447
	jmp	.L1005
.L6758:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	je	.L7453
	cmp	dl, 111
	je	.L7453
	jmp	.L1005
.L6755:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L7456
	cmp	dl, 100
	je	.L7456
	jmp	.L1005
.L6767:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	je	.L7459
	cmp	dl, 117
	je	.L7459
	jmp	.L1005
.L6761:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L7462
	cmp	dl, 105
	je	.L7462
	jmp	.L1005
.L6770:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	jg	.L7464
	cmp	dl, 67
	je	.L7466
	cmp	dl, 72
	jle	.L1005
	jmp	.L7468
.L7464:
	cmp	dl, 99
	jg	.L7470
	cmp	dl, 98
	jle	.L1005
	jmp	.L7466
.L7470:
	cmp	dl, 105
	je	.L7468
	jmp	.L1005
.L7466:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L7475
	cmp	dl, 115
	je	.L7475
	jmp	.L1005
.L7468:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7478
	cmp	dl, 116
	jne	.L1005
.L7478:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], threebyte_insn
	mov	DWORD [edi+4], -1730419967
	jmp	.L9136
.L7475:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7484
	cmp	dl, 116
	jne	.L1005
.L7484:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L7487
	cmp	dl, 112
	jne	.L1005
.L7487:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14284545
	jmp	.L9136
.L7462:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L7493
	cmp	dl, 118
	jne	.L1005
.L7493:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L7495
	test	dl, dl
	jle	.L7497
	cmp	dl, 81
	jle	.L999
	jmp	.L7499
.L7495:
	cmp	dl, 114
	je	.L7499
	jmp	.L999
.L7497:
	mov	DWORD [edi], fiarith_insn
	mov	DWORD [edi+4], 449026
	jmp	.L9136
.L7499:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fiarith_insn
	mov	DWORD [edi+4], 514562
	jmp	.L9136
.L7459:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L7508
	cmp	dl, 108
	jne	.L1005
.L7508:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fiarith_insn
	mov	DWORD [edi+4], 121346
	jmp	.L9136
.L7456:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L7514
	cmp	dl, 100
	jne	.L1005
.L7514:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fiarith_insn
	mov	DWORD [edi+4], 55810
	jmp	.L9136
.L7453:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L7520
	cmp	dl, 109
	jne	.L1005
.L7520:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7522
	test	dl, dl
	jle	.L7524
	cmp	dl, 79
	jle	.L999
	jmp	.L7526
.L7522:
	cmp	dl, 112
	je	.L7526
	jmp	.L999
.L7524:
	mov	DWORD [edi], fiarith_insn
	mov	DWORD [edi+4], 186882
	jmp	.L9136
.L7526:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fiarith_insn
	mov	DWORD [edi+4], 252418
	jmp	.L9136
.L7446:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7534
	test	dl, dl
	jle	.L7536
	cmp	dl, 79
	jle	.L999
	jmp	.L7538
.L7534:
	cmp	dl, 112
	je	.L7538
	jmp	.L999
.L7536:
	mov	DWORD [edi], fiarith_insn
	mov	DWORD [edi+4], 187138
	jmp	.L9136
.L7447:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L7544
	cmp	dl, 98
	jne	.L1005
.L7544:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L7546
	test	dl, dl
	jle	.L7548
	cmp	dl, 81
	jle	.L999
	jmp	.L7550
.L7546:
	cmp	dl, 114
	je	.L7550
	jmp	.L999
.L7548:
	mov	DWORD [edi], fiarith_insn
	mov	DWORD [edi+4], 317954
	jmp	.L9136
.L7550:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fiarith_insn
	mov	DWORD [edi+4], 383490
	jmp	.L9136
.L7538:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fildstp_insn
	mov	DWORD [edi+4], 459523
	jmp	.L9136
.L7441:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fildstp_insn
	mov	DWORD [edi+4], 327683
	jmp	.L9136
.L6750:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L7565
	cmp	dl, 101
	jne	.L1005
.L7565:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L7568
	cmp	dl, 101
	jne	.L1005
.L7568:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7570
	test	dl, dl
	jle	.L7572
	cmp	dl, 79
	jle	.L999
	jmp	.L7574
.L7570:
	cmp	dl, 112
	je	.L7574
	jmp	.L999
.L7572:
	mov	DWORD [edi], ffree_insn
	mov	DWORD [edi+4], 56577
	jmp	.L9136
.L7574:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ffree_insn
	mov	DWORD [edi+4], 57089
	mov	DWORD [edi+8], 2101280
	jmp	.L8696
.L6747:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L7583
	cmp	dl, 109
	jne	.L1005
.L7583:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L7586
	cmp	dl, 115
	jne	.L1005
.L7586:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 986625
.L8826:
	mov	DWORD [edi+8], 65536
	jmp	.L8696
.L6740:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L7592
	cmp	dl, 118
	je	.L7592
	jmp	.L1005
.L6738:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L7595
	cmp	dl, 99
	jne	.L1005
.L7595:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L7598
	cmp	dl, 115
	jne	.L1005
.L7598:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7601
	cmp	dl, 116
	jne	.L1005
.L7601:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L7604
	cmp	dl, 112
	jne	.L1005
.L7604:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14284289
	jmp	.L9136
.L7592:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L7609
	cmp	dl, 79
	jg	.L7610
	test	dl, dl
	jg	.L999
	jmp	.L7617
.L7610:
	cmp	dl, 80
	jle	.L7614
	cmp	dl, 81
	jle	.L999
	jmp	.L7616
.L7609:
	cmp	dl, 112
	jg	.L7618
	cmp	dl, 111
	jle	.L999
	jmp	.L7614
.L7618:
	cmp	dl, 114
	je	.L7616
	jmp	.L999
.L7617:
	mov	DWORD [edi], farith_insn
	mov	DWORD [edi+4], 116455430
	jmp	.L9136
.L7614:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], farithp_insn
	mov	DWORD [edi+4], 63491
	jmp	.L9136
.L7616:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7627
	test	dl, dl
	jle	.L7629
	cmp	dl, 79
	jle	.L999
	jmp	.L7631
.L7627:
	cmp	dl, 112
	je	.L7631
	jmp	.L999
.L7629:
	mov	DWORD [edi], farith_insn
	mov	DWORD [edi+4], 133754886
	jmp	.L9136
.L7631:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], farithp_insn
	mov	DWORD [edi+4], 61443
	jmp	.L9136
.L6728:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L7639
	cmp	dl, 77
	je	.L7641
	cmp	dl, 82
	jle	.L1005
	jmp	.L7643
.L7639:
	cmp	dl, 109
	jg	.L7645
	cmp	dl, 108
	jle	.L1005
	jmp	.L7641
.L7645:
	cmp	dl, 115
	je	.L7643
	jmp	.L1005
.L6721:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L7650
	cmp	dl, 115
	je	.L7650
	jmp	.L1005
.L6724:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L7653
	cmp	dl, 101
	je	.L7653
	jmp	.L1005
.L6726:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	je	.L7656
	cmp	dl, 111
	jne	.L1005
.L7656:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 86
	je	.L7659
	cmp	dl, 118
	jne	.L1005
.L7659:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 85
	jg	.L7661
	cmp	dl, 69
	jg	.L7662
	cmp	dl, 66
	je	.L7664
	cmp	dl, 68
	jmp	.L8691
.L7662:
	cmp	dl, 78
	je	.L7669
	cmp	dl, 84
	jle	.L1005
	jmp	.L7671
.L7661:
	cmp	dl, 101
	jg	.L7673
	cmp	dl, 98
	je	.L7664
	cmp	dl, 100
.L8691:
	jle	.L1005
	jmp	.L7666
.L7673:
	cmp	dl, 110
	jg	.L7677
	cmp	dl, 109
	jle	.L1005
	jmp	.L7669
.L7677:
	cmp	dl, 117
	je	.L7671
	jmp	.L1005
.L7664:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L7681
	test	dl, dl
	jle	.L7683
	cmp	dl, 68
	jle	.L999
	jmp	.L7685
.L7681:
	cmp	dl, 101
	je	.L7685
	jmp	.L999
.L7683:
	mov	DWORD [edi], fcmovcc_insn
	mov	DWORD [edi+4], 14336001
	jmp	.L9130
.L7666:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fcmovcc_insn
	mov	DWORD [edi+4], 14338049
	jmp	.L9130
.L7669:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L7693
	cmp	dl, 66
	je	.L7695
	cmp	dl, 68
	jle	.L1005
	jmp	.L7697
.L7693:
	cmp	dl, 98
	jg	.L7699
	cmp	dl, 97
	jle	.L1005
	jmp	.L7695
.L7699:
	cmp	dl, 101
	je	.L7697
	jmp	.L1005
.L7671:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fcmovcc_insn
	mov	DWORD [edi+4], 14342145
	jmp	.L9130
.L7695:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L7706
	test	dl, dl
	jle	.L7708
	cmp	dl, 68
	jle	.L999
	jmp	.L7710
.L7706:
	cmp	dl, 101
	je	.L7710
	jmp	.L999
.L7708:
	mov	DWORD [edi], fcmovcc_insn
	mov	DWORD [edi+4], 14401537
	jmp	.L9130
.L7697:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fcmovcc_insn
	mov	DWORD [edi+4], 14403585
	jmp	.L9130
.L7710:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fcmovcc_insn
	mov	DWORD [edi+4], 14405633
	jmp	.L9130
.L7685:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fcmovcc_insn
	mov	DWORD [edi+4], 14340097
	jmp	.L9130
.L7653:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L7725
	cmp	dl, 120
	jne	.L1005
.L7725:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], threebyte_insn
	mov	DWORD [edi+4], -1730420223
	jmp	.L9136
.L7650:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14278657
	jmp	.L9136
.L7641:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7733
	cmp	dl, 72
	jg	.L7734
	test	dl, dl
	jg	.L999
	jmp	.L7741
.L7734:
	cmp	dl, 73
	jle	.L7738
	cmp	dl, 79
	jle	.L999
	jmp	.L7740
.L7733:
	cmp	dl, 105
	jg	.L7742
	cmp	dl, 104
	jle	.L999
	jmp	.L7738
.L7742:
	cmp	dl, 112
	je	.L7740
	jmp	.L999
.L7741:
	mov	DWORD [edi], fcom_insn
	mov	DWORD [edi+4], 184324
	jmp	.L9136
.L7643:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14286593
.L9078:
	mov	DWORD [edi+8], 4098
	jmp	.L8696
.L7740:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7751
	test	dl, dl
	jle	.L7753
	cmp	dl, 79
	jle	.L999
	jmp	.L7755
.L7751:
	cmp	dl, 112
	je	.L7755
	jmp	.L999
.L7753:
	mov	DWORD [edi], fcom_insn
	mov	DWORD [edi+4], 251908
	jmp	.L9136
.L7738:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7760
	test	dl, dl
	jle	.L7762
	cmp	dl, 79
	jle	.L999
	jmp	.L7764
.L7760:
	cmp	dl, 112
	je	.L7764
	jmp	.L999
.L7762:
	mov	DWORD [edi], fcom2_insn
	mov	DWORD [edi+4], 14413826
	jmp	.L9130
.L7764:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fcom2_insn
	mov	DWORD [edi+4], 14675970
.L9130:
	mov	DWORD [edi+8], 4128
	jmp	.L8696
.L7755:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14604545
	jmp	.L9136
.L6710:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L7776
	cmp	dl, 100
	je	.L7776
	jmp	.L1005
.L6712:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7779
	cmp	dl, 116
	jne	.L1005
.L7779:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L7782
	cmp	dl, 112
	jne	.L1005
.L7782:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fbldstp_insn
	mov	DWORD [edi+4], 1537
	jmp	.L9136
.L7776:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], fbldstp_insn
	mov	DWORD [edi+4], 1025
	jmp	.L9136
.L6701:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 3
	jmp	.L8697
.L6699:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L7792
	cmp	dl, 100
	je	.L7792
	jmp	.L1005
.L6696:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L7795
	cmp	dl, 115
	jne	.L1005
.L7795:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14278913
	jmp	.L9136
.L7792:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	jg	.L7800
	test	dl, dl
	jle	.L7802
	cmp	dl, 79
	jle	.L999
	jmp	.L7804
.L7800:
	cmp	dl, 112
	je	.L7804
	jmp	.L999
.L7802:
	mov	DWORD [edi], farith_insn
	mov	DWORD [edi+4], 12632070
	jmp	.L9136
.L7804:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], farithp_insn
	mov	DWORD [edi+4], 49155
	jmp	.L9136
.L6691:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 77
	je	.L7813
	cmp	dl, 109
	jne	.L1005
.L7813:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 49
	jne	.L1005
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 14282753
.L9136:
	mov	DWORD [edi+8], 4096
	jmp	.L8696
.L1016:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L7819
	cmp	dl, 75
	jg	.L7820
	cmp	dl, 72
	jmp	.L8658
.L7820:
	cmp	dl, 76
	jle	.L7825
	cmp	dl, 81
	jle	.L1005
	jmp	.L7827
.L7819:
	cmp	dl, 107
	jg	.L7829
	cmp	dl, 104
.L8658:
	je	.L7822
	jmp	.L1005
.L7829:
	cmp	dl, 108
	jle	.L7825
	cmp	dl, 114
	je	.L7827
	jmp	.L1005
.L1019:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L7835
	cmp	dl, 98
	je	.L7835
	jmp	.L1005
.L1022:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L7838
	cmp	dl, 97
	je	.L7838
	jmp	.L1005
.L1025:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7841
	cmp	dl, 116
	je	.L7841
	jmp	.L1005
.L1028:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L7844
	cmp	dl, 101
	je	.L7844
	jmp	.L1005
.L1031:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L7847
	cmp	dl, 100
	je	.L7847
	jmp	.L1005
.L1034:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 76
	cmp	eax, 41
	ja	.L1005
	jmp	DWORD [.L7863+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L7863:
	dd	.L7852
	dd	.L1005
	dd	.L1005
	dd	.L7855
	dd	.L1005
	dd	.L1005
	dd	.L7858
	dd	.L1005
	dd	.L1005
	dd	.L7861
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L7852
	dd	.L1005
	dd	.L1005
	dd	.L7855
	dd	.L1005
	dd	.L1005
	dd	.L7858
	dd	.L1005
	dd	.L1005
	dd	.L7861
	[section .text]
.L1037:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	jg	.L7864
	cmp	dl, 67
	jg	.L7865
	test	dl, dl
	jg	.L999
	jmp	.L7872
.L7865:
	cmp	dl, 68
	jle	.L7869
	cmp	dl, 75
	jle	.L999
	jmp	.L7871
.L7864:
	cmp	dl, 100
	jg	.L7873
	cmp	dl, 99
	jle	.L999
	jmp	.L7869
.L7873:
	cmp	dl, 108
	je	.L7871
	jmp	.L999
.L7872:
	mov	DWORD [edi], 54
	jmp	.L8699
.L1040:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L7878
	cmp	dl, 100
	je	.L7878
	jmp	.L1005
.L1043:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L7880
	cmp	dl, 73
	je	.L7882
	cmp	dl, 82
	jle	.L1005
	jmp	.L7884
.L7880:
	cmp	dl, 105
	jg	.L7886
	cmp	dl, 104
	jle	.L1005
	jmp	.L7882
.L7886:
	cmp	dl, 115
	je	.L7884
	jmp	.L1005
.L1046:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	jg	.L7890
	test	dl, dl
	jle	.L7892
	cmp	dl, 75
	jle	.L999
	jmp	.L7894
.L7890:
	cmp	dl, 108
	je	.L7894
	jmp	.L999
.L7892:
	mov	DWORD [edi], 52
	jmp	.L8699
.L1049:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L7898
	cmp	dl, 114
	je	.L7898
	jmp	.L1005
.L1052:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L7901
	push	ebx
	push	DWORD LC16
	push	esi
	push	DWORD 0
	call	yasm__warning
	add	esp, 16
.L7901:
	mov	DWORD [edi], 13826
.L8700:
	mov	eax, 4
	jmp	.L926
.L1055:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 48
	cmp	eax, 66
	ja	.L1005
	jmp	DWORD [.L7931+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L7931:
	dd	.L7911
	dd	.L7911
	dd	.L7911
	dd	.L7911
	dd	.L7911
	dd	.L7911
	dd	.L7911
	dd	.L7911
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L7914
	dd	.L7917
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L7920
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L7923
	dd	.L1005
	dd	.L7926
	dd	.L1005
	dd	.L1005
	dd	.L7929
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L7914
	dd	.L7917
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L7920
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L7923
	dd	.L1005
	dd	.L7926
	dd	.L1005
	dd	.L1005
	dd	.L7929
	[section .text]
.L1058:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 66
	je	.L7933
	cmp	dl, 98
	je	.L7933
	jmp	.L1005
.L1061:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L7935
	cmp	dl, 75
	jg	.L7936
	cmp	dl, 68
	jmp	.L8657
.L7936:
	cmp	dl, 76
	jle	.L7941
	cmp	dl, 83
	jle	.L1005
	jmp	.L7943
.L7935:
	cmp	dl, 107
	jg	.L7945
	cmp	dl, 100
.L8657:
	je	.L7938
	jmp	.L1005
.L7945:
	cmp	dl, 108
	jle	.L7941
	cmp	dl, 116
	je	.L7943
	jmp	.L1005
.L1064:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L7951
	cmp	dl, 97
	je	.L7951
	jmp	.L1005
.L1067:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L7954
	cmp	dl, 115
	jne	.L1005
.L7954:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	jg	.L7956
	cmp	dl, 68
	jg	.L7957
	cmp	dl, 67
	jmp	.L8692
.L7957:
	cmp	dl, 69
	jle	.L7961
	cmp	dl, 81
	jle	.L1005
	jmp	.L7963
.L7956:
	cmp	dl, 100
	jg	.L7965
	cmp	dl, 99
.L8692:
	jne	.L1005
	jmp	.L7964
.L7965:
	cmp	dl, 101
	jle	.L7961
	cmp	dl, 114
	je	.L7963
	jmp	.L1005
.L7964:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 65
	je	.L7971
	cmp	dl, 97
	je	.L7971
	jmp	.L1005
.L7961:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	jg	.L7973
	cmp	dl, 78
	je	.L7975
	cmp	dl, 87
	jle	.L1005
	jmp	.L7977
.L7973:
	cmp	dl, 110
	jg	.L7979
	cmp	dl, 109
	jle	.L1005
	jmp	.L7975
.L7979:
	cmp	dl, 120
	je	.L7977
	jmp	.L1005
.L7963:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L7984
	cmp	dl, 101
	jne	.L1005
.L7984:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7987
	cmp	dl, 116
	jne	.L1005
.L7987:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 984833
	mov	DWORD [edi+8], 8650784
	jmp	.L8696
.L7975:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7993
	cmp	dl, 116
	je	.L7993
	jmp	.L1005
.L7977:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 73
	je	.L7996
	cmp	dl, 105
	jne	.L1005
.L7996:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L7999
	cmp	dl, 116
	jne	.L1005
.L7999:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 996609
	mov	DWORD [edi+8], 8388640
	jmp	.L8696
.L7993:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L8008
	cmp	dl, 101
	jne	.L1005
.L8008:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L8011
	cmp	dl, 114
	jne	.L1005
.L8011:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 996353
.L9010:
	mov	DWORD [edi+8], 32
	jmp	.L8696
.L7971:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L8020
	cmp	dl, 108
	jne	.L1005
.L8020:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L8023
	cmp	dl, 108
	jne	.L1005
.L8023:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 984321
	mov	DWORD [edi+8], 262176
	jmp	.L8696
.L7951:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L8029
	cmp	dl, 112
	jne	.L1005
.L8029:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 71
	je	.L8032
	cmp	dl, 103
	jne	.L1005
.L8032:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L8035
	cmp	dl, 115
	jne	.L1005
.L8035:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], threebyte_insn
	mov	DWORD [edi+4], 251787265
	jmp	.L9148
.L7938:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L8042
	cmp	dl, 99
	je	.L8042
	jmp	.L1005
.L7941:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L8045
	cmp	dl, 100
	je	.L8045
	jmp	.L1005
.L7943:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L8048
	cmp	dl, 115
	jne	.L1005
.L8048:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixsmm_insn
	mov	DWORD [edi+4], 31745
	jmp	.L9141
.L8045:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L8054
	cmp	dl, 116
	jne	.L1005
.L8054:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], cyrixsmm_insn
	mov	DWORD [edi+4], 31233
	jmp	.L9141
.L8042:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], svdc_insn
.L9257:
	mov	DWORD [edi+4], 1
.L9141:
	mov	DWORD [edi+8], 655368
	jmp	.L8696
.L7933:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L8062
	cmp	dl, 79
	jg	.L8063
	test	dl, dl
	jg	.L999
	jmp	.L8070
.L8063:
	cmp	dl, 80
	jle	.L8067
	cmp	dl, 82
	jle	.L999
	jmp	.L8069
.L8062:
	cmp	dl, 112
	jg	.L8071
	cmp	dl, 111
	jle	.L999
	jmp	.L8067
.L8071:
	cmp	dl, 115
	je	.L8069
	jmp	.L999
.L8070:
	mov	DWORD [edi], arith_insn
	mov	DWORD [edi+4], 337943
	jmp	.L8695
.L8067:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L8077
	cmp	dl, 68
	je	.L8079
	cmp	dl, 82
	jle	.L1005
	jmp	.L8081
.L8077:
	cmp	dl, 100
	jg	.L8083
	cmp	dl, 99
	jle	.L1005
	jmp	.L8079
.L8083:
	cmp	dl, 115
	je	.L8081
	jmp	.L1005
.L8069:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L8087
	cmp	dl, 68
	je	.L8089
	cmp	dl, 82
	jle	.L1005
	jmp	.L8091
.L8087:
	cmp	dl, 100
	jg	.L8092
	cmp	dl, 99
	jle	.L1005
	jmp	.L8089
.L8092:
	cmp	dl, 115
	jne	.L1005
.L8091:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15948801
	jmp	.L9152
.L8089:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15883265
	jmp	.L9151
.L8081:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 23553
	jmp	.L9152
.L8079:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6708225
	jmp	.L9151
.L7911:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	movsx	eax, BYTE [ebx+2]
	sub	eax, 48
	or	eax, 96
.L9201:
	mov	DWORD [edi], eax
	jmp	.L8699
.L7914:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 63745
	jmp	.L8695
.L7917:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 64769
	jmp	.L8695
.L7920:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 64257
	jmp	.L8695
.L7926:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L8119
	cmp	dl, 115
	je	.L8119
	jmp	.L1005
.L7929:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], str_insn
	mov	DWORD [edi+4], 4
.L9031:
	mov	DWORD [edi+8], 1048578
	jmp	.L8696
.L7923:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 88
	je	.L8125
	cmp	dl, 120
	jne	.L1005
.L8125:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L8128
	cmp	dl, 99
	jne	.L1005
.L8128:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L8131
	cmp	dl, 115
	jne	.L1005
.L8131:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L8134
	cmp	dl, 114
	jne	.L1005
.L8134:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ldstmxcsr_insn
.L9264:
	mov	DWORD [edi+4], 769
	jmp	.L9152
.L8119:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L8139
	cmp	dl, 68
	jg	.L8140
	cmp	dl, 66
	je	.L8142
	cmp	dl, 67
	jmp	.L8693
.L8140:
	cmp	dl, 81
	je	.L8147
	cmp	dl, 86
	jle	.L1005
	jmp	.L8149
.L8139:
	cmp	dl, 100
	jg	.L8151
	cmp	dl, 98
	je	.L8142
	cmp	dl, 99
.L8693:
	jle	.L1005
	jmp	.L8144
.L8151:
	cmp	dl, 113
	jg	.L8155
	cmp	dl, 112
	jle	.L1005
	jmp	.L8147
.L8155:
	cmp	dl, 119
	je	.L8149
	jmp	.L1005
.L8142:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 43521
	jmp	.L8695
.L8149:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1092353
	jmp	.L8695
.L8144:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2140929
	jmp	.L9186
.L8147:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9214
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 4238081
	jmp	.L9148
.L7898:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L8173
	cmp	dl, 116
	jne	.L1005
.L8173:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L8175
	cmp	dl, 80
	je	.L8177
	cmp	dl, 82
	jle	.L1005
	jmp	.L8179
.L8175:
	cmp	dl, 112
	jg	.L8181
	cmp	dl, 111
	jle	.L1005
	jmp	.L8177
.L8181:
	cmp	dl, 115
	je	.L8179
	jmp	.L1005
.L8177:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L8185
	cmp	dl, 68
	je	.L8187
	cmp	dl, 82
	jle	.L1005
	jmp	.L8189
.L8185:
	cmp	dl, 100
	jg	.L8191
	cmp	dl, 99
	jle	.L1005
	jmp	.L8187
.L8191:
	cmp	dl, 115
	je	.L8189
	jmp	.L1005
.L8179:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L8195
	cmp	dl, 68
	je	.L8197
	cmp	dl, 82
	jle	.L1005
	jmp	.L8199
.L8195:
	cmp	dl, 100
	jg	.L8201
	cmp	dl, 99
	jle	.L1005
	jmp	.L8197
.L8201:
	cmp	dl, 115
	je	.L8199
	jmp	.L1005
.L8197:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15880449
	jmp	.L9151
.L8199:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 15945985
	jmp	.L9152
.L8187:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssess_insn
	mov	DWORD [edi+4], 6705409
	jmp	.L9151
.L8189:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sseps_insn
	mov	DWORD [edi+4], 20737
	jmp	.L9152
.L7894:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L9215
	mov	DWORD [edi], 36
	jmp	.L8699
.L7884:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	je	.L8220
	cmp	dl, 119
	je	.L8220
	jmp	.L1005
.L7882:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	jg	.L8222
	test	dl, dl
	jle	.L8224
	cmp	dl, 77
	jle	.L999
	jmp	.L8226
.L8222:
	cmp	dl, 110
	je	.L8226
	jmp	.L999
.L8224:
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 61697
.L9016:
	mov	DWORD [edi+8], 2097156
	jmp	.L8696
.L8226:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L8232
	cmp	dl, 116
	jne	.L1005
.L8232:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	jg	.L8234
	test	dl, dl
	jle	.L8236
	cmp	dl, 78
	jle	.L999
	jmp	.L8238
.L8234:
	cmp	dl, 111
	je	.L8238
	jmp	.L999
.L8236:
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 997377
	mov	DWORD [edi+8], 131104
	jmp	.L8696
.L8238:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 76
	je	.L8244
	cmp	dl, 108
	jne	.L1005
.L8244:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	je	.L8247
	cmp	dl, 100
	jne	.L1005
.L8247:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobyte_insn
	mov	DWORD [edi+4], 1015297
	mov	DWORD [edi+8], 4325384
	jmp	.L8696
.L8220:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sldtmsw_insn
	mov	DWORD [edi+4], 262406
	jmp	.L9153
.L7878:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L8256
	cmp	dl, 116
	jne	.L1005
.L8256:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], sldtmsw_insn
	mov	DWORD [edi+4], 6
.L9153:
	mov	DWORD [edi+8], 2
	jmp	.L8696
.L7871:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L8262
.L9215:
	push	ebx
	push	DWORD LC15
	jmp	.L8698
.L8262:
	mov	DWORD [edi], 38
.L8699:
	mov	eax, 3
	jmp	.L926
.L7869:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L8264
	cmp	dl, 116
	jne	.L1005
.L8264:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 17760513
	jmp	.L9154
.L7855:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L8270
	cmp	dl, 114
	je	.L8270
	jmp	.L1005
.L7852:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	jg	.L8272
	test	dl, dl
	jle	.L8603
	cmp	dl, 67
	jle	.L999
	jmp	.L8276
.L8272:
	cmp	dl, 100
	je	.L8276
	jmp	.L999
.L8274:
.L7858:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 68
	jg	.L8281
	test	dl, dl
	jle	.L8283
	cmp	dl, 67
	jle	.L999
	jmp	.L8285
.L8281:
	cmp	dl, 100
	je	.L8285
	jmp	.L999
.L8283:
	mov	DWORD [edi], shift_insn
	mov	DWORD [edi+4], 1288
	jmp	.L8695
.L7861:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 70
	je	.L8291
	cmp	dl, 102
	jne	.L1005
.L8291:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 80
	je	.L8294
	cmp	dl, 112
	jne	.L1005
.L8294:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	jg	.L8296
	cmp	dl, 68
	je	.L8298
	cmp	dl, 82
	jle	.L1005
	jmp	.L8300
.L8296:
	cmp	dl, 100
	jg	.L8302
	cmp	dl, 99
	jle	.L1005
	jmp	.L8298
.L8302:
	cmp	dl, 115
	je	.L8300
	jmp	.L1005
.L8298:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssessimm_insn
	mov	DWORD [edi+4], 6735361
.L9151:
	mov	DWORD [edi+8], 32768
	jmp	.L8696
.L8300:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], ssepsimm_insn
	mov	DWORD [edi+4], 50689
.L9152:
	mov	DWORD [edi+8], 16384
	jmp	.L8696
.L8285:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], shlrd_insn
	mov	DWORD [edi+4], 44038
	jmp	.L9186
.L8276:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], shlrd_insn
	mov	DWORD [edi+4], 41990
	jmp	.L9186
.L8270:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L8319
	cmp	dl, 116
	jne	.L1005
.L8319:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 2
	jmp	.L8697
.L7847:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	je	.L8323
	cmp	dl, 116
	jne	.L1005
.L8323:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], twobytemem_insn
	mov	DWORD [edi+4], 983297
.L9154:
	mov	DWORD [edi+8], 8388610
	jmp	.L8696
.L7844:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 78
	je	.L8329
	cmp	dl, 110
	jne	.L1005
.L8329:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	je	.L8332
	cmp	dl, 99
	jne	.L1005
.L8332:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	je	.L8335
	cmp	dl, 101
	jne	.L1005
.L8335:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], threebyte_insn
	mov	DWORD [edi+4], 263124993
.L9024:
	mov	DWORD [edi+8], 64
	jmp	.L8696
.L7841:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 57
	ja	.L1005
	jmp	DWORD [.L8375+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L8375:
	dd	.L8343
	dd	.L8346
	dd	.L8488
	dd	.L1005
	dd	.L8373
	dd	.L1005
	dd	.L8355
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L8358
	dd	.L1005
	dd	.L8361
	dd	.L8364
	dd	.L8367
	dd	.L1005
	dd	.L1005
	dd	.L8370
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L8373
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L8343
	dd	.L8346
	dd	.L8488
	dd	.L1005
	dd	.L8373
	dd	.L1005
	dd	.L8355
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L8358
	dd	.L1005
	dd	.L8361
	dd	.L8364
	dd	.L8367
	dd	.L1005
	dd	.L1005
	dd	.L8370
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L8373
	[section .text]
.L8343:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L8376
	test	dl, dl
	jle	.L9213
	cmp	dl, 68
	jle	.L999
	jmp	.L8380
.L8376:
	cmp	dl, 101
	je	.L8380
	jmp	.L999
.L8378:
.L8346:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L8385
	test	dl, dl
	jle	.L9212
	cmp	dl, 68
	jle	.L999
	jmp	.L8389
.L8385:
	cmp	dl, 101
	je	.L8389
	jmp	.L999
.L8387:
.L8349:
.L8352:
.L8355:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L8400
	test	dl, dl
	jle	.L9210
	cmp	dl, 68
	jle	.L999
	jmp	.L8404
.L8400:
	cmp	dl, 101
	je	.L8404
	jmp	.L999
.L8402:
.L8358:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L8409
	test	dl, dl
	jle	.L9209
	cmp	dl, 68
	jle	.L999
	jmp	.L8413
.L8409:
	cmp	dl, 101
	je	.L8413
	jmp	.L999
.L8411:
.L8361:
	inc	ecx
	mov	dl, BYTE [ecx]
	movsx	eax, dl
	sub	eax, 65
	cmp	eax, 57
	ja	.L1005
	jmp	DWORD [.L8450+eax*4]
	[section	.rodata]
	[align 4]
	[align 4]
.L8450:
	dd	.L8421
	dd	.L8424
	dd	.L8380
	dd	.L1005
	dd	.L8448
	dd	.L1005
	dd	.L8433
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L8436
	dd	.L1005
	dd	.L1005
	dd	.L8439
	dd	.L8442
	dd	.L1005
	dd	.L1005
	dd	.L8445
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L8448
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L8421
	dd	.L8424
	dd	.L8380
	dd	.L1005
	dd	.L8448
	dd	.L1005
	dd	.L8433
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L8436
	dd	.L1005
	dd	.L1005
	dd	.L8439
	dd	.L8442
	dd	.L1005
	dd	.L1005
	dd	.L8445
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L1005
	dd	.L8448
	[section .text]
.L8364:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 1
	jmp	.L9186
.L8367:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 79
	jg	.L8454
	cmp	dl, 68
	jg	.L8455
	test	dl, dl
	jmp	.L9266
.L8455:
	cmp	dl, 69
	jle	.L8459
	cmp	dl, 78
	jle	.L999
	jmp	.L8442
.L8454:
	cmp	dl, 101
	jg	.L8463
	cmp	dl, 100
	jle	.L999
	jmp	.L8459
.L8463:
	cmp	dl, 111
	je	.L8442
	jmp	.L999
.L8370:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 2049
	jmp	.L9186
.L8373:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 1025
	jmp	.L9186
.L8459:
	inc	ecx
	cmp	BYTE [ecx], 0
.L9266:
	jg	.L999
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 2561
	jmp	.L9186
.L8461:
.L8439:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 257
	jmp	.L9186
.L8421:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L8484
	test	dl, dl
	jle	.L9206
	cmp	dl, 68
	jle	.L999
	jmp	.L8488
.L8484:
	cmp	dl, 101
	je	.L8488
	jmp	.L999
.L8486:
.L8424:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L8493
	test	dl, dl
	jle	.L9205
	cmp	dl, 68
	jle	.L999
	jmp	.L8497
.L8493:
	cmp	dl, 101
	je	.L8497
	jmp	.L999
.L8495:
.L8427:
.L8430:
.L8448:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 1281
	jmp	.L9186
.L8445:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 2305
	jmp	.L9186
.L8442:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 2817
	jmp	.L9186
.L8433:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L8517
	test	dl, dl
	jle	.L9203
	cmp	dl, 68
	jle	.L999
	jmp	.L8521
.L8517:
	cmp	dl, 101
	je	.L8521
	jmp	.L999
.L8519:
.L8436:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 69
	jg	.L8526
	test	dl, dl
	jle	.L9202
	cmp	dl, 68
	jle	.L999
	jmp	.L8530
.L8526:
	cmp	dl, 101
	jne	.L999
.L8528:
.L8530:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9210:
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 3841
	jmp	.L9186
.L8521:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9209:
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 3073
	jmp	.L9186
.L8497:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9213:
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 1793
	jmp	.L9186
.L8488:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9212:
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 513
	jmp	.L9186
.L8413:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9203:
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 3585
	jmp	.L9186
.L8404:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9202:
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 3329
	jmp	.L9186
.L8389:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9206:
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 1537
	jmp	.L9186
.L8380:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
.L9205:
	mov	DWORD [edi], setcc_insn
	mov	DWORD [edi+4], 769
	jmp	.L9186
.L7838:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 83
	je	.L8560
	cmp	dl, 115
	jne	.L1005
.L8560:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 87
	jg	.L8562
	cmp	dl, 68
	jg	.L8563
	cmp	dl, 66
	je	.L8565
	cmp	dl, 67
	jmp	.L8694
.L8563:
	cmp	dl, 81
	je	.L8570
	cmp	dl, 86
	jle	.L1005
	jmp	.L8572
.L8562:
	cmp	dl, 100
	jg	.L8574
	cmp	dl, 98
	je	.L8565
	cmp	dl, 99
.L8694:
	jle	.L1005
	jmp	.L8567
.L8574:
	cmp	dl, 113
	jg	.L8578
	cmp	dl, 112
	jle	.L1005
	jmp	.L8570
.L8578:
	cmp	dl, 119
	je	.L8572
	jmp	.L1005
.L8565:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 44545
	jmp	.L8695
.L8572:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 1093377
	jmp	.L8695
.L8567:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 2141953
.L9186:
	mov	DWORD [edi+8], 4
	jmp	.L8696
.L8570:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L8592
.L9214:
	push	ebx
	push	DWORD LC13
.L8698:
	push	esi
	push	DWORD 0
	call	yasm__warning
.L8702:
	mov	eax, 0
	jmp	.L926
.L8592:
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 4239105
.L9148:
	mov	DWORD [edi+8], 16779264
	jmp	.L8696
.L7835:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], arith_insn
	mov	DWORD [edi+4], 202775
	jmp	.L8695
.L7822:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 70
	je	.L8599
	cmp	dl, 102
	je	.L8599
	jmp	.L1005
.L7825:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 67
	jg	.L8601
	test	dl, dl
	jle	.L8603
	cmp	dl, 66
	jle	.L999
	jmp	.L8605
.L8601:
	cmp	dl, 99
	je	.L8605
	jmp	.L999
.L8603:
	mov	DWORD [edi], shift_insn
	mov	DWORD [edi+4], 1032
	jmp	.L8695
.L7827:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], shift_insn
	mov	DWORD [edi+4], 1800
	jmp	.L8695
.L8605:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	je	.L9187
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 54785
	mov	DWORD [edi+8], 2097152
	jmp	.L8696
.L8599:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	cmp	BYTE [yasm_x86_LTX_mode_bits], 64
	jne	.L8620
.L9187:
	sub	esp, 4
	push	ebx
	push	DWORD LC14
	push	esi
	call	yasm__error
	mov	DWORD [edi], not64_insn
	mov	DWORD [edi+4], 1
	mov	DWORD [edi+8], 33554432
	add	esp, 16
	jmp	.L8696
.L8620:
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 40449
	jmp	.L8695
.L1003:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 71
	jg	.L8625
	cmp	dl, 65
	je	.L8627
	cmp	dl, 70
	jle	.L1005
	jmp	.L8629
.L8625:
	cmp	dl, 97
	jg	.L8631
	cmp	dl, 96
	jle	.L1005
	jmp	.L8627
.L8631:
	cmp	dl, 103
	je	.L8629
	jmp	.L1005
.L1006:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 84
	jg	.L8635
	cmp	dl, 80
	je	.L8637
	cmp	dl, 83
	jle	.L1005
	jmp	.L8639
.L8635:
	cmp	dl, 112
	jg	.L8640
	cmp	dl, 111
	jle	.L1005
	jmp	.L8637
.L8640:
	cmp	dl, 116
	jne	.L1005
.L8639:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], f6_insn
	mov	DWORD [edi+4], 516
	jmp	.L8695
.L8637:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], onebyte_insn
	mov	DWORD [edi+4], 36865
	jmp	.L8695
.L8627:
	inc	ecx
	mov	dl, BYTE [ecx]
	cmp	dl, 82
	je	.L8651
	cmp	dl, 114
	je	.L8651
	jmp	.L1005
.L8629:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], f6_insn
	mov	DWORD [edi+4], 772
.L8695:
	mov	DWORD [edi+8], 0
.L8696:
	mov	eax, 1
	jmp	.L926
.L8651:
	inc	ecx
	cmp	BYTE [ecx], 0
	jg	.L999
	mov	DWORD [edi], 1
.L8697:
	mov	eax, 5
.L926:
	lea	esp, [ebp-12]
	pop	ebx
	pop	esi
	pop	edi
	leave
	ret
.Lfe4:
	;.size	yasm_x86__parse_check_id,.Lfe4-yasm_x86__parse_check_id
	;.ident	"GCC: (GNU) 3.2.3"
