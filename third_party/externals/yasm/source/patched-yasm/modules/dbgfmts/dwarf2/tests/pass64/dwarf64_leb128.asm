	.file	"leb128_test.c"
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.text
.Ltext0:
#APP
	.ident	"$Id: intnum.c 1295 2005-10-30 06:13:24Z peter $"
#NO_APP
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"0"
	.section	.rodata
.LC1:
	.string	""
	.string	""
	.section	.rodata.str1.1
.LC2:
	.string	"2"
.LC3:
	.string	"\002"
.LC4:
	.string	"7F"
.LC5:
	.string	"\177"
.LC6:
	.string	"80"
.LC7:
	.string	"\200\001"
.LC8:
	.string	"81"
.LC9:
	.string	"\201\001"
.LC10:
	.string	"82"
.LC11:
	.string	"\202\001"
.LC12:
	.string	"3239"
.LC13:
	.string	"\271d"
	.section	.rodata
.LC14:
	.string	"\377"
	.string	""
	.section	.rodata.str1.1
.LC15:
	.string	"~"
.LC16:
	.string	"\201\177"
.LC17:
	.string	"\200\177"
.LC18:
	.string	"\377~"
	.data
	.align 32
	.type	tests, @object
	.size	tests, 512
tests:
	.long	0
	.long	0
	.quad	.LC0
	.quad	1
	.quad	.LC1
	.long	0
	.long	0
	.quad	.LC2
	.quad	1
	.quad	.LC3
	.long	0
	.long	0
	.quad	.LC4
	.quad	1
	.quad	.LC5
	.long	0
	.long	0
	.quad	.LC6
	.quad	2
	.quad	.LC7
	.long	0
	.long	0
	.quad	.LC8
	.quad	2
	.quad	.LC9
	.long	0
	.long	0
	.quad	.LC10
	.quad	2
	.quad	.LC11
	.long	0
	.long	0
	.quad	.LC12
	.quad	2
	.quad	.LC13
	.long	1
	.long	0
	.quad	.LC0
	.quad	1
	.quad	.LC1
	.long	1
	.long	0
	.quad	.LC2
	.quad	1
	.quad	.LC3
	.long	1
	.long	0
	.quad	.LC4
	.quad	2
	.quad	.LC14
	.long	1
	.long	0
	.quad	.LC6
	.quad	2
	.quad	.LC7
	.long	1
	.long	0
	.quad	.LC8
	.quad	2
	.quad	.LC9
	.long	1
	.long	1
	.quad	.LC2
	.quad	1
	.quad	.LC15
	.long	1
	.long	1
	.quad	.LC4
	.quad	2
	.quad	.LC16
	.long	1
	.long	1
	.quad	.LC6
	.quad	2
	.quad	.LC17
	.long	1
	.long	1
	.quad	.LC8
	.quad	2
	.quad	.LC18
	.text
	.p2align 4,,15
.globl yasm_intnum_initialize
	.type	yasm_intnum_initialize, @function
yasm_intnum_initialize:
.LFB25:
	.file 1 "./libyasm/intnum.c"
	.loc 1 65 0
.LVL0:
	subq	$8, %rsp
.LCFI0:
.LVL1:
	.loc 1 66 0
	xorl	%esi, %esi
	movl	$128, %edi
	call	BitVector_Create
	.loc 1 67 0
	xorl	%esi, %esi
	movl	$128, %edi
	.loc 1 66 0
	movq	%rax, conv_bv(%rip)
	.loc 1 67 0
	call	BitVector_Create
	.loc 1 68 0
	xorl	%esi, %esi
	movl	$128, %edi
	.loc 1 67 0
	movq	%rax, result(%rip)
	.loc 1 68 0
	call	BitVector_Create
	.loc 1 69 0
	xorl	%esi, %esi
	movl	$128, %edi
	.loc 1 68 0
	movq	%rax, spare(%rip)
	.loc 1 69 0
	call	BitVector_Create
	.loc 1 70 0
	xorl	%esi, %esi
	movl	$128, %edi
	.loc 1 69 0
	movq	%rax, op1static(%rip)
	.loc 1 70 0
	call	BitVector_Create
	.loc 1 71 0
	movl	$128, %edi
	.loc 1 70 0
	movq	%rax, op2static(%rip)
	.loc 1 71 0
	call	BitVector_from_Dec_static_Boot
	movq	%rax, from_dec_data(%rip)
	.loc 1 72 0
	addq	$8, %rsp
.LVL2:
	ret
.LFE25:
	.size	yasm_intnum_initialize, .-yasm_intnum_initialize
	.p2align 4,,15
.globl yasm_intnum_cleanup
	.type	yasm_intnum_cleanup, @function
yasm_intnum_cleanup:
.LFB26:
	.loc 1 76 0
.LVL3:
	subq	$8, %rsp
.LCFI1:
.LVL4:
	.loc 1 77 0
	movq	from_dec_data(%rip), %rdi
	call	BitVector_from_Dec_static_Shutdown
	.loc 1 78 0
	movq	op2static(%rip), %rdi
	call	BitVector_Destroy
	.loc 1 79 0
	movq	op1static(%rip), %rdi
	call	BitVector_Destroy
	.loc 1 80 0
	movq	spare(%rip), %rdi
	call	BitVector_Destroy
	.loc 1 81 0
	movq	result(%rip), %rdi
	call	BitVector_Destroy
	.loc 1 82 0
	movq	conv_bv(%rip), %rdi
	.loc 1 83 0
	addq	$8, %rsp
.LVL5:
	.loc 1 82 0
	jmp	BitVector_Destroy
.LFE26:
	.size	yasm_intnum_cleanup, .-yasm_intnum_cleanup
	.section	.rodata.str1.8,"aMS",@progbits,1
	.align 8
.LC19:
	.string	"Numeric constant too large for internal format"
	.text
	.p2align 4,,15
.globl yasm_intnum_create_dec
	.type	yasm_intnum_create_dec, @function
yasm_intnum_create_dec:
.LFB27:
	.loc 1 87 0
.LVL6:
	movq	%rbx, -24(%rsp)
.LCFI2:
	movq	%rbp, -16(%rsp)
.LCFI3:
	movq	%rdi, %rbx
	movq	%r12, -8(%rsp)
.LCFI4:
	.loc 1 88 0
	movl	$16, %edi
.LVL7:
	.loc 1 87 0
	subq	$24, %rsp
.LCFI5:
.LVL8:
	.loc 1 87 0
	movq	%rsi, %r12
	.loc 1 88 0
	call	*yasm_xmalloc(%rip)
.LVL9:
.LVL10:
	.loc 1 90 0
	movb	$0, 12(%rax)
	.loc 1 92 0
	movq	conv_bv(%rip), %rsi
	movq	%rbx, %rdx
	movq	from_dec_data(%rip), %rdi
.LVL11:
	.loc 1 88 0
	movq	%rax, %rbp
.LVL12:
	.loc 1 92 0
	call	BitVector_from_Dec_static
	cmpl	$12, %eax
	je	.L13
.L6:
	.loc 1 96 0
	movq	conv_bv(%rip), %rdi
	call	Set_Max
	cmpq	$31, %rax
	jg	.L8
	.loc 1 98 0
	movq	conv_bv(%rip), %rdi
	.loc 1 97 0
	movl	$0, 8(%rbp)
	.loc 1 98 0
	xorl	%edx, %edx
	movl	$32, %esi
	call	BitVector_Chunk_Read
	movq	%rax, (%rbp)
	.loc 1 105 0
	movq	%rbp, %rax
	movq	(%rsp), %rbx
.LVL13:
	movq	8(%rsp), %rbp
.LVL14:
	movq	16(%rsp), %r12
.LVL15:
	addq	$24, %rsp
.LVL16:
	ret
.LVL17:
	.p2align 4,,7
.L8:
	.loc 1 101 0
	movq	conv_bv(%rip), %rdi
	.loc 1 100 0
	movl	$1, 8(%rbp)
	.loc 1 101 0
	call	BitVector_Clone
	movq	%rax, (%rbp)
	.loc 1 105 0
	movq	%rbp, %rax
	movq	(%rsp), %rbx
.LVL18:
	movq	8(%rsp), %rbp
.LVL19:
	movq	16(%rsp), %r12
.LVL20:
	addq	$24, %rsp
.LVL21:
	ret
.LVL22:
	.p2align 4,,7
.L13:
	.loc 1 94 0
	movl	$.LC19, %edx
	movq	%r12, %rsi
	xorl	%edi, %edi
	xorl	%eax, %eax
	call	yasm__warning
	jmp	.L6
.LFE27:
	.size	yasm_intnum_create_dec, .-yasm_intnum_create_dec
	.p2align 4,,15
.globl yasm_intnum_create_bin
	.type	yasm_intnum_create_bin, @function
yasm_intnum_create_bin:
.LFB28:
	.loc 1 109 0
.LVL23:
	movq	%rbp, -16(%rsp)
.LCFI6:
	movq	%rdi, %rbp
	movq	%rbx, -24(%rsp)
.LCFI7:
	movq	%r12, -8(%rsp)
.LCFI8:
	.loc 1 110 0
	movl	$16, %edi
.LVL24:
	.loc 1 109 0
	subq	$24, %rsp
.LCFI9:
.LVL25:
	.loc 1 109 0
	movq	%rsi, %r12
	.loc 1 110 0
	call	*yasm_xmalloc(%rip)
.LVL26:
	.loc 1 112 0
	movq	%rbp, %rdi
	.loc 1 110 0
	movq	%rax, %rbx
.LVL27:
	.loc 1 112 0
	call	strlen
	.loc 1 114 0
	cmpb	$-128, %al
	.loc 1 112 0
	movb	%al, 12(%rbx)
	.loc 1 114 0
	ja	.L21
.L15:
	.loc 1 118 0
	movq	conv_bv(%rip), %rdi
	movq	%rbp, %rsi
	call	BitVector_from_Bin
	.loc 1 119 0
	movq	conv_bv(%rip), %rdi
	call	Set_Max
	cmpq	$31, %rax
	jg	.L17
	.loc 1 121 0
	movq	conv_bv(%rip), %rdi
	.loc 1 120 0
	movl	$0, 8(%rbx)
	.loc 1 121 0
	xorl	%edx, %edx
	movl	$32, %esi
	call	BitVector_Chunk_Read
	movq	%rax, (%rbx)
	.loc 1 128 0
	movq	%rbx, %rax
	movq	8(%rsp), %rbp
.LVL28:
	movq	(%rsp), %rbx
.LVL29:
	movq	16(%rsp), %r12
.LVL30:
	addq	$24, %rsp
.LVL31:
	ret
.LVL32:
	.p2align 4,,7
.L17:
	.loc 1 124 0
	movq	conv_bv(%rip), %rdi
	.loc 1 123 0
	movl	$1, 8(%rbx)
	.loc 1 124 0
	call	BitVector_Clone
	movq	%rax, (%rbx)
	.loc 1 128 0
	movq	%rbx, %rax
	movq	8(%rsp), %rbp
.LVL33:
	movq	(%rsp), %rbx
.LVL34:
	movq	16(%rsp), %r12
.LVL35:
	addq	$24, %rsp
.LVL36:
	ret
.LVL37:
	.p2align 4,,7
.L21:
	.loc 1 115 0
	movl	$.LC19, %edx
	movq	%r12, %rsi
	xorl	%edi, %edi
	xorl	%eax, %eax
	call	yasm__warning
	jmp	.L15
.LFE28:
	.size	yasm_intnum_create_bin, .-yasm_intnum_create_bin
	.p2align 4,,15
.globl yasm_intnum_create_oct
	.type	yasm_intnum_create_oct, @function
yasm_intnum_create_oct:
.LFB29:
	.loc 1 132 0
.LVL38:
	movq	%rbp, -16(%rsp)
.LCFI10:
	movq	%rdi, %rbp
	movq	%rbx, -24(%rsp)
.LCFI11:
	movq	%r12, -8(%rsp)
.LCFI12:
	.loc 1 133 0
	movl	$16, %edi
.LVL39:
	.loc 1 132 0
	subq	$24, %rsp
.LCFI13:
.LVL40:
	.loc 1 132 0
	movq	%rsi, %r12
	.loc 1 133 0
	call	*yasm_xmalloc(%rip)
.LVL41:
	.loc 1 135 0
	movq	%rbp, %rdi
	.loc 1 133 0
	movq	%rax, %rbx
.LVL42:
	.loc 1 135 0
	call	strlen
	leaq	(%rax,%rax,2), %rax
	.loc 1 137 0
	cmpb	$-128, %al
	.loc 1 135 0
	movb	%al, 12(%rbx)
	.loc 1 137 0
	ja	.L29
.L23:
	.loc 1 141 0
	movq	conv_bv(%rip), %rdi
	movq	%rbp, %rsi
	call	BitVector_from_Oct
	.loc 1 142 0
	movq	conv_bv(%rip), %rdi
	call	Set_Max
	cmpq	$31, %rax
	jg	.L25
	.loc 1 144 0
	movq	conv_bv(%rip), %rdi
	.loc 1 143 0
	movl	$0, 8(%rbx)
	.loc 1 144 0
	xorl	%edx, %edx
	movl	$32, %esi
	call	BitVector_Chunk_Read
	movq	%rax, (%rbx)
	.loc 1 151 0
	movq	%rbx, %rax
	movq	8(%rsp), %rbp
.LVL43:
	movq	(%rsp), %rbx
.LVL44:
	movq	16(%rsp), %r12
.LVL45:
	addq	$24, %rsp
.LVL46:
	ret
.LVL47:
	.p2align 4,,7
.L25:
	.loc 1 147 0
	movq	conv_bv(%rip), %rdi
	.loc 1 146 0
	movl	$1, 8(%rbx)
	.loc 1 147 0
	call	BitVector_Clone
	movq	%rax, (%rbx)
	.loc 1 151 0
	movq	%rbx, %rax
	movq	8(%rsp), %rbp
.LVL48:
	movq	(%rsp), %rbx
.LVL49:
	movq	16(%rsp), %r12
.LVL50:
	addq	$24, %rsp
.LVL51:
	ret
.LVL52:
	.p2align 4,,7
.L29:
	.loc 1 138 0
	movl	$.LC19, %edx
	movq	%r12, %rsi
	xorl	%edi, %edi
	xorl	%eax, %eax
	call	yasm__warning
	jmp	.L23
.LFE29:
	.size	yasm_intnum_create_oct, .-yasm_intnum_create_oct
	.p2align 4,,15
.globl yasm_intnum_create_hex
	.type	yasm_intnum_create_hex, @function
yasm_intnum_create_hex:
.LFB30:
	.loc 1 155 0
.LVL53:
	movq	%rbp, -16(%rsp)
.LCFI14:
	movq	%rdi, %rbp
	movq	%rbx, -24(%rsp)
.LCFI15:
	movq	%r12, -8(%rsp)
.LCFI16:
	.loc 1 156 0
	movl	$16, %edi
.LVL54:
	.loc 1 155 0
	subq	$24, %rsp
.LCFI17:
.LVL55:
	.loc 1 155 0
	movq	%rsi, %r12
	.loc 1 156 0
	call	*yasm_xmalloc(%rip)
.LVL56:
	.loc 1 158 0
	movq	%rbp, %rdi
	.loc 1 156 0
	movq	%rax, %rbx
.LVL57:
	.loc 1 158 0
	call	strlen
	salq	$2, %rax
	.loc 1 160 0
	cmpb	$-128, %al
	.loc 1 158 0
	movb	%al, 12(%rbx)
	.loc 1 160 0
	ja	.L37
.L31:
	.loc 1 164 0
	movq	conv_bv(%rip), %rdi
	movq	%rbp, %rsi
	call	BitVector_from_Hex
	.loc 1 165 0
	movq	conv_bv(%rip), %rdi
	call	Set_Max
	cmpq	$31, %rax
	jg	.L33
	.loc 1 167 0
	movq	conv_bv(%rip), %rdi
	.loc 1 166 0
	movl	$0, 8(%rbx)
	.loc 1 167 0
	xorl	%edx, %edx
	movl	$32, %esi
	call	BitVector_Chunk_Read
	movq	%rax, (%rbx)
	.loc 1 174 0
	movq	%rbx, %rax
	movq	8(%rsp), %rbp
.LVL58:
	movq	(%rsp), %rbx
.LVL59:
	movq	16(%rsp), %r12
.LVL60:
	addq	$24, %rsp
.LVL61:
	ret
.LVL62:
	.p2align 4,,7
.L33:
	.loc 1 170 0
	movq	conv_bv(%rip), %rdi
	.loc 1 169 0
	movl	$1, 8(%rbx)
	.loc 1 170 0
	call	BitVector_Clone
	movq	%rax, (%rbx)
	.loc 1 174 0
	movq	%rbx, %rax
	movq	8(%rsp), %rbp
.LVL63:
	movq	(%rsp), %rbx
.LVL64:
	movq	16(%rsp), %r12
.LVL65:
	addq	$24, %rsp
.LVL66:
	ret
.LVL67:
	.p2align 4,,7
.L37:
	.loc 1 161 0
	movl	$.LC19, %edx
	movq	%r12, %rsi
	xorl	%edi, %edi
	xorl	%eax, %eax
	call	yasm__warning
	jmp	.L31
.LFE30:
	.size	yasm_intnum_create_hex, .-yasm_intnum_create_hex
	.section	.rodata.str1.8
	.align 8
.LC20:
	.string	"Character constant too large for internal format"
	.text
	.p2align 4,,15
.globl yasm_intnum_create_charconst_nasm
	.type	yasm_intnum_create_charconst_nasm, @function
yasm_intnum_create_charconst_nasm:
.LFB31:
	.loc 1 179 0
.LVL68:
	movq	%rbx, -32(%rsp)
.LCFI18:
	movq	%rdi, %rbx
	movq	%rbp, -24(%rsp)
.LCFI19:
	movq	%r12, -16(%rsp)
.LCFI20:
	movq	%r13, -8(%rsp)
.LCFI21:
	movq	%rsi, %rbp
	subq	$40, %rsp
.LCFI22:
.LVL69:
	.loc 1 180 0
	movl	$16, %edi
.LVL70:
	call	*yasm_xmalloc(%rip)
.LVL71:
	.loc 1 181 0
	movq	%rbx, %rdi
	.loc 1 180 0
	movq	%rax, %r13
.LVL72:
	.loc 1 181 0
	call	strlen
	movq	%rax, %r12
	.loc 1 183 0
	leaq	0(,%rax,8), %rax
	.loc 1 185 0
	cmpb	$-128, %al
	.loc 1 183 0
	movb	%al, 12(%r13)
	.loc 1 185 0
	ja	.L59
.L39:
	.loc 1 189 0
	cmpq	$4, %r12
	ja	.L60
	.loc 1 193 0
	movq	$0, (%r13)
	.loc 1 194 0
	movl	$0, 8(%r13)
.L43:
	.loc 1 197 0
	cmpl	$4, %r12d
	ja	.L44
	mov	%r12d, %eax
.LVL73:
	jmp	*.L50(,%rax,8)
	.section	.rodata
	.align 8
	.align 4
.L50:
	.quad	.L45
	.quad	.L55
	.quad	.L56
	.quad	.L57
	.quad	.L49
	.text
.L49:
	.loc 1 199 0
	movsbq	3(%rbx),%rdx
	orq	(%r13), %rdx
	.loc 1 200 0
	salq	$8, %rdx
	movq	%rdx, (%r13)
.L48:
	.loc 1 203 0
	movsbq	2(%rbx),%rax
.LVL74:
	orq	%rax, %rdx
	.loc 1 204 0
	salq	$8, %rdx
	movq	%rdx, (%r13)
.LVL75:
.L47:
	.loc 1 207 0
	movsbq	1(%rbx),%rax
.LVL76:
	orq	%rax, %rdx
	.loc 1 208 0
	salq	$8, %rdx
	movq	%rdx, (%r13)
.LVL77:
.L46:
	.loc 1 211 0
	movsbq	(%rbx),%rax
.LVL78:
	orq	%rax, %rdx
	movq	%rdx, (%r13)
.LVL79:
.L45:
	.loc 1 225 0
	movq	%r13, %rax
.LVL80:
	movq	8(%rsp), %rbx
.LVL81:
	movq	16(%rsp), %rbp
.LVL82:
	movq	24(%rsp), %r12
	movq	32(%rsp), %r13
.LVL83:
	addq	$40, %rsp
.LVL84:
	ret
.LVL85:
	.p2align 4,,7
.L44:
	.loc 1 216 0
	testq	%r12, %r12
	jne	.L61
	.loc 1 221 0
	movq	conv_bv(%rip), %rdi
	call	BitVector_Clone
	movq	%rax, (%r13)
	jmp	.L45
	.p2align 4,,7
.L60:
	.loc 1 190 0
	movq	conv_bv(%rip), %rdi
	call	BitVector_Empty
	.loc 1 191 0
	movl	$1, 8(%r13)
	jmp	.L43
	.p2align 4,,7
.L59:
	.loc 1 186 0
	movl	$.LC20, %edx
	movq	%rbp, %rsi
	xorl	%edi, %edi
	xorl	%eax, %eax
	call	yasm__warning
	jmp	.L39
	.p2align 4,,7
.L61:
	.loc 1 179 0
	leaq	(%rbx,%r12), %rbp
.LVL86:
	xorl	%ebx, %ebx
.LVL87:
	.p2align 4,,7
.L51:
	.loc 1 217 0
	movq	conv_bv(%rip), %rdi
	movl	$8, %esi
	.loc 1 218 0
	incq	%rbx
	.loc 1 217 0
	call	BitVector_Move_Left
	.loc 1 218 0
	movsbq	-1(%rbp),%rcx
	movq	conv_bv(%rip), %rdi
	xorl	%edx, %edx
	movl	$8, %esi
	decq	%rbp
	call	BitVector_Chunk_Store
	.loc 1 216 0
	cmpq	%rbx, %r12
	jne	.L51
	.loc 1 221 0
	movq	conv_bv(%rip), %rdi
	call	BitVector_Clone
	movq	%rax, (%r13)
	jmp	.L45
.LVL88:
.L57:
	movq	(%r13), %rdx
	jmp	.L48
.L56:
	movq	(%r13), %rdx
	.p2align 4,,5
	jmp	.L47
.L55:
	movq	(%r13), %rdx
	.p2align 4,,5
	jmp	.L46
.LFE31:
	.size	yasm_intnum_create_charconst_nasm, .-yasm_intnum_create_charconst_nasm
	.p2align 4,,15
.globl yasm_intnum_create_uint
	.type	yasm_intnum_create_uint, @function
yasm_intnum_create_uint:
.LFB32:
	.loc 1 230 0
.LVL89:
	pushq	%rbx
.LCFI23:
.LVL90:
	.loc 1 230 0
	movq	%rdi, %rbx
	.loc 1 231 0
	movl	$16, %edi
.LVL91:
	call	*yasm_xmalloc(%rip)
	.loc 1 233 0
	movq	%rbx, (%rax)
	.loc 1 234 0
	movl	$0, 8(%rax)
	.loc 1 235 0
	movb	$0, 12(%rax)
	.loc 1 238 0
	popq	%rbx
.LVL92:
	ret
.LFE32:
	.size	yasm_intnum_create_uint, .-yasm_intnum_create_uint
	.p2align 4,,15
.globl yasm_intnum_create_int
	.type	yasm_intnum_create_int, @function
yasm_intnum_create_int:
.LFB33:
	.loc 1 242 0
.LVL93:
	.loc 1 246 0
	testq	%rdi, %rdi
	.loc 1 242 0
	pushq	%rbx
.LCFI24:
.LVL94:
	.loc 1 242 0
	movq	%rdi, %rbx
	.loc 1 246 0
	js	.L65
.LVL95:
	.loc 1 259 0
	popq	%rbx
.LVL96:
	.loc 1 247 0
	jmp	yasm_intnum_create_uint
.LVL97:
	.p2align 4,,7
.L65:
	.loc 1 249 0
	movq	conv_bv(%rip), %rdi
	.loc 1 250 0
	negq	%rbx
	.loc 1 249 0
	call	BitVector_Empty
	.loc 1 250 0
	movq	conv_bv(%rip), %rdi
	movq	%rbx, %rcx
	xorl	%edx, %edx
	movl	$32, %esi
	call	BitVector_Chunk_Store
	.loc 1 251 0
	movq	conv_bv(%rip), %rdi
	movq	%rdi, %rsi
	call	BitVector_Negate
	.loc 1 253 0
	movl	$16, %edi
	call	*yasm_xmalloc(%rip)
	.loc 1 254 0
	movq	conv_bv(%rip), %rdi
	.loc 1 253 0
	movq	%rax, %rbx
.LVL98:
	.loc 1 254 0
	call	BitVector_Clone
	.loc 1 255 0
	movl	$1, 8(%rbx)
	.loc 1 254 0
	movq	%rax, (%rbx)
	.loc 1 259 0
	movq	%rbx, %rax
	.loc 1 256 0
	movb	$0, 12(%rbx)
	.loc 1 259 0
	popq	%rbx
.LVL99:
	ret
.LFE33:
	.size	yasm_intnum_create_int, .-yasm_intnum_create_int
	.p2align 4,,15
.globl yasm_intnum_copy
	.type	yasm_intnum_copy, @function
yasm_intnum_copy:
.LFB34:
	.loc 1 263 0
.LVL100:
	movq	%rbp, -8(%rsp)
.LCFI25:
	movq	%rdi, %rbp
	movq	%rbx, -16(%rsp)
.LCFI26:
	.loc 1 264 0
	movl	$16, %edi
.LVL101:
	.loc 1 263 0
	subq	$24, %rsp
.LCFI27:
.LVL102:
	.loc 1 264 0
	call	*yasm_xmalloc(%rip)
	.loc 1 266 0
	movl	8(%rbp), %edx
	.loc 1 264 0
	movq	%rax, %rbx
.LVL103:
	.loc 1 266 0
	testl	%edx, %edx
	je	.L71
	cmpl	$1, %edx
	je	.L74
	.loc 1 275 0
	movzbl	12(%rbp), %eax
	.loc 1 274 0
	movl	%edx, 8(%rbx)
	.loc 1 275 0
	movb	%al, 12(%rbx)
	.loc 1 278 0
	movq	%rbx, %rax
	movq	16(%rsp), %rbp
.LVL104:
	movq	8(%rsp), %rbx
.LVL105:
	addq	$24, %rsp
.LVL106:
	ret
.LVL107:
	.p2align 4,,7
.L71:
	.loc 1 268 0
	movq	(%rbp), %rax
	.loc 1 274 0
	movl	%edx, 8(%rbx)
	.loc 1 268 0
	movq	%rax, (%rbx)
	.loc 1 275 0
	movzbl	12(%rbp), %eax
	movb	%al, 12(%rbx)
	.loc 1 278 0
	movq	%rbx, %rax
	movq	16(%rsp), %rbp
.LVL108:
	movq	8(%rsp), %rbx
.LVL109:
	addq	$24, %rsp
.LVL110:
	ret
.LVL111:
	.p2align 4,,7
.L74:
	.loc 1 271 0
	movq	(%rbp), %rdi
	call	BitVector_Clone
	movq	%rax, (%rbx)
	.loc 1 275 0
	movzbl	12(%rbp), %eax
	movl	8(%rbp), %edx
	movb	%al, 12(%rbx)
	.loc 1 274 0
	movl	%edx, 8(%rbx)
	.loc 1 278 0
	movq	%rbx, %rax
	movq	16(%rsp), %rbp
.LVL112:
	movq	8(%rsp), %rbx
.LVL113:
	addq	$24, %rsp
.LVL114:
	ret
.LFE34:
	.size	yasm_intnum_copy, .-yasm_intnum_copy
	.p2align 4,,15
.globl yasm_intnum_destroy
	.type	yasm_intnum_destroy, @function
yasm_intnum_destroy:
.LFB35:
	.loc 1 282 0
.LVL115:
	pushq	%rbx
.LCFI28:
.LVL116:
	.loc 1 283 0
	cmpl	$1, 8(%rdi)
	.loc 1 282 0
	movq	%rdi, %rbx
	.loc 1 283 0
	je	.L79
.LVL117:
	.loc 1 285 0
	movq	%rbx, %rdi
	movq	yasm_xfree(%rip), %r11
	.loc 1 286 0
	popq	%rbx
.LVL118:
	.loc 1 285 0
	jmp	*%r11
.LVL119:
.LVL120:
	.p2align 4,,7
.L79:
	.loc 1 284 0
	movq	(%rdi), %rdi
	call	BitVector_Destroy
	.loc 1 285 0
	movq	%rbx, %rdi
	movq	yasm_xfree(%rip), %r11
	.loc 1 286 0
	popq	%rbx
.LVL121:
	.loc 1 285 0
	jmp	*%r11
.LVL122:
.LFE35:
	.size	yasm_intnum_destroy, .-yasm_intnum_destroy
	.section	.rodata.str1.1
.LC21:
	.string	"Operation needs an operand"
.LC22:
	.string	"./libyasm/intnum.c"
.LC23:
	.string	"SEG"
.LC24:
	.string	"invalid use of '%s'"
.LC25:
	.string	"WRT"
.LC26:
	.string	":"
	.section	.rodata.str1.8
	.align 8
.LC27:
	.string	"invalid operation in intnum calculation"
	.text
	.p2align 4,,15
.globl yasm_intnum_calc
	.type	yasm_intnum_calc, @function
yasm_intnum_calc:
.LFB36:
	.loc 1 292 0
.LVL123:
	movq	%rbx, -48(%rsp)
.LCFI29:
	movq	%rbp, -40(%rsp)
.LCFI30:
	movq	%rdx, %rbx
	movq	%r14, -16(%rsp)
.LCFI31:
	movq	%r15, -8(%rsp)
.LCFI32:
	movq	%rdi, %r14
	movq	%r12, -32(%rsp)
.LCFI33:
	movq	%r13, -24(%rsp)
.LCFI34:
	subq	$72, %rsp
.LCFI35:
.LVL124:
	.loc 1 299 0
	cmpl	$1, 8(%rdi)
	.loc 1 292 0
	movl	%esi, %ebp
	movq	%rcx, %r15
	.loc 1 293 0
	movl	$0, 20(%rsp)
.LVL125:
	.loc 1 299 0
	je	.L146
.LVL126:
	.loc 1 302 0
	movq	op1static(%rip), %r12
.LVL127:
	.loc 1 303 0
	movq	%r12, %rdi
.LVL128:
	call	BitVector_Empty
	.loc 1 304 0
	movq	(%r14), %rcx
	xorl	%edx, %edx
	movl	$32, %esi
	movq	%r12, %rdi
	call	BitVector_Chunk_Store
.LVL129:
.L83:
	.loc 1 307 0
	xorl	%r13d, %r13d
.LVL130:
	testq	%rbx, %rbx
	je	.L86
	.loc 1 308 0
	cmpl	$1, 8(%rbx)
	je	.L147
	.loc 1 311 0
	movq	op2static(%rip), %r13
	.loc 1 312 0
	movq	%r13, %rdi
	call	BitVector_Empty
	.loc 1 313 0
	movq	(%rbx), %rcx
	xorl	%edx, %edx
	movl	$32, %esi
	movq	%r13, %rdi
	call	BitVector_Chunk_Store
.LVL131:
.L86:
	.loc 1 317 0
	testq	%rbx, %rbx
	sete	%dl
	cmpl	$8, %ebp
	setne	%al
	testb	%al, %dl
	je	.L89
	cmpl	$9, %ebp
	setne	%dl
	cmpl	$18, %ebp
	setne	%al
	testb	%al, %dl
	jne	.L148
.L89:
	.loc 1 322 0
	cmpl	$28, %ebp
	ja	.L92
	mov	%ebp, %eax
	jmp	*.L121(,%rax,8)
	.section	.rodata
	.align 8
	.align 4
.L121:
	.quad	.L93
	.quad	.L94
	.quad	.L95
	.quad	.L96
	.quad	.L98
	.quad	.L98
	.quad	.L100
	.quad	.L100
	.quad	.L101
	.quad	.L102
	.quad	.L103
	.quad	.L104
	.quad	.L105
	.quad	.L106
	.quad	.L107
	.quad	.L108
	.quad	.L109
	.quad	.L110
	.quad	.L111
	.quad	.L112
	.quad	.L113
	.quad	.L114
	.quad	.L115
	.quad	.L116
	.quad	.L117
	.quad	.L92
	.quad	.L118
	.quad	.L119
	.quad	.L120
	.text
	.p2align 4,,7
.L92:
	.loc 1 431 0
	movl	$.LC27, %edx
	movl	$431, %esi
	movl	$.LC22, %edi
	call	*yasm_internal_error_(%rip)
.LVL132:
.L122:
	.loc 1 435 0
	movq	result(%rip), %rdi
	call	Set_Max
	cmpq	$31, %rax
	jg	.L136
	.loc 1 436 0
	cmpl	$1, 8(%r14)
	je	.L149
.L138:
	.loc 1 440 0
	movq	result(%rip), %rdi
	xorl	%edx, %edx
	movl	$32, %esi
	call	BitVector_Chunk_Read
	movq	%rax, (%r14)
.L143:
	.loc 1 449 0
	movq	24(%rsp), %rbx
.LVL133:
	movq	32(%rsp), %rbp
.LVL134:
	movq	40(%rsp), %r12
.LVL135:
	movq	48(%rsp), %r13
.LVL136:
	movq	56(%rsp), %r14
.LVL137:
	movq	64(%rsp), %r15
.LVL138:
	addq	$72, %rsp
.LVL139:
	ret
.LVL140:
	.p2align 4,,7
.L136:
	.loc 1 442 0
	cmpl	$1, 8(%r14)
	je	.L150
	.loc 1 446 0
	movq	result(%rip), %rdi
	.loc 1 445 0
	movl	$1, 8(%r14)
	.loc 1 446 0
	call	BitVector_Clone
	movq	%rax, (%r14)
	jmp	.L143
.LVL141:
	.p2align 4,,7
.L147:
	.loc 1 309 0
	movq	(%rbx), %r13
	jmp	.L86
.LVL142:
	.p2align 4,,7
.L146:
	.loc 1 300 0
	movq	(%rdi), %r12
.LVL143:
	.p2align 4,,5
	jmp	.L83
.LVL144:
	.p2align 4,,7
.L148:
	.loc 1 319 0
	movl	$.LC21, %edx
	movl	$319, %esi
	movl	$.LC22, %edi
	call	*yasm_internal_error_(%rip)
	jmp	.L89
	.p2align 4,,7
.L150:
	.loc 1 443 0
	movq	result(%rip), %rsi
	movq	(%r14), %rdi
	call	BitVector_Copy
	jmp	.L143
.L149:
	.loc 1 437 0
	movq	(%r14), %rdi
	call	BitVector_Destroy
	.loc 1 438 0
	movl	$0, 8(%r14)
	.p2align 4,,4
	jmp	.L138
.LVL145:
.L100:
	.loc 1 344 0
	movq	result(%rip), %rcx
	movq	spare(%rip), %rdi
	movq	%r13, %rdx
	movq	%r12, %rsi
	call	BitVector_Divide
.LVL146:
	jmp	.L122
.LVL147:
.L98:
	.loc 1 337 0
	movq	spare(%rip), %rcx
	movq	result(%rip), %rdi
	movq	%r13, %rdx
	movq	%r12, %rsi
	call	BitVector_Divide
.LVL148:
	jmp	.L122
.LVL149:
.L111:
	.loc 1 390 0
	movq	result(%rip), %rdi
	call	BitVector_Empty
.LVL150:
	.loc 1 391 0
	movq	%r12, %rdi
	call	BitVector_is_empty
	movl	%eax, %esi
.L145:
	.loc 1 395 0
	movq	result(%rip), %rdi
	call	BitVector_LSB
	jmp	.L122
.LVL151:
.L112:
	.loc 1 398 0
	movq	result(%rip), %rdi
	call	BitVector_Empty
.LVL152:
	.loc 1 399 0
	movq	%r13, %rsi
	movq	%r12, %rdi
	call	BitVector_Lexicompare
	movq	result(%rip), %rdi
	movl	%eax, %esi
	shrl	$31, %esi
	call	BitVector_LSB
	jmp	.L122
.LVL153:
.L113:
	.loc 1 402 0
	movq	result(%rip), %rdi
	call	BitVector_Empty
.LVL154:
	.loc 1 403 0
	movq	%r13, %rsi
	movq	%r12, %rdi
	call	BitVector_Lexicompare
	movq	result(%rip), %rdi
	xorl	%esi, %esi
	testl	%eax, %eax
	setg	%sil
	call	BitVector_LSB
	jmp	.L122
.LVL155:
.L114:
	.loc 1 394 0
	movq	result(%rip), %rdi
	call	BitVector_Empty
.LVL156:
	.loc 1 395 0
	movq	%r13, %rsi
	movq	%r12, %rdi
	call	BitVector_equal
	movl	%eax, %esi
	jmp	.L145
.LVL157:
.L119:
	.loc 1 421 0
	movl	$.LC25, %edx
	movl	$.LC24, %esi
	movq	%r15, %rdi
	xorl	%eax, %eax
.LVL158:
	call	yasm__error
	jmp	.L122
.LVL159:
.L120:
	.loc 1 424 0
	movl	$.LC26, %edx
	movl	$.LC24, %esi
	movq	%r15, %rdi
	xorl	%eax, %eax
.LVL160:
	call	yasm__error
	jmp	.L122
.LVL161:
.L101:
	.loc 1 347 0
	movq	result(%rip), %rdi
	movq	%r12, %rsi
	call	BitVector_Negate
.LVL162:
	jmp	.L122
.LVL163:
.L93:
	.loc 1 427 0
	movq	result(%rip), %rdi
	testq	%rdi, %rdi
	je	.L122
	.loc 1 428 0
	movq	%r12, %rsi
	call	BitVector_Copy
.LVL164:
	jmp	.L122
.LVL165:
.L110:
	.loc 1 385 0
	movq	result(%rip), %rdi
	call	BitVector_Empty
.LVL166:
	.loc 1 386 0
	movq	%r12, %rdi
	call	BitVector_is_empty
	testl	%eax, %eax
	je	.L151
	xorl	%esi, %esi
	.p2align 4,,2
	jmp	.L145
.LVL167:
.L96:
	.loc 1 330 0
	movq	result(%rip), %rdi
	movq	%r13, %rdx
	movq	%r12, %rsi
	call	BitVector_Multiply
.LVL168:
	jmp	.L122
.LVL169:
.L94:
	.loc 1 324 0
	movq	result(%rip), %rdi
	leaq	20(%rsp), %rcx
	movq	%r13, %rdx
	movq	%r12, %rsi
	call	BitVector_add
.LVL170:
	jmp	.L122
.LVL171:
.L95:
	.loc 1 327 0
	movq	result(%rip), %rdi
	leaq	20(%rsp), %rcx
	movq	%r13, %rdx
	movq	%r12, %rsi
	call	BitVector_sub
.LVL172:
	jmp	.L122
.LVL173:
.L103:
	.loc 1 353 0
	movq	result(%rip), %rdi
	movq	%r13, %rdx
	movq	%r12, %rsi
	call	Set_Union
.LVL174:
	jmp	.L122
.LVL175:
.L104:
	.loc 1 356 0
	movq	result(%rip), %rdi
	movq	%r13, %rdx
	movq	%r12, %rsi
	call	Set_Intersection
.LVL176:
	jmp	.L122
.LVL177:
.L105:
	.loc 1 359 0
	movq	result(%rip), %rdi
	movq	%r13, %rdx
	movq	%r12, %rsi
	call	Set_ExclusiveOr
.LVL178:
	jmp	.L122
.LVL179:
.L106:
	.loc 1 362 0
	movq	result(%rip), %rdi
	movq	%r12, %rsi
	movq	%r13, %rdx
	call	Set_Union
.LVL180:
	.loc 1 363 0
	movq	result(%rip), %rdi
	movq	%rdi, %rsi
	call	Set_Complement
	jmp	.L122
.LVL181:
.L107:
	.loc 1 366 0
	movl	8(%rbx), %edx
	testl	%edx, %edx
	jne	.L125
	.loc 1 367 0
	movq	result(%rip), %rdi
	movq	%r12, %rsi
	call	BitVector_Copy
.LVL182:
	.loc 1 368 0
	movq	(%rbx), %rax
	movq	result(%rip), %rdi
	movl	%eax, %esi
	call	BitVector_Move_Left
	jmp	.L122
.LVL183:
.L108:
	.loc 1 373 0
	movl	8(%rbx), %eax
.LVL184:
	testl	%eax, %eax
	jne	.L125
	.loc 1 374 0
	movq	result(%rip), %rdi
	movq	%r12, %rsi
	call	BitVector_Copy
	.loc 1 375 0
	movq	(%rbx), %rax
	movq	result(%rip), %rdi
	movl	%eax, %esi
	call	BitVector_Move_Right
	jmp	.L122
.LVL185:
.L109:
	.loc 1 380 0
	movq	result(%rip), %rdi
	call	BitVector_Empty
.LVL186:
	.loc 1 381 0
	movq	%r12, %rdi
	call	BitVector_is_empty
	testl	%eax, %eax
	jne	.L152
	movl	$1, %esi
	.p2align 4,,2
	jmp	.L145
.LVL187:
.L115:
	.loc 1 406 0
	movq	result(%rip), %rdi
	call	BitVector_Empty
.LVL188:
	.loc 1 407 0
	movq	%r13, %rsi
	movq	%r12, %rdi
	call	BitVector_Lexicompare
	movq	result(%rip), %rdi
	xorl	%esi, %esi
	testl	%eax, %eax
	setle	%sil
	call	BitVector_LSB
	jmp	.L122
.LVL189:
.L116:
	.loc 1 410 0
	movq	result(%rip), %rdi
	call	BitVector_Empty
.LVL190:
	.loc 1 411 0
	movq	%r13, %rsi
	movq	%r12, %rdi
	call	BitVector_Lexicompare
	movq	result(%rip), %rdi
	movl	%eax, %esi
	notl	%esi
	shrl	$31, %esi
	call	BitVector_LSB
	jmp	.L122
.LVL191:
.L117:
	.loc 1 414 0
	movq	result(%rip), %rdi
	call	BitVector_Empty
.LVL192:
	.loc 1 415 0
	movq	%r13, %rsi
	movq	%r12, %rdi
	call	BitVector_equal
	movq	result(%rip), %rdi
	xorl	%esi, %esi
	testl	%eax, %eax
	sete	%sil
	call	BitVector_LSB
	jmp	.L122
.LVL193:
.L118:
	.loc 1 418 0
	movl	$.LC23, %edx
	movl	$.LC24, %esi
	movq	%r15, %rdi
	xorl	%eax, %eax
.LVL194:
	call	yasm__error
	jmp	.L122
.LVL195:
.L102:
	.loc 1 350 0
	movq	result(%rip), %rdi
	movq	%r12, %rsi
	call	Set_Complement
.LVL196:
	jmp	.L122
.L152:
	.loc 1 381 0
	movq	%r13, %rdi
	call	BitVector_is_empty
	xorl	%esi, %esi
	testl	%eax, %eax
	.p2align 4,,2
	jne	.L145
	movl	$1, %esi
	jmp	.L145
.L151:
	.loc 1 386 0
	movq	%r13, %rdi
	call	BitVector_is_empty
	testl	%eax, %eax
	movl	$1, %esi
	je	.L145
	xorl	%esi, %esi
	jmp	.L145
.LVL197:
.L125:
	.loc 1 377 0
	movq	result(%rip), %rdi
	call	BitVector_Empty
	jmp	.L122
.LFE36:
	.size	yasm_intnum_calc, .-yasm_intnum_calc
	.p2align 4,,15
.globl yasm_intnum_zero
	.type	yasm_intnum_zero, @function
yasm_intnum_zero:
.LFB37:
	.loc 1 454 0
.LVL198:
	pushq	%rbx
.LCFI36:
.LVL199:
	.loc 1 455 0
	cmpl	$1, 8(%rdi)
	.loc 1 454 0
	movq	%rdi, %rbx
	.loc 1 455 0
	je	.L157
.LVL200:
	.loc 1 459 0
	movq	$0, (%rbx)
	.loc 1 460 0
	popq	%rbx
.LVL201:
	ret
.LVL202:
	.p2align 4,,7
.L157:
	.loc 1 456 0
	movq	(%rdi), %rdi
	call	BitVector_Destroy
	.loc 1 457 0
	movl	$0, 8(%rbx)
	.loc 1 459 0
	movq	$0, (%rbx)
	.loc 1 460 0
	popq	%rbx
.LVL203:
	ret
.LFE37:
	.size	yasm_intnum_zero, .-yasm_intnum_zero
	.p2align 4,,15
.globl yasm_intnum_is_zero
	.type	yasm_intnum_is_zero, @function
yasm_intnum_is_zero:
.LFB38:
	.loc 1 464 0
.LVL204:
	.loc 1 465 0
	movl	8(%rdi), %ecx
	testl	%ecx, %ecx
	jne	.L159
	cmpq	$0, (%rdi)
	movl	$1, %eax
	jne	.L159
	.loc 1 466 0
	rep ; ret
	.p2align 4,,7
.L159:
	.loc 1 465 0
	xorl	%eax, %eax
	.loc 1 466 0
	ret
.LFE38:
	.size	yasm_intnum_is_zero, .-yasm_intnum_is_zero
	.p2align 4,,15
.globl yasm_intnum_is_pos1
	.type	yasm_intnum_is_pos1, @function
yasm_intnum_is_pos1:
.LFB39:
	.loc 1 470 0
.LVL205:
	.loc 1 471 0
	movl	8(%rdi), %esi
	testl	%esi, %esi
	jne	.L165
	cmpq	$1, (%rdi)
	movl	$1, %eax
	je	.L168
.L165:
	xorl	%eax, %eax
.L168:
	.loc 1 472 0
	rep ; ret
.LFE39:
	.size	yasm_intnum_is_pos1, .-yasm_intnum_is_pos1
	.p2align 4,,15
.globl yasm_intnum_is_neg1
	.type	yasm_intnum_is_neg1, @function
yasm_intnum_is_neg1:
.LFB40:
	.loc 1 476 0
.LVL206:
	subq	$8, %rsp
.LCFI37:
.LVL207:
	.loc 1 477 0
	cmpl	$1, 8(%rdi)
	je	.L177
.L171:
	xorl	%edx, %edx
	.loc 1 478 0
	addq	$8, %rsp
.LVL208:
	movl	%edx, %eax
	ret
.LVL209:
	.p2align 4,,7
.L177:
	.loc 1 477 0
	movq	(%rdi), %rdi
.LVL210:
	call	BitVector_is_full
	testl	%eax, %eax
	movl	$1, %edx
	je	.L171
	.loc 1 478 0
	movl	%edx, %eax
	addq	$8, %rsp
.LVL211:
	ret
.LFE40:
	.size	yasm_intnum_is_neg1, .-yasm_intnum_is_neg1
	.p2align 4,,15
.globl yasm_intnum_sign
	.type	yasm_intnum_sign, @function
yasm_intnum_sign:
.LFB41:
	.loc 1 482 0
.LVL212:
	.loc 1 483 0
	movl	8(%rdi), %r8d
	testl	%r8d, %r8d
	jne	.L186
	.loc 1 484 0
	xorl	%eax, %eax
	cmpq	$0, (%rdi)
	setne	%al
	.loc 1 490 0
	ret
	.p2align 4,,7
.L186:
	.loc 1 489 0
	movq	(%rdi), %rdi
.LVL213:
	jmp	BitVector_Sign
.LFE41:
	.size	yasm_intnum_sign, .-yasm_intnum_sign
	.section	.rodata.str1.1
.LC28:
	.string	"unknown intnum type"
	.text
	.p2align 4,,15
.globl yasm_intnum_get_uint
	.type	yasm_intnum_get_uint, @function
yasm_intnum_get_uint:
.LFB42:
	.loc 1 494 0
.LVL214:
	subq	$8, %rsp
.LCFI38:
.LVL215:
	.loc 1 495 0
	movl	8(%rdi), %eax
	testl	%eax, %eax
	je	.L189
	decl	%eax
	je	.L194
	.loc 1 501 0
	movl	$.LC28, %edx
	movl	$501, %esi
	movl	$.LC22, %edi
.LVL216:
	call	*yasm_internal_error_(%rip)
	xorl	%eax, %eax
	.loc 1 505 0
	addq	$8, %rsp
.LVL217:
	ret
.LVL218:
	.p2align 4,,7
.L189:
	.loc 1 497 0
	movq	(%rdi), %rax
	.loc 1 505 0
	addq	$8, %rsp
.LVL219:
	ret
.LVL220:
	.p2align 4,,7
.L194:
	.loc 1 499 0
	movq	(%rdi), %rdi
.LVL221:
	xorl	%edx, %edx
	movl	$32, %esi
	.loc 1 505 0
	addq	$8, %rsp
.LVL222:
	.loc 1 499 0
	jmp	BitVector_Chunk_Read
.LFE42:
	.size	yasm_intnum_get_uint, .-yasm_intnum_get_uint
	.p2align 4,,15
.globl yasm_intnum_get_int
	.type	yasm_intnum_get_int, @function
yasm_intnum_get_int:
.LFB43:
	.loc 1 509 0
.LVL223:
	pushq	%rbx
.LCFI39:
.LVL224:
	.loc 1 510 0
	movl	8(%rdi), %eax
	.loc 1 509 0
	movq	%rdi, %rbx
	.loc 1 510 0
	testl	%eax, %eax
	je	.L197
.LVL225:
	decl	%eax
	je	.L208
	.loc 1 534 0
	movl	$.LC28, %edx
	movl	$534, %esi
	movl	$.LC22, %edi
	call	*yasm_internal_error_(%rip)
	xorl	%eax, %eax
.LVL226:
.L201:
	.loc 1 538 0
	popq	%rbx
.LVL227:
	ret
.LVL228:
	.p2align 4,,7
.L197:
	.loc 1 513 0
	movq	(%rdi), %rax
	testl	%eax, %eax
	jns	.L201
.L199:
	.loc 1 538 0
	popq	%rbx
.LVL229:
	.loc 1 536 0
	movabsq	$9223372036854775807, %rax
	.loc 1 538 0
	.p2align 4,,2
	ret
.LVL230:
	.p2align 4,,7
.L208:
	.loc 1 515 0
	movq	(%rdi), %rdi
	call	BitVector_msb_
	testl	%eax, %eax
	.p2align 4,,2
	je	.L199
.LBB2:
	.loc 1 521 0
	movq	(%rbx), %rsi
	movq	conv_bv(%rip), %rdi
	call	BitVector_Negate
	.loc 1 522 0
	movq	conv_bv(%rip), %rdi
	call	Set_Max
	cmpq	$31, %rax
	jle	.L209
.LVL231:
.L203:
.LBE2:
	.loc 1 538 0
	popq	%rbx
.LVL232:
	.loc 1 536 0
	movabsq	$-9223372036854775808, %rax
.LVL233:
	.loc 1 538 0
	ret
.LVL234:
.L209:
.LBB3:
	.loc 1 526 0
	movq	conv_bv(%rip), %rdi
	xorl	%edx, %edx
	movl	$32, %esi
	call	BitVector_Chunk_Read
.LVL235:
	.loc 1 528 0
	testl	%eax, %eax
	js	.L203
.LBE3:
	.loc 1 538 0
	popq	%rbx
.LVL236:
.LBB4:
	.loc 1 528 0
	negq	%rax
.LVL237:
.LBE4:
	.loc 1 538 0
	ret
.LFE43:
	.size	yasm_intnum_get_int, .-yasm_intnum_get_int
	.p2align 4,,15
.globl yasm_intnum_check_size
	.type	yasm_intnum_check_size, @function
yasm_intnum_check_size:
.LFB45:
	.loc 1 610 0
.LVL238:
	movq	%rbx, -48(%rsp)
.LCFI40:
	movq	%r13, -24(%rsp)
.LCFI41:
	movq	%rdi, %rbx
	movq	%r14, -16(%rsp)
.LCFI42:
	movq	%r15, -8(%rsp)
.LCFI43:
	movq	%rsi, %r14
	movq	%rbp, -40(%rsp)
.LCFI44:
	movq	%r12, -32(%rsp)
.LCFI45:
	subq	$56, %rsp
.LCFI46:
.LVL239:
	.loc 1 614 0
	cmpl	$1, 8(%rdi)
	.loc 1 610 0
	movq	%rdx, %r13
	movl	%ecx, %r15d
	.loc 1 614 0
	je	.L230
.LVL240:
	.loc 1 621 0
	movq	conv_bv(%rip), %rbp
.LVL241:
	.loc 1 622 0
	movq	%rbp, %rdi
.LVL242:
	call	BitVector_Empty
	.loc 1 623 0
	movq	(%rbx), %rcx
	xorl	%edx, %edx
	movl	$32, %esi
	movq	%rbp, %rdi
	call	BitVector_Chunk_Store
.LVL243:
.L215:
	.loc 1 626 0
	cmpq	$127, %r14
	movl	$1, %eax
.LVL244:
	ja	.L218
	.loc 1 629 0
	testq	%r13, %r13
	jne	.L231
.L219:
	.loc 1 635 0
	testl	%r15d, %r15d
	.p2align 4,,3
	jle	.L223
	.loc 1 636 0
	movq	%rbp, %rdi
	.p2align 4,,5
	call	BitVector_msb_
.LVL245:
	testl	%eax, %eax
	.p2align 4,,2
	jne	.L232
	.loc 1 648 0
	xorl	%eax, %eax
	cmpl	$1, %r15d
	sete	%al
	subq	%rax, %r14
.LVL246:
.L223:
	.loc 1 650 0
	movq	%rbp, %rdi
	call	Set_Max
.LVL247:
	cmpq	%r14, %rax
	setl	%al
	movzbl	%al, %eax
.LVL248:
.L218:
	.loc 1 651 0
	movq	8(%rsp), %rbx
.LVL249:
	movq	16(%rsp), %rbp
.LVL250:
	movq	24(%rsp), %r12
	movq	32(%rsp), %r13
.LVL251:
	movq	40(%rsp), %r14
.LVL252:
	movq	48(%rsp), %r15
.LVL253:
	addq	$56, %rsp
.LVL254:
	ret
.LVL255:
	.p2align 4,,7
.L231:
.LBB5:
	.loc 1 630 0
	movq	%rbp, %rdi
	.loc 1 631 0
	xorl	%ebx, %ebx
.LVL256:
	.loc 1 630 0
	call	BitVector_msb_
.LVL257:
	movl	%eax, %r12d
	.p2align 4,,7
.L222:
	.loc 1 632 0
	movl	%r12d, %esi
	movq	%rbp, %rdi
	incq	%rbx
	call	BitVector_shift_right
	.loc 1 631 0
	cmpq	%rbx, %r13
	jne	.L222
	jmp	.L219
.LVL258:
	.p2align 4,,7
.L230:
.LBE5:
	.loc 1 615 0
	testq	%rdx, %rdx
	.p2align 4,,7
	jne	.L233
.LVL259:
	.loc 1 619 0
	movq	(%rdi), %rbp
.LVL260:
	.p2align 4,,7
	jmp	.L215
.LVL261:
.L232:
.LBB6:
	.loc 1 640 0
	movq	conv_bv(%rip), %rdi
	movq	%rbp, %rsi
	call	BitVector_Negate
	.loc 1 641 0
	movq	conv_bv(%rip), %rdi
	movq	%rdi, %rsi
	call	BitVector_dec
	.loc 1 642 0
	movq	conv_bv(%rip), %rdi
	call	Set_Max
	leaq	-1(%r14), %rdx
	cmpq	%rdx, %rax
	setl	%al
	movzbl	%al, %eax
.LVL262:
	jmp	.L218
.LVL263:
.L233:
.LBE6:
	.loc 1 616 0
	movq	conv_bv(%rip), %rbp
.LVL264:
	.loc 1 617 0
	movq	(%rdi), %rsi
	movq	%rbp, %rdi
.LVL265:
	call	BitVector_Copy
.LVL266:
	jmp	.L215
.LFE45:
	.size	yasm_intnum_check_size, .-yasm_intnum_check_size
	.section	.rodata.str1.1
.LC29:
	.string	"destination too large"
	.section	.rodata.str1.8
	.align 8
.LC30:
	.string	"value does not fit in %d bit field"
	.section	.rodata.str1.1
.LC31:
	.string	"big endian not implemented"
	.section	.rodata.str1.8
	.align 8
.LC32:
	.string	"misaligned value, truncating to boundary"
	.text
	.p2align 4,,15
.globl yasm_intnum_get_sized
	.type	yasm_intnum_get_sized, @function
yasm_intnum_get_sized:
.LFB44:
	.loc 1 544 0
.LVL267:
	movq	%rbx, -48(%rsp)
.LCFI47:
	movq	%r12, -32(%rsp)
.LCFI48:
	movq	%rdi, %rbx
	movq	%r13, -24(%rsp)
.LCFI49:
	movq	%r14, -16(%rsp)
.LCFI50:
	movl	%r8d, %r12d
	movq	%r15, -8(%rsp)
.LCFI51:
	movq	%rbp, -40(%rsp)
.LCFI52:
	.loc 1 548 0
	xorl	%r15d, %r15d
.LVL268:
	.loc 1 544 0
	subq	$88, %rsp
.LCFI53:
.LVL269:
	.loc 1 548 0
	testl	%r8d, %r8d
	.loc 1 544 0
	movq	%rdx, %r14
	movq	%rsi, 16(%rsp)
	movq	%rcx, 8(%rsp)
	movl	%r9d, 4(%rsp)
	.loc 1 545 0
	movq	op1static(%rip), %r13
.LVL270:
	.loc 1 548 0
	js	.L263
.LVL271:
.L237:
	.loc 1 552 0
	leaq	0(,%r14,8), %rax
	cmpq	$128, %rax
	ja	.L264
.LVL272:
.L238:
	.loc 1 556 0
	movl	96(%rsp), %ebp
	testl	%ebp, %ebp
	jne	.L265
.L240:
	.loc 1 561 0
	movl	4(%rsp), %r11d
	testl	%r11d, %r11d
	je	.L243
	.loc 1 563 0
	movl	$.LC31, %edx
	movl	$563, %esi
	movl	$.LC22, %edi
	call	*yasm_internal_error_(%rip)
	.loc 1 568 0
	cmpl	$1, 8(%rbx)
	je	.L266
.L246:
	.loc 1 571 0
	movq	op2static(%rip), %rbp
.LVL273:
	.loc 1 572 0
	movq	%rbp, %rdi
	call	BitVector_Empty
	.loc 1 573 0
	movq	(%rbx), %rcx
	xorl	%edx, %edx
	movl	$32, %esi
	movq	%rbp, %rdi
	call	BitVector_Chunk_Store
	.loc 1 577 0
	movl	96(%rsp), %r10d
	testq	%r15, %r15
	setne	%bl
.LVL274:
	testl	%r10d, %r10d
	setne	%al
	testb	%bl, %al
	jne	.L267
.L249:
	.loc 1 586 0
	testb	%bl, %bl
	movl	%r12d, %edx
	jne	.L268
.L254:
	.loc 1 594 0
	movl	8(%rsp), %r8d
	xorl	%ecx, %ecx
	movq	%rbp, %rsi
	movq	%r13, %rdi
	call	BitVector_Interval_Copy
	.loc 1 597 0
	leaq	36(%rsp), %rsi
	movq	%r13, %rdi
	call	BitVector_Block_Read
	.loc 1 598 0
	movl	4(%rsp), %r9d
	.loc 1 597 0
	movq	%rax, %rbx
.LVL275:
	.loc 1 598 0
	testl	%r9d, %r9d
	je	.L258
.LVL276:
	.loc 1 600 0
	movl	$.LC31, %edx
	movl	$600, %esi
	movl	$.LC22, %edi
	call	*yasm_internal_error_(%rip)
.L260:
	.loc 1 603 0
	movq	%rbx, %rdi
	call	*yasm_xfree(%rip)
	.loc 1 604 0
	movq	40(%rsp), %rbx
.LVL277:
	movq	48(%rsp), %rbp
.LVL278:
	movq	56(%rsp), %r12
.LVL279:
	movq	64(%rsp), %r13
.LVL280:
	movq	72(%rsp), %r14
.LVL281:
	movq	80(%rsp), %r15
.LVL282:
	addq	$88, %rsp
.LVL283:
	ret
.LVL284:
	.p2align 4,,7
.L243:
	.loc 1 565 0
	movq	16(%rsp), %rsi
	movl	%r14d, %edx
	movq	%r13, %rdi
	call	BitVector_Block_Store
	.loc 1 568 0
	cmpl	$1, 8(%rbx)
	jne	.L246
.L266:
	.loc 1 577 0
	movl	96(%rsp), %r10d
	testq	%r15, %r15
	.loc 1 569 0
	movq	(%rbx), %rbp
.LVL285:
	.loc 1 577 0
	setne	%bl
.LVL286:
	testl	%r10d, %r10d
	setne	%al
	testb	%bl, %al
	je	.L249
.L267:
	.loc 1 578 0
	movq	conv_bv(%rip), %rdi
	movq	%rbp, %rsi
	call	BitVector_Copy
	.loc 1 579 0
	movq	conv_bv(%rip), %rdi
	movl	$128, %esi
	subl	%r15d, %esi
	call	BitVector_Move_Left
	.loc 1 580 0
	movq	conv_bv(%rip), %rdi
	call	BitVector_is_empty
	testl	%eax, %eax
	jne	.L249
	.loc 1 581 0
	movq	104(%rsp), %rsi
	movl	$.LC32, %edx
	xorl	%edi, %edi
	call	yasm__warning
	jmp	.L249
.LVL287:
	.p2align 4,,7
.L258:
	.loc 1 602 0
	movq	16(%rsp), %rdi
	movq	%r14, %rdx
	movq	%rax, %rsi
	call	memcpy
	jmp	.L260
.LVL288:
	.p2align 4,,7
.L265:
	.loc 1 556 0
	movq	8(%rsp), %rsi
	movl	$2, %ecx
	movq	%r15, %rdx
	movq	%rbx, %rdi
	call	yasm_intnum_check_size
	testl	%eax, %eax
	jne	.L240
	.loc 1 557 0
	movq	8(%rsp), %rcx
	movq	104(%rsp), %rsi
	movl	$.LC30, %edx
	xorl	%edi, %edi
	call	yasm__warning
	jmp	.L240
.LVL289:
	.p2align 4,,7
.L264:
	.loc 1 553 0
	movl	$.LC29, %edx
	movl	$553, %esi
	movl	$.LC22, %edi
	call	*yasm_internal_error_(%rip)
.LVL290:
	jmp	.L238
.LVL291:
	.p2align 4,,7
.L268:
	.loc 1 587 0
	movq	%rbp, %rdi
	call	BitVector_msb_
	.loc 1 588 0
	testq	%r15, %r15
	.loc 1 587 0
	movl	%eax, %r12d
.LVL292:
	.loc 1 588 0
	.p2align 4,,2
	je	.L255
	xorl	%ebx, %ebx
	.p2align 4,,7
.L257:
	.loc 1 589 0
	movl	%r12d, %esi
	movq	%rbp, %rdi
	incq	%rbx
	call	BitVector_shift_right
	.loc 1 588 0
	cmpq	%rbx, %r15
	jne	.L257
.L255:
	xorl	%edx, %edx
	jmp	.L254
.LVL293:
	.p2align 4,,7
.L263:
	.loc 1 548 0
	movl	%r8d, %eax
	negl	%eax
	movslq	%eax,%r15
	jmp	.L237
.LFE44:
	.size	yasm_intnum_get_sized, .-yasm_intnum_get_sized
	.p2align 4,,15
.globl yasm_intnum_get_leb128
	.type	yasm_intnum_get_leb128, @function
yasm_intnum_get_leb128:
.LFB46:
	.loc 1 655 0
.LVL294:
	movq	%rbx, -40(%rsp)
.LCFI54:
	movq	%rbp, -32(%rsp)
.LCFI55:
	movq	%rdi, %rbx
	movq	%r13, -16(%rsp)
.LCFI56:
	movq	%r14, -8(%rsp)
.LCFI57:
	movl	%edx, %ebp
	movq	%r12, -24(%rsp)
.LCFI58:
	subq	$40, %rsp
.LCFI59:
.LVL295:
	.loc 1 661 0
	movl	8(%rdi), %eax
	.loc 1 655 0
	movq	%rsi, %r14
	.loc 1 656 0
	movq	op1static(%rip), %r13
.LVL296:
	.loc 1 661 0
	testl	%eax, %eax
	jne	.L270
.LVL297:
	cmpq	$0, (%rdi)
	je	.L290
.L272:
	.loc 1 670 0
	movq	%r13, %rdi
.LVL298:
	call	BitVector_Empty
	.loc 1 671 0
	movq	(%rbx), %rcx
	xorl	%edx, %edx
	movl	$32, %esi
	movq	%r13, %rdi
	call	BitVector_Chunk_Store
	.loc 1 674 0
	testl	%ebp, %ebp
	jne	.L291
.LVL299:
.L277:
	.loc 1 686 0
	movq	%r13, %rdi
	call	Set_Max
	leaq	1(%rax), %r12
.LVL300:
.L281:
	.loc 1 690 0
	xorl	%eax, %eax
	testq	%r12, %r12
	movq	%r14, %rbx
.LVL301:
	jne	.L292
.LVL302:
.L284:
	.loc 1 695 0
	andb	$127, -1(%rbx)
.LVL303:
.L274:
	.loc 1 697 0
	movq	(%rsp), %rbx
.LVL304:
	movq	8(%rsp), %rbp
.LVL305:
	movq	16(%rsp), %r12
.LVL306:
	movq	24(%rsp), %r13
.LVL307:
	movq	32(%rsp), %r14
.LVL308:
	addq	$40, %rsp
.LVL309:
	ret
.LVL310:
	.p2align 4,,7
.L270:
	.loc 1 667 0
	decl	%eax
	jne	.L272
	.loc 1 674 0
	testl	%ebp, %ebp
	.loc 1 668 0
	movq	(%rdi), %r13
	.loc 1 674 0
	je	.L277
.LVL311:
	.p2align 4,,7
.L291:
	.loc 1 676 0
	movq	%r13, %rdi
	call	BitVector_msb_
	testl	%eax, %eax
	jne	.L293
	.loc 1 682 0
	movq	%r13, %rdi
	.loc 1 690 0
	movq	%r14, %rbx
.LVL312:
	.loc 1 682 0
	call	Set_Max
	leaq	2(%rax), %r12
.LVL313:
	.loc 1 690 0
	xorl	%eax, %eax
	testq	%r12, %r12
	je	.L284
	.p2align 4,,7
.L292:
	xorl	%ebp, %ebp
.LVL314:
	.p2align 4,,7
.L285:
	.loc 1 691 0
	movl	%ebp, %edx
	movl	$7, %esi
	movq	%r13, %rdi
	call	BitVector_Chunk_Read
	.loc 1 690 0
	addq	$7, %rbp
	.loc 1 692 0
	orl	$-128, %eax
	movb	%al, (%rbx)
	.loc 1 693 0
	incq	%rbx
	.loc 1 690 0
	cmpq	%rbp, %r12
	ja	.L285
	movq	%rbx, %rax
	subq	%r14, %rax
	jmp	.L284
.LVL315:
	.p2align 4,,7
.L290:
	.loc 1 662 0
	movl	$1, %eax
	movb	$0, (%rsi)
	jmp	.L274
.LVL316:
.L293:
	.loc 1 678 0
	movq	conv_bv(%rip), %rdi
	movq	%r13, %rsi
	call	BitVector_Negate
	.loc 1 679 0
	movq	conv_bv(%rip), %rdi
	call	Set_Max
	leaq	2(%rax), %r12
.LVL317:
	jmp	.L281
.LFE46:
	.size	yasm_intnum_get_leb128, .-yasm_intnum_get_leb128
	.p2align 4,,15
.globl yasm_intnum_size_leb128
	.type	yasm_intnum_size_leb128, @function
yasm_intnum_size_leb128:
.LFB47:
	.loc 1 701 0
.LVL318:
	movq	%rbx, -24(%rsp)
.LCFI60:
	movq	%rbp, -16(%rsp)
.LCFI61:
	movq	%rdi, %rbx
	movq	%r12, -8(%rsp)
.LCFI62:
	subq	$24, %rsp
.LCFI63:
.LVL319:
	.loc 1 705 0
	movl	8(%rdi), %eax
	.loc 1 701 0
	movl	%esi, %r12d
	.loc 1 702 0
	movq	op1static(%rip), %rbp
.LVL320:
	.loc 1 705 0
	testl	%eax, %eax
	jne	.L295
.LVL321:
	cmpq	$0, (%rdi)
	movl	$1, %edx
	je	.L299
.L297:
	.loc 1 713 0
	movq	%rbp, %rdi
.LVL322:
	call	BitVector_Empty
	.loc 1 714 0
	movq	(%rbx), %rcx
	xorl	%edx, %edx
	movl	$32, %esi
	movq	%rbp, %rdi
	call	BitVector_Chunk_Store
	.loc 1 717 0
	testl	%r12d, %r12d
	je	.L302
.LVL323:
.L308:
	.loc 1 719 0
	movq	%rbp, %rdi
	call	BitVector_msb_
	testl	%eax, %eax
	.p2align 4,,2
	je	.L304
	.loc 1 721 0
	movq	conv_bv(%rip), %rdi
	movq	%rbp, %rsi
	call	BitVector_Negate
	.loc 1 722 0
	movq	conv_bv(%rip), %rdi
	call	Set_Max
	leaq	8(%rax), %rcx
	.p2align 4,,7
.L307:
	.loc 1 729 0
	movq	%rcx, %rax
	movabsq	$5270498306774157605, %rdx
	imulq	%rdx
	movq	%rcx, %rax
	sarq	$63, %rax
	sarq	%rdx
	subq	%rax, %rdx
.LVL324:
.L299:
	.loc 1 731 0
	movq	(%rsp), %rbx
.LVL325:
	movq	8(%rsp), %rbp
.LVL326:
	movq	%rdx, %rax
	movq	16(%rsp), %r12
.LVL327:
	addq	$24, %rsp
.LVL328:
	ret
.LVL329:
	.p2align 4,,7
.L295:
	.loc 1 710 0
	decl	%eax
	jne	.L297
	.loc 1 717 0
	testl	%r12d, %r12d
	.loc 1 711 0
	movq	(%rdi), %rbp
	.loc 1 717 0
	jne	.L308
.LVL330:
	.p2align 4,,7
.L302:
	.loc 1 729 0
	movq	%rbp, %rdi
	call	Set_Max
	leaq	7(%rax), %rcx
	jmp	.L307
	.p2align 4,,7
.L304:
	.loc 1 725 0
	movq	%rbp, %rdi
	call	Set_Max
	leaq	8(%rax), %rcx
	.p2align 4,,2
	jmp	.L307
.LFE47:
	.size	yasm_intnum_size_leb128, .-yasm_intnum_size_leb128
	.section	.rodata.str1.1
.LC33:
	.string	"0x%lx/%u"
.LC34:
	.string	"0x%s/%u"
	.text
	.p2align 4,,15
.globl yasm_intnum_print
	.type	yasm_intnum_print, @function
yasm_intnum_print:
.LFB48:
	.loc 1 735 0
.LVL331:
	movq	%rbp, -16(%rsp)
.LCFI64:
	movq	%r12, -8(%rsp)
.LCFI65:
	movq	%rdi, %rbp
	movq	%rbx, -24(%rsp)
.LCFI66:
	subq	$24, %rsp
.LCFI67:
.LVL332:
	.loc 1 738 0
	movl	8(%rdi), %eax
	.loc 1 735 0
	movq	%rsi, %r12
	.loc 1 738 0
	testl	%eax, %eax
	je	.L311
.LVL333:
	decl	%eax
	je	.L315
	.loc 1 748 0
	movq	(%rsp), %rbx
	movq	8(%rsp), %rbp
	movq	16(%rsp), %r12
.LVL334:
	addq	$24, %rsp
.LVL335:
	ret
.LVL336:
	.p2align 4,,7
.L311:
	.loc 1 740 0
	movzbl	12(%rdi), %ecx
	movq	(%rdi), %rdx
	movl	$.LC33, %esi
	movq	%r12, %rdi
.LVL337:
	.loc 1 748 0
	movq	(%rsp), %rbx
	movq	8(%rsp), %rbp
.LVL338:
	movq	16(%rsp), %r12
.LVL339:
	.loc 1 740 0
	xorl	%eax, %eax
	.loc 1 748 0
	addq	$24, %rsp
.LVL340:
	.loc 1 740 0
	jmp	fprintf
.LVL341:
	.p2align 4,,7
.L315:
	.loc 1 743 0
	movq	(%rdi), %rdi
.LVL342:
	call	BitVector_to_Hex
	.loc 1 744 0
	movzbl	12(%rbp), %ecx
	.loc 1 743 0
	movq	%rax, %rbx
.LVL343:
	.loc 1 744 0
	movq	%r12, %rdi
	movq	%rax, %rdx
	movl	$.LC34, %esi
	xorl	%eax, %eax
	call	fprintf
	.loc 1 745 0
	movq	%rbx, %rdi
	movq	yasm_xfree(%rip), %r11
	.loc 1 748 0
	movq	(%rsp), %rbx
.LVL344:
	movq	8(%rsp), %rbp
.LVL345:
	movq	16(%rsp), %r12
.LVL346:
	addq	$24, %rsp
.LVL347:
	.loc 1 745 0
	jmp	*%r11
.LVL348:
.LFE48:
	.size	yasm_intnum_print, .-yasm_intnum_print
	.section	.rodata.str1.1
.LC35:
	.string	"Test leb128_test: "
.LC36:
	.string	"-"
.LC37:
	.string	""
.LC38:
	.string	"un"
	.section	.rodata.str1.8
	.align 8
.LC39:
	.string	"%ssigned %s%s size() bad size: expected %lu, got %lu!"
	.align 8
.LC40:
	.string	"%ssigned %s%s get() bad size: expected %lu, got %lu!"
	.align 8
.LC41:
	.string	"%ssigned %s%s get() bad output!"
	.section	.rodata.str1.1
.LC42:
	.string	"%s ** F: %s\n"
.LC43:
	.string	" +%d-%d/%d %d%%\n%s"
	.text
	.p2align 4,,15
.globl main
	.type	main, @function
main:
.LFB50:
	.file 2 "libyasm/tests/leb128_test.c"
	.loc 2 134 0
.LVL349:
	pushq	%r15
.LCFI68:
.LVL350:
	pushq	%r14
.LCFI69:
.LVL351:
	pushq	%r13
.LCFI70:
.LVL352:
	pushq	%r12
.LCFI71:
.LVL353:
	pushq	%rbp
.LCFI72:
.LVL354:
	pushq	%rbx
.LCFI73:
.LVL355:
	subq	$168, %rsp
.LCFI74:
.LVL356:
	.loc 2 139 0
	call	BitVector_Boot
	testl	%eax, %eax
	movl	$1, %edx
	je	.L366
	.loc 2 159 0
	addq	$168, %rsp
.LVL357:
	movl	%edx, %eax
	popq	%rbx
.LVL358:
	popq	%rbp
.LVL359:
	popq	%r12
.LVL360:
	popq	%r13
.LVL361:
	popq	%r14
.LVL362:
	popq	%r15
.LVL363:
	ret
.LVL364:
.L366:
	leaq	48(%rsp), %rbp
	.loc 2 144 0
	xorl	%r14d, %r14d
.LVL365:
	movl	$tests+16, %r13d
	.loc 2 141 0
	call	yasm_intnum_initialize
	movl	$tests, %r15d
	.loc 2 144 0
	movl	$.LC35, %edi
	xorl	%eax, %eax
	.loc 2 143 0
	movb	$0, failed(%rip)
	.loc 2 144 0
	call	printf
	movl	$0, 44(%rsp)
.LVL366:
	movq	$tests+4, 32(%rsp)
	movq	$tests+8, 24(%rsp)
	jmp	.L320
.LVL367:
.L370:
.LBB7:
.LBB8:
.LBB9:
	.loc 2 98 0
	movq	%r12, %rdi
	call	yasm_intnum_destroy
	.loc 2 99 0
	movl	-12(%r13), %eax
	movl	$.LC37, %edx
	movl	$.LC36, %ecx
	movq	(%r13), %r9
	movq	-8(%r13), %r8
	movl	$.LC39, %esi
	testl	%eax, %eax
	movl	(%r15), %eax
	movq	%rbx, (%rsp)
	cmove	%rdx, %rcx
	testl	%eax, %eax
	movl	$.LC38, %eax
	cmove	%rax, %rdx
.LVL368:
.L365:
	.loc 2 110 0
	movl	$failmsg, %edi
	xorl	%eax, %eax
	movl	$1, %ebx
.LVL369:
	call	sprintf
	movl	$70, %edi
.L331:
.LBE9:
.LBE8:
	.loc 2 147 0
	call	putchar
	.loc 2 148 0
	movq	stdout(%rip), %rdi
	call	fflush
	.loc 2 149 0
	testl	%ebx, %ebx
	jne	.L367
.LBE7:
	.loc 2 145 0
	incl	%r14d
.LBB10:
	.loc 2 151 0
	addl	%ebx, 44(%rsp)
.LBE10:
	.loc 2 145 0
	addq	$32, 24(%rsp)
	addq	$32, %r15
	addq	$32, %r13
	addq	$32, 32(%rsp)
	cmpl	$16, %r14d
	je	.L368
.L320:
.LBB11:
.LBB12:
.LBB13:
	.loc 2 85 0
	movq	-8(%r13), %rdi
	call	yasm__xstrdup
	.loc 2 86 0
	xorl	%esi, %esi
	.loc 2 85 0
	movq	%rax, %rbx
.LVL370:
	.loc 2 86 0
	movq	%rax, %rdi
	call	yasm_intnum_create_hex
	.loc 2 91 0
	movq	%rbx, %rdi
	.loc 2 86 0
	movq	%rax, %r12
	.loc 2 91 0
	call	*yasm_xfree(%rip)
	.loc 2 93 0
	movl	-12(%r13), %edx
	testl	%edx, %edx
	jne	.L369
.L321:
	.loc 2 96 0
	movl	-16(%r13), %esi
	movq	%r12, %rdi
	call	yasm_intnum_size_leb128
.LVL371:
	.loc 2 97 0
	cmpq	(%r13), %rax
.LVL372:
	.loc 2 96 0
	movq	%rax, %rbx
.LVL373:
	.loc 2 97 0
	jne	.L370
	.loc 2 99 0
	movl	$1, %eax
	.p2align 4,,7
.L332:
	.loc 2 106 0
	movb	$-1, -1(%rax,%rbp)
	incq	%rax
	.loc 2 105 0
	cmpq	$101, %rax
	jne	.L332
	.loc 2 107 0
	movl	(%r15), %edx
	movq	%rbp, %rsi
	movq	%r12, %rdi
	call	yasm_intnum_get_leb128
.LVL374:
	.loc 2 108 0
	cmpq	(%r13), %rax
.LVL375:
	.loc 2 107 0
	movq	%rax, %rbx
.LVL376:
	.loc 2 108 0
	je	.L334
	.loc 2 109 0
	movq	%r12, %rdi
	call	yasm_intnum_destroy
	.loc 2 110 0
	movq	24(%rsp), %rax
	movl	$.LC37, %edx
	movq	(%r13), %r9
	movl	$.LC36, %ecx
	movl	$.LC40, %esi
	movq	(%rax), %r8
	movq	32(%rsp), %rax
	movl	(%rax), %eax
	testl	%eax, %eax
	movl	(%r15), %eax
	movq	%rbx, (%rsp)
	cmove	%rdx, %rcx
	testl	%eax, %eax
	movl	$.LC38, %eax
	cmove	%rax, %rdx
	jmp	.L365
.LVL377:
.L369:
	.loc 2 94 0
	xorl	%ecx, %ecx
	xorl	%edx, %edx
	movl	$8, %esi
	movq	%r12, %rdi
	call	yasm_intnum_calc
	jmp	.L321
.LVL378:
.L367:
.LBE13:
.LBE12:
	.loc 2 150 0
	xorl	%eax, %eax
	movl	$failmsg, %ecx
	movl	$failed, %edx
	movl	$.LC42, %esi
	movl	$failed, %edi
.LBE11:
	.loc 2 145 0
	incl	%r14d
.LBB14:
	.loc 2 150 0
	call	sprintf
.LBE14:
	.loc 2 145 0
	addq	$32, %r15
.LBB15:
	.loc 2 151 0
	addl	%ebx, 44(%rsp)
.LBE15:
	.loc 2 145 0
	addq	$32, 24(%rsp)
	addq	$32, %r13
	addq	$32, 32(%rsp)
	cmpl	$16, %r14d
	jne	.L320
.L368:
	.loc 2 154 0
	call	yasm_intnum_cleanup
	.loc 2 156 0
	movl	%r14d, %esi
	subl	44(%rsp), %esi
	movl	$100, %r12d
.LVL379:
	movl	44(%rsp), %edx
	movl	$failed, %r9d
	movl	$16, %ecx
	movl	$.LC43, %edi
	movl	%esi, %r8d
	imull	%r12d, %r8d
	leal	15(%r8), %eax
	testl	%r8d, %r8d
	cmovs	%eax, %r8d
	xorl	%eax, %eax
	sarl	$4, %r8d
	call	printf
	.loc 2 158 0
	xorl	%edx, %edx
	cmpl	$0, 44(%rsp)
	setne	%dl
	.loc 2 159 0
	addq	$168, %rsp
.LVL380:
	popq	%rbx
.LVL381:
	popq	%rbp
.LVL382:
	popq	%r12
.LVL383:
	popq	%r13
.LVL384:
	popq	%r14
.LVL385:
	popq	%r15
.LVL386:
	movl	%edx, %eax
	ret
.LVL387:
.L334:
.LBB16:
.LBB17:
.LBB18:
	.loc 2 117 0
	testq	%rax, %rax
	je	.L346
.LVL388:
	.loc 2 118 0
	movq	8(%r13), %rdi
	xorl	%esi, %esi
.LVL389:
	movl	$1, %ecx
.L342:
	movzbl	-1(%rdi,%rcx), %eax
.LVL390:
	cmpb	%al, -1(%rcx,%rbp)
	movl	$1, %eax
	cmovne	%eax, %esi
	.loc 2 117 0
	testl	%esi, %esi
	sete	%dl
	cmpq	%rcx, %rbx
	seta	%al
	incq	%rcx
	testb	%al, %dl
	jne	.L342
	.loc 2 121 0
	testl	%esi, %esi
	je	.L346
	.loc 2 122 0
	movq	%r12, %rdi
	.loc 2 123 0
	movl	$1, %ebx
.LVL391:
	.loc 2 122 0
	call	yasm_intnum_destroy
.LVL392:
	.loc 2 123 0
	movq	32(%rsp), %rax
	movl	$.LC37, %edx
	movl	$.LC36, %ecx
	movl	$failmsg, %edi
	movl	$.LC41, %esi
	movl	(%rax), %eax
	testl	%eax, %eax
	movl	(%r15), %eax
	cmove	%rdx, %rcx
	testl	%eax, %eax
	movl	$.LC38, %eax
	cmove	%rax, %rdx
	movq	24(%rsp), %rax
	movq	(%rax), %r8
	xorl	%eax, %eax
	call	sprintf
	movl	$70, %edi
	jmp	.L331
.LVL393:
.L346:
	.loc 2 128 0
	movq	%r12, %rdi
	xorl	%ebx, %ebx
.LVL394:
	call	yasm_intnum_destroy
.LVL395:
	movl	$46, %edi
	jmp	.L331
.LBE18:
.LBE17:
.LBE16:
.LFE50:
	.size	main, .-main
	.local	failmsg
	.comm	failmsg,100,32
	.local	failed
	.comm	failed,1000,32
	.local	from_dec_data
	.comm	from_dec_data,8,8
	.local	op2static
	.comm	op2static,8,8
	.local	op1static
	.comm	op1static,8,8
	.local	spare
	.comm	spare,8,8
	.local	result
	.comm	result,8,8
	.local	conv_bv
	.comm	conv_bv,8,8
	.section	.debug_frame,"",@progbits
.Lframe0:
	.long	.LECIE0-.LSCIE0
.LSCIE0:
	.long	0xffffffff
	.byte	0x1
	.string	""
	.uleb128 0x1
	.sleb128 -8
	.byte	0x10
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.byte	0x90
	.uleb128 0x1
	.align 8
.LECIE0:
.LSFDE0:
	.long	.LEFDE0-.LASFDE0
.LASFDE0:
	.long	.Lframe0
	.quad	.LFB25
	.quad	.LFE25-.LFB25
	.byte	0x4
	.long	.LCFI0-.LFB25
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE0:
.LSFDE2:
	.long	.LEFDE2-.LASFDE2
.LASFDE2:
	.long	.Lframe0
	.quad	.LFB26
	.quad	.LFE26-.LFB26
	.byte	0x4
	.long	.LCFI1-.LFB26
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE2:
.LSFDE4:
	.long	.LEFDE4-.LASFDE4
.LASFDE4:
	.long	.Lframe0
	.quad	.LFB27
	.quad	.LFE27-.LFB27
	.byte	0x4
	.long	.LCFI3-.LFB27
	.byte	0x86
	.uleb128 0x3
	.byte	0x83
	.uleb128 0x4
	.byte	0x4
	.long	.LCFI5-.LCFI3
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.align 8
.LEFDE4:
.LSFDE6:
	.long	.LEFDE6-.LASFDE6
.LASFDE6:
	.long	.Lframe0
	.quad	.LFB28
	.quad	.LFE28-.LFB28
	.byte	0x4
	.long	.LCFI6-.LFB28
	.byte	0x86
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI9-.LCFI6
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE6:
.LSFDE8:
	.long	.LEFDE8-.LASFDE8
.LASFDE8:
	.long	.Lframe0
	.quad	.LFB29
	.quad	.LFE29-.LFB29
	.byte	0x4
	.long	.LCFI10-.LFB29
	.byte	0x86
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI13-.LCFI10
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE8:
.LSFDE10:
	.long	.LEFDE10-.LASFDE10
.LASFDE10:
	.long	.Lframe0
	.quad	.LFB30
	.quad	.LFE30-.LFB30
	.byte	0x4
	.long	.LCFI14-.LFB30
	.byte	0x86
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI17-.LCFI14
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE10:
.LSFDE12:
	.long	.LEFDE12-.LASFDE12
.LASFDE12:
	.long	.Lframe0
	.quad	.LFB31
	.quad	.LFE31-.LFB31
	.byte	0x4
	.long	.LCFI18-.LFB31
	.byte	0x83
	.uleb128 0x5
	.byte	0x4
	.long	.LCFI21-.LCFI18
	.byte	0x8d
	.uleb128 0x2
	.byte	0x8c
	.uleb128 0x3
	.byte	0x86
	.uleb128 0x4
	.byte	0x4
	.long	.LCFI22-.LCFI21
	.byte	0xe
	.uleb128 0x30
	.align 8
.LEFDE12:
.LSFDE14:
	.long	.LEFDE14-.LASFDE14
.LASFDE14:
	.long	.Lframe0
	.quad	.LFB32
	.quad	.LFE32-.LFB32
	.byte	0x4
	.long	.LCFI23-.LFB32
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE14:
.LSFDE16:
	.long	.LEFDE16-.LASFDE16
.LASFDE16:
	.long	.Lframe0
	.quad	.LFB33
	.quad	.LFE33-.LFB33
	.byte	0x4
	.long	.LCFI24-.LFB33
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE16:
.LSFDE18:
	.long	.LEFDE18-.LASFDE18
.LASFDE18:
	.long	.Lframe0
	.quad	.LFB34
	.quad	.LFE34-.LFB34
	.byte	0x4
	.long	.LCFI25-.LFB34
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI27-.LCFI25
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x3
	.align 8
.LEFDE18:
.LSFDE20:
	.long	.LEFDE20-.LASFDE20
.LASFDE20:
	.long	.Lframe0
	.quad	.LFB35
	.quad	.LFE35-.LFB35
	.byte	0x4
	.long	.LCFI28-.LFB35
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE20:
.LSFDE22:
	.long	.LEFDE22-.LASFDE22
.LASFDE22:
	.long	.Lframe0
	.quad	.LFB36
	.quad	.LFE36-.LFB36
	.byte	0x4
	.long	.LCFI30-.LFB36
	.byte	0x86
	.uleb128 0x6
	.byte	0x83
	.uleb128 0x7
	.byte	0x4
	.long	.LCFI32-.LCFI30
	.byte	0x8f
	.uleb128 0x2
	.byte	0x8e
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI35-.LCFI32
	.byte	0xe
	.uleb128 0x50
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x5
	.align 8
.LEFDE22:
.LSFDE24:
	.long	.LEFDE24-.LASFDE24
.LASFDE24:
	.long	.Lframe0
	.quad	.LFB37
	.quad	.LFE37-.LFB37
	.byte	0x4
	.long	.LCFI36-.LFB37
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE24:
.LSFDE26:
	.long	.LEFDE26-.LASFDE26
.LASFDE26:
	.long	.Lframe0
	.quad	.LFB38
	.quad	.LFE38-.LFB38
	.align 8
.LEFDE26:
.LSFDE28:
	.long	.LEFDE28-.LASFDE28
.LASFDE28:
	.long	.Lframe0
	.quad	.LFB39
	.quad	.LFE39-.LFB39
	.align 8
.LEFDE28:
.LSFDE30:
	.long	.LEFDE30-.LASFDE30
.LASFDE30:
	.long	.Lframe0
	.quad	.LFB40
	.quad	.LFE40-.LFB40
	.byte	0x4
	.long	.LCFI37-.LFB40
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE30:
.LSFDE32:
	.long	.LEFDE32-.LASFDE32
.LASFDE32:
	.long	.Lframe0
	.quad	.LFB41
	.quad	.LFE41-.LFB41
	.align 8
.LEFDE32:
.LSFDE34:
	.long	.LEFDE34-.LASFDE34
.LASFDE34:
	.long	.Lframe0
	.quad	.LFB42
	.quad	.LFE42-.LFB42
	.byte	0x4
	.long	.LCFI38-.LFB42
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE34:
.LSFDE36:
	.long	.LEFDE36-.LASFDE36
.LASFDE36:
	.long	.Lframe0
	.quad	.LFB43
	.quad	.LFE43-.LFB43
	.byte	0x4
	.long	.LCFI39-.LFB43
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE36:
.LSFDE38:
	.long	.LEFDE38-.LASFDE38
.LASFDE38:
	.long	.Lframe0
	.quad	.LFB45
	.quad	.LFE45-.LFB45
	.byte	0x4
	.long	.LCFI41-.LFB45
	.byte	0x8d
	.uleb128 0x4
	.byte	0x83
	.uleb128 0x7
	.byte	0x4
	.long	.LCFI43-.LCFI41
	.byte	0x8f
	.uleb128 0x2
	.byte	0x8e
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI46-.LCFI43
	.byte	0xe
	.uleb128 0x40
	.byte	0x8c
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x6
	.align 8
.LEFDE38:
.LSFDE40:
	.long	.LEFDE40-.LASFDE40
.LASFDE40:
	.long	.Lframe0
	.quad	.LFB44
	.quad	.LFE44-.LFB44
	.byte	0x4
	.long	.LCFI48-.LFB44
	.byte	0x8c
	.uleb128 0x5
	.byte	0x83
	.uleb128 0x7
	.byte	0x4
	.long	.LCFI52-.LCFI48
	.byte	0x86
	.uleb128 0x6
	.byte	0x8f
	.uleb128 0x2
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8d
	.uleb128 0x4
	.byte	0x4
	.long	.LCFI53-.LCFI52
	.byte	0xe
	.uleb128 0x60
	.align 8
.LEFDE40:
.LSFDE42:
	.long	.LEFDE42-.LASFDE42
.LASFDE42:
	.long	.Lframe0
	.quad	.LFB46
	.quad	.LFE46-.LFB46
	.byte	0x4
	.long	.LCFI55-.LFB46
	.byte	0x86
	.uleb128 0x5
	.byte	0x83
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI59-.LCFI55
	.byte	0xe
	.uleb128 0x30
	.byte	0x8c
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x2
	.byte	0x8d
	.uleb128 0x3
	.align 8
.LEFDE42:
.LSFDE44:
	.long	.LEFDE44-.LASFDE44
.LASFDE44:
	.long	.Lframe0
	.quad	.LFB47
	.quad	.LFE47-.LFB47
	.byte	0x4
	.long	.LCFI61-.LFB47
	.byte	0x86
	.uleb128 0x3
	.byte	0x83
	.uleb128 0x4
	.byte	0x4
	.long	.LCFI63-.LCFI61
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.align 8
.LEFDE44:
.LSFDE46:
	.long	.LEFDE46-.LASFDE46
.LASFDE46:
	.long	.Lframe0
	.quad	.LFB48
	.quad	.LFE48-.LFB48
	.byte	0x4
	.long	.LCFI65-.LFB48
	.byte	0x8c
	.uleb128 0x2
	.byte	0x86
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI67-.LCFI65
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE46:
.LSFDE48:
	.long	.LEFDE48-.LASFDE48
.LASFDE48:
	.long	.Lframe0
	.quad	.LFB50
	.quad	.LFE50-.LFB50
	.byte	0x4
	.long	.LCFI68-.LFB50
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI69-.LCFI68
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI70-.LCFI69
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI71-.LCFI70
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI72-.LCFI71
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI73-.LCFI72
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI74-.LCFI73
	.byte	0xe
	.uleb128 0xe0
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE48:
	.section	.eh_frame,"a",@progbits
.Lframe1:
	.long	.LECIE1-.LSCIE1
.LSCIE1:
	.long	0x0
	.byte	0x1
	.string	""
	.uleb128 0x1
	.sleb128 -8
	.byte	0x10
	.byte	0xc
	.uleb128 0x7
	.uleb128 0x8
	.byte	0x90
	.uleb128 0x1
	.align 8
.LECIE1:
.LSFDE1:
	.long	.LEFDE1-.LASFDE1
.LASFDE1:
	.long	.LASFDE1-.Lframe1
	.quad	.LFB25
	.quad	.LFE25-.LFB25
	.byte	0x4
	.long	.LCFI0-.LFB25
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE1:
.LSFDE3:
	.long	.LEFDE3-.LASFDE3
.LASFDE3:
	.long	.LASFDE3-.Lframe1
	.quad	.LFB26
	.quad	.LFE26-.LFB26
	.byte	0x4
	.long	.LCFI1-.LFB26
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE3:
.LSFDE5:
	.long	.LEFDE5-.LASFDE5
.LASFDE5:
	.long	.LASFDE5-.Lframe1
	.quad	.LFB27
	.quad	.LFE27-.LFB27
	.byte	0x4
	.long	.LCFI3-.LFB27
	.byte	0x86
	.uleb128 0x3
	.byte	0x83
	.uleb128 0x4
	.byte	0x4
	.long	.LCFI5-.LCFI3
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.align 8
.LEFDE5:
.LSFDE7:
	.long	.LEFDE7-.LASFDE7
.LASFDE7:
	.long	.LASFDE7-.Lframe1
	.quad	.LFB28
	.quad	.LFE28-.LFB28
	.byte	0x4
	.long	.LCFI6-.LFB28
	.byte	0x86
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI9-.LCFI6
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE7:
.LSFDE9:
	.long	.LEFDE9-.LASFDE9
.LASFDE9:
	.long	.LASFDE9-.Lframe1
	.quad	.LFB29
	.quad	.LFE29-.LFB29
	.byte	0x4
	.long	.LCFI10-.LFB29
	.byte	0x86
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI13-.LCFI10
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE9:
.LSFDE11:
	.long	.LEFDE11-.LASFDE11
.LASFDE11:
	.long	.LASFDE11-.Lframe1
	.quad	.LFB30
	.quad	.LFE30-.LFB30
	.byte	0x4
	.long	.LCFI14-.LFB30
	.byte	0x86
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI17-.LCFI14
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE11:
.LSFDE13:
	.long	.LEFDE13-.LASFDE13
.LASFDE13:
	.long	.LASFDE13-.Lframe1
	.quad	.LFB31
	.quad	.LFE31-.LFB31
	.byte	0x4
	.long	.LCFI18-.LFB31
	.byte	0x83
	.uleb128 0x5
	.byte	0x4
	.long	.LCFI21-.LCFI18
	.byte	0x8d
	.uleb128 0x2
	.byte	0x8c
	.uleb128 0x3
	.byte	0x86
	.uleb128 0x4
	.byte	0x4
	.long	.LCFI22-.LCFI21
	.byte	0xe
	.uleb128 0x30
	.align 8
.LEFDE13:
.LSFDE15:
	.long	.LEFDE15-.LASFDE15
.LASFDE15:
	.long	.LASFDE15-.Lframe1
	.quad	.LFB32
	.quad	.LFE32-.LFB32
	.byte	0x4
	.long	.LCFI23-.LFB32
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE15:
.LSFDE17:
	.long	.LEFDE17-.LASFDE17
.LASFDE17:
	.long	.LASFDE17-.Lframe1
	.quad	.LFB33
	.quad	.LFE33-.LFB33
	.byte	0x4
	.long	.LCFI24-.LFB33
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE17:
.LSFDE19:
	.long	.LEFDE19-.LASFDE19
.LASFDE19:
	.long	.LASFDE19-.Lframe1
	.quad	.LFB34
	.quad	.LFE34-.LFB34
	.byte	0x4
	.long	.LCFI25-.LFB34
	.byte	0x86
	.uleb128 0x2
	.byte	0x4
	.long	.LCFI27-.LCFI25
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x3
	.align 8
.LEFDE19:
.LSFDE21:
	.long	.LEFDE21-.LASFDE21
.LASFDE21:
	.long	.LASFDE21-.Lframe1
	.quad	.LFB35
	.quad	.LFE35-.LFB35
	.byte	0x4
	.long	.LCFI28-.LFB35
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE21:
.LSFDE23:
	.long	.LEFDE23-.LASFDE23
.LASFDE23:
	.long	.LASFDE23-.Lframe1
	.quad	.LFB36
	.quad	.LFE36-.LFB36
	.byte	0x4
	.long	.LCFI30-.LFB36
	.byte	0x86
	.uleb128 0x6
	.byte	0x83
	.uleb128 0x7
	.byte	0x4
	.long	.LCFI32-.LCFI30
	.byte	0x8f
	.uleb128 0x2
	.byte	0x8e
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI35-.LCFI32
	.byte	0xe
	.uleb128 0x50
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8c
	.uleb128 0x5
	.align 8
.LEFDE23:
.LSFDE25:
	.long	.LEFDE25-.LASFDE25
.LASFDE25:
	.long	.LASFDE25-.Lframe1
	.quad	.LFB37
	.quad	.LFE37-.LFB37
	.byte	0x4
	.long	.LCFI36-.LFB37
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE25:
.LSFDE27:
	.long	.LEFDE27-.LASFDE27
.LASFDE27:
	.long	.LASFDE27-.Lframe1
	.quad	.LFB38
	.quad	.LFE38-.LFB38
	.align 8
.LEFDE27:
.LSFDE29:
	.long	.LEFDE29-.LASFDE29
.LASFDE29:
	.long	.LASFDE29-.Lframe1
	.quad	.LFB39
	.quad	.LFE39-.LFB39
	.align 8
.LEFDE29:
.LSFDE31:
	.long	.LEFDE31-.LASFDE31
.LASFDE31:
	.long	.LASFDE31-.Lframe1
	.quad	.LFB40
	.quad	.LFE40-.LFB40
	.byte	0x4
	.long	.LCFI37-.LFB40
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE31:
.LSFDE33:
	.long	.LEFDE33-.LASFDE33
.LASFDE33:
	.long	.LASFDE33-.Lframe1
	.quad	.LFB41
	.quad	.LFE41-.LFB41
	.align 8
.LEFDE33:
.LSFDE35:
	.long	.LEFDE35-.LASFDE35
.LASFDE35:
	.long	.LASFDE35-.Lframe1
	.quad	.LFB42
	.quad	.LFE42-.LFB42
	.byte	0x4
	.long	.LCFI38-.LFB42
	.byte	0xe
	.uleb128 0x10
	.align 8
.LEFDE35:
.LSFDE37:
	.long	.LEFDE37-.LASFDE37
.LASFDE37:
	.long	.LASFDE37-.Lframe1
	.quad	.LFB43
	.quad	.LFE43-.LFB43
	.byte	0x4
	.long	.LCFI39-.LFB43
	.byte	0xe
	.uleb128 0x10
	.byte	0x83
	.uleb128 0x2
	.align 8
.LEFDE37:
.LSFDE39:
	.long	.LEFDE39-.LASFDE39
.LASFDE39:
	.long	.LASFDE39-.Lframe1
	.quad	.LFB45
	.quad	.LFE45-.LFB45
	.byte	0x4
	.long	.LCFI41-.LFB45
	.byte	0x8d
	.uleb128 0x4
	.byte	0x83
	.uleb128 0x7
	.byte	0x4
	.long	.LCFI43-.LCFI41
	.byte	0x8f
	.uleb128 0x2
	.byte	0x8e
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI46-.LCFI43
	.byte	0xe
	.uleb128 0x40
	.byte	0x8c
	.uleb128 0x5
	.byte	0x86
	.uleb128 0x6
	.align 8
.LEFDE39:
.LSFDE41:
	.long	.LEFDE41-.LASFDE41
.LASFDE41:
	.long	.LASFDE41-.Lframe1
	.quad	.LFB44
	.quad	.LFE44-.LFB44
	.byte	0x4
	.long	.LCFI48-.LFB44
	.byte	0x8c
	.uleb128 0x5
	.byte	0x83
	.uleb128 0x7
	.byte	0x4
	.long	.LCFI52-.LCFI48
	.byte	0x86
	.uleb128 0x6
	.byte	0x8f
	.uleb128 0x2
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8d
	.uleb128 0x4
	.byte	0x4
	.long	.LCFI53-.LCFI52
	.byte	0xe
	.uleb128 0x60
	.align 8
.LEFDE41:
.LSFDE43:
	.long	.LEFDE43-.LASFDE43
.LASFDE43:
	.long	.LASFDE43-.Lframe1
	.quad	.LFB46
	.quad	.LFE46-.LFB46
	.byte	0x4
	.long	.LCFI55-.LFB46
	.byte	0x86
	.uleb128 0x5
	.byte	0x83
	.uleb128 0x6
	.byte	0x4
	.long	.LCFI59-.LCFI55
	.byte	0xe
	.uleb128 0x30
	.byte	0x8c
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x2
	.byte	0x8d
	.uleb128 0x3
	.align 8
.LEFDE43:
.LSFDE45:
	.long	.LEFDE45-.LASFDE45
.LASFDE45:
	.long	.LASFDE45-.Lframe1
	.quad	.LFB47
	.quad	.LFE47-.LFB47
	.byte	0x4
	.long	.LCFI61-.LFB47
	.byte	0x86
	.uleb128 0x3
	.byte	0x83
	.uleb128 0x4
	.byte	0x4
	.long	.LCFI63-.LCFI61
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.align 8
.LEFDE45:
.LSFDE47:
	.long	.LEFDE47-.LASFDE47
.LASFDE47:
	.long	.LASFDE47-.Lframe1
	.quad	.LFB48
	.quad	.LFE48-.LFB48
	.byte	0x4
	.long	.LCFI65-.LFB48
	.byte	0x8c
	.uleb128 0x2
	.byte	0x86
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI67-.LCFI65
	.byte	0xe
	.uleb128 0x20
	.byte	0x83
	.uleb128 0x4
	.align 8
.LEFDE47:
.LSFDE49:
	.long	.LEFDE49-.LASFDE49
.LASFDE49:
	.long	.LASFDE49-.Lframe1
	.quad	.LFB50
	.quad	.LFE50-.LFB50
	.byte	0x4
	.long	.LCFI68-.LFB50
	.byte	0xe
	.uleb128 0x10
	.byte	0x4
	.long	.LCFI69-.LCFI68
	.byte	0xe
	.uleb128 0x18
	.byte	0x4
	.long	.LCFI70-.LCFI69
	.byte	0xe
	.uleb128 0x20
	.byte	0x4
	.long	.LCFI71-.LCFI70
	.byte	0xe
	.uleb128 0x28
	.byte	0x4
	.long	.LCFI72-.LCFI71
	.byte	0xe
	.uleb128 0x30
	.byte	0x4
	.long	.LCFI73-.LCFI72
	.byte	0xe
	.uleb128 0x38
	.byte	0x4
	.long	.LCFI74-.LCFI73
	.byte	0xe
	.uleb128 0xe0
	.byte	0x83
	.uleb128 0x7
	.byte	0x86
	.uleb128 0x6
	.byte	0x8c
	.uleb128 0x5
	.byte	0x8d
	.uleb128 0x4
	.byte	0x8e
	.uleb128 0x3
	.byte	0x8f
	.uleb128 0x2
	.align 8
.LEFDE49:
	.file 3 "./libyasm/coretype.h"
	.file 4 "./libyasm/bitvect.h"
	.file 5 "/usr/lib/gcc/x86_64-linux-gnu/4.0.2/include/stddef.h"
	.file 6 "/usr/include/stdio.h"
	.file 7 "/usr/include/libio.h"
	.file 8 "/usr/include/bits/types.h"
	.file 9 "./libyasm/errwarn.h"
	.text
.Letext0:
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST0:
	.quad	.LVL0-.Ltext0
	.quad	.LVL1-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL1-.Ltext0
	.quad	.LVL2-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL2-.Ltext0
	.quad	.LFE25-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	0x0
	.quad	0x0
.LLST1:
	.quad	.LVL3-.Ltext0
	.quad	.LVL4-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL4-.Ltext0
	.quad	.LVL5-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL5-.Ltext0
	.quad	.LFE26-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	0x0
	.quad	0x0
.LLST2:
	.quad	.LVL6-.Ltext0
	.quad	.LVL8-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL8-.Ltext0
	.quad	.LVL16-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL16-.Ltext0
	.quad	.LVL17-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL17-.Ltext0
	.quad	.LVL21-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL21-.Ltext0
	.quad	.LVL22-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL22-.Ltext0
	.quad	.LFE27-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST3:
	.quad	.LVL6-.Ltext0
	.quad	.LVL7-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL7-.Ltext0
	.quad	.LVL13-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL17-.Ltext0
	.quad	.LVL18-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL22-.Ltext0
	.quad	.LFE27-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST4:
	.quad	.LVL6-.Ltext0
	.quad	.LVL9-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL9-.Ltext0
	.quad	.LVL15-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL17-.Ltext0
	.quad	.LVL20-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL22-.Ltext0
	.quad	.LFE27-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	0x0
	.quad	0x0
.LLST5:
	.quad	.LVL10-.Ltext0
	.quad	.LVL11-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL12-.Ltext0
	.quad	.LVL14-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL17-.Ltext0
	.quad	.LVL19-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL22-.Ltext0
	.quad	.LFE27-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST6:
	.quad	.LVL23-.Ltext0
	.quad	.LVL25-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL25-.Ltext0
	.quad	.LVL31-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL31-.Ltext0
	.quad	.LVL32-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL32-.Ltext0
	.quad	.LVL36-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL36-.Ltext0
	.quad	.LVL37-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL37-.Ltext0
	.quad	.LFE28-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST7:
	.quad	.LVL23-.Ltext0
	.quad	.LVL24-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL24-.Ltext0
	.quad	.LVL28-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL32-.Ltext0
	.quad	.LVL33-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL37-.Ltext0
	.quad	.LFE28-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST8:
	.quad	.LVL23-.Ltext0
	.quad	.LVL26-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL26-.Ltext0
	.quad	.LVL30-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL32-.Ltext0
	.quad	.LVL35-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL37-.Ltext0
	.quad	.LFE28-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	0x0
	.quad	0x0
.LLST9:
	.quad	.LVL27-.Ltext0
	.quad	.LVL29-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL32-.Ltext0
	.quad	.LVL34-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL37-.Ltext0
	.quad	.LFE28-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST10:
	.quad	.LVL38-.Ltext0
	.quad	.LVL40-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL40-.Ltext0
	.quad	.LVL46-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL46-.Ltext0
	.quad	.LVL47-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL47-.Ltext0
	.quad	.LVL51-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL51-.Ltext0
	.quad	.LVL52-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL52-.Ltext0
	.quad	.LFE29-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST11:
	.quad	.LVL38-.Ltext0
	.quad	.LVL39-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL39-.Ltext0
	.quad	.LVL43-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL47-.Ltext0
	.quad	.LVL48-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL52-.Ltext0
	.quad	.LFE29-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST12:
	.quad	.LVL38-.Ltext0
	.quad	.LVL41-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL41-.Ltext0
	.quad	.LVL45-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL47-.Ltext0
	.quad	.LVL50-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL52-.Ltext0
	.quad	.LFE29-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	0x0
	.quad	0x0
.LLST13:
	.quad	.LVL42-.Ltext0
	.quad	.LVL44-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL47-.Ltext0
	.quad	.LVL49-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL52-.Ltext0
	.quad	.LFE29-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST14:
	.quad	.LVL53-.Ltext0
	.quad	.LVL55-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL55-.Ltext0
	.quad	.LVL61-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL61-.Ltext0
	.quad	.LVL62-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL62-.Ltext0
	.quad	.LVL66-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL66-.Ltext0
	.quad	.LVL67-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL67-.Ltext0
	.quad	.LFE30-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST15:
	.quad	.LVL53-.Ltext0
	.quad	.LVL54-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL54-.Ltext0
	.quad	.LVL58-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL62-.Ltext0
	.quad	.LVL63-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL67-.Ltext0
	.quad	.LFE30-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST16:
	.quad	.LVL53-.Ltext0
	.quad	.LVL56-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL56-.Ltext0
	.quad	.LVL60-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL62-.Ltext0
	.quad	.LVL65-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL67-.Ltext0
	.quad	.LFE30-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	0x0
	.quad	0x0
.LLST17:
	.quad	.LVL57-.Ltext0
	.quad	.LVL59-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL62-.Ltext0
	.quad	.LVL64-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL67-.Ltext0
	.quad	.LFE30-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST18:
	.quad	.LVL68-.Ltext0
	.quad	.LVL69-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -40
	.quad	.LVL69-.Ltext0
	.quad	.LVL84-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL84-.Ltext0
	.quad	.LVL85-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -40
	.quad	.LVL85-.Ltext0
	.quad	.LFE31-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST19:
	.quad	.LVL68-.Ltext0
	.quad	.LVL70-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL70-.Ltext0
	.quad	.LVL81-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL85-.Ltext0
	.quad	.LVL87-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL88-.Ltext0
	.quad	.LFE31-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST20:
	.quad	.LVL68-.Ltext0
	.quad	.LVL71-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL71-.Ltext0
	.quad	.LVL82-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL85-.Ltext0
	.quad	.LVL86-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL88-.Ltext0
	.quad	.LFE31-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST21:
	.quad	.LVL72-.Ltext0
	.quad	.LVL83-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	.LVL85-.Ltext0
	.quad	.LFE31-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	0x0
	.quad	0x0
.LLST22:
	.quad	.LVL73-.Ltext0
	.quad	.LVL74-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL75-.Ltext0
	.quad	.LVL76-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL77-.Ltext0
	.quad	.LVL78-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL79-.Ltext0
	.quad	.LVL80-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL88-.Ltext0
	.quad	.LFE31-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	0x0
	.quad	0x0
.LLST23:
	.quad	.LVL89-.Ltext0
	.quad	.LVL90-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL90-.Ltext0
	.quad	.LVL92-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL92-.Ltext0
	.quad	.LFE32-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	0x0
	.quad	0x0
.LLST24:
	.quad	.LVL89-.Ltext0
	.quad	.LVL91-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL91-.Ltext0
	.quad	.LVL92-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST25:
	.quad	.LVL93-.Ltext0
	.quad	.LVL94-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL94-.Ltext0
	.quad	.LVL96-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL96-.Ltext0
	.quad	.LVL97-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL97-.Ltext0
	.quad	.LVL99-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL99-.Ltext0
	.quad	.LFE33-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	0x0
	.quad	0x0
.LLST26:
	.quad	.LVL93-.Ltext0
	.quad	.LVL95-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL95-.Ltext0
	.quad	.LVL96-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL96-.Ltext0
	.quad	.LVL97-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL97-.Ltext0
	.quad	.LVL98-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST27:
	.quad	.LVL100-.Ltext0
	.quad	.LVL102-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL102-.Ltext0
	.quad	.LVL106-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL106-.Ltext0
	.quad	.LVL107-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL107-.Ltext0
	.quad	.LVL110-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL110-.Ltext0
	.quad	.LVL111-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL111-.Ltext0
	.quad	.LVL114-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL114-.Ltext0
	.quad	.LFE34-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	0x0
	.quad	0x0
.LLST28:
	.quad	.LVL100-.Ltext0
	.quad	.LVL101-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL101-.Ltext0
	.quad	.LVL104-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL107-.Ltext0
	.quad	.LVL108-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL111-.Ltext0
	.quad	.LVL112-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST29:
	.quad	.LVL103-.Ltext0
	.quad	.LVL105-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL107-.Ltext0
	.quad	.LVL109-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL111-.Ltext0
	.quad	.LVL113-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST30:
	.quad	.LVL115-.Ltext0
	.quad	.LVL116-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL116-.Ltext0
	.quad	.LVL118-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL118-.Ltext0
	.quad	.LVL120-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL120-.Ltext0
	.quad	.LVL121-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL121-.Ltext0
	.quad	.LFE35-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	0x0
	.quad	0x0
.LLST31:
	.quad	.LVL115-.Ltext0
	.quad	.LVL117-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL117-.Ltext0
	.quad	.LVL118-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL118-.Ltext0
	.quad	.LVL119-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL120-.Ltext0
	.quad	.LVL121-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL121-.Ltext0
	.quad	.LVL122-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	0x0
	.quad	0x0
.LLST32:
	.quad	.LVL123-.Ltext0
	.quad	.LVL124-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -72
	.quad	.LVL124-.Ltext0
	.quad	.LVL139-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL139-.Ltext0
	.quad	.LVL140-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -72
	.quad	.LVL140-.Ltext0
	.quad	.LFE36-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST33:
	.quad	.LVL123-.Ltext0
	.quad	.LVL128-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL128-.Ltext0
	.quad	.LVL137-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	.LVL140-.Ltext0
	.quad	.LVL142-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	.LVL142-.Ltext0
	.quad	.LVL144-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL144-.Ltext0
	.quad	.LFE36-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	0x0
	.quad	0x0
.LLST34:
	.quad	.LVL123-.Ltext0
	.quad	.LVL126-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL126-.Ltext0
	.quad	.LVL134-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL140-.Ltext0
	.quad	.LVL145-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL145-.Ltext0
	.quad	.LVL146-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL146-.Ltext0
	.quad	.LVL147-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL147-.Ltext0
	.quad	.LVL148-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL148-.Ltext0
	.quad	.LVL149-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL149-.Ltext0
	.quad	.LVL150-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL150-.Ltext0
	.quad	.LVL151-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL151-.Ltext0
	.quad	.LVL152-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL152-.Ltext0
	.quad	.LVL153-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL153-.Ltext0
	.quad	.LVL154-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL154-.Ltext0
	.quad	.LVL155-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL155-.Ltext0
	.quad	.LVL156-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL156-.Ltext0
	.quad	.LVL157-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL157-.Ltext0
	.quad	.LVL158-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL158-.Ltext0
	.quad	.LVL159-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL159-.Ltext0
	.quad	.LVL160-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL160-.Ltext0
	.quad	.LVL161-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL161-.Ltext0
	.quad	.LVL162-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL162-.Ltext0
	.quad	.LVL163-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL163-.Ltext0
	.quad	.LVL164-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL164-.Ltext0
	.quad	.LVL165-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL165-.Ltext0
	.quad	.LVL166-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL166-.Ltext0
	.quad	.LVL167-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL167-.Ltext0
	.quad	.LVL168-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL168-.Ltext0
	.quad	.LVL169-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL169-.Ltext0
	.quad	.LVL170-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL170-.Ltext0
	.quad	.LVL171-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL171-.Ltext0
	.quad	.LVL172-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL172-.Ltext0
	.quad	.LVL173-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL173-.Ltext0
	.quad	.LVL174-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL174-.Ltext0
	.quad	.LVL175-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL175-.Ltext0
	.quad	.LVL176-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL176-.Ltext0
	.quad	.LVL177-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL177-.Ltext0
	.quad	.LVL178-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL178-.Ltext0
	.quad	.LVL179-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL179-.Ltext0
	.quad	.LVL180-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL180-.Ltext0
	.quad	.LVL181-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL181-.Ltext0
	.quad	.LVL182-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL182-.Ltext0
	.quad	.LVL183-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL183-.Ltext0
	.quad	.LVL184-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL184-.Ltext0
	.quad	.LVL185-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL185-.Ltext0
	.quad	.LVL186-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL186-.Ltext0
	.quad	.LVL187-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL187-.Ltext0
	.quad	.LVL188-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL188-.Ltext0
	.quad	.LVL189-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL189-.Ltext0
	.quad	.LVL190-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL190-.Ltext0
	.quad	.LVL191-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL191-.Ltext0
	.quad	.LVL192-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL192-.Ltext0
	.quad	.LVL193-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL193-.Ltext0
	.quad	.LVL194-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL194-.Ltext0
	.quad	.LVL195-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL195-.Ltext0
	.quad	.LVL196-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL196-.Ltext0
	.quad	.LFE36-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST35:
	.quad	.LVL123-.Ltext0
	.quad	.LVL126-.Ltext0
	.value	0x1
	.byte	0x51
	.quad	.LVL126-.Ltext0
	.quad	.LVL133-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL140-.Ltext0
	.quad	.LFE36-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST36:
	.quad	.LVL123-.Ltext0
	.quad	.LVL126-.Ltext0
	.value	0x1
	.byte	0x52
	.quad	.LVL126-.Ltext0
	.quad	.LVL138-.Ltext0
	.value	0x1
	.byte	0x5f
	.quad	.LVL140-.Ltext0
	.quad	.LFE36-.Ltext0
	.value	0x1
	.byte	0x5f
	.quad	0x0
	.quad	0x0
.LLST37:
	.quad	.LVL127-.Ltext0
	.quad	.LVL135-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL140-.Ltext0
	.quad	.LVL142-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL143-.Ltext0
	.quad	.LFE36-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	0x0
	.quad	0x0
.LLST38:
	.quad	.LVL130-.Ltext0
	.quad	.LVL136-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	.LVL140-.Ltext0
	.quad	.LVL142-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	.LVL144-.Ltext0
	.quad	.LFE36-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	0x0
	.quad	0x0
.LLST39:
	.quad	.LVL198-.Ltext0
	.quad	.LVL199-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL199-.Ltext0
	.quad	.LVL201-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL201-.Ltext0
	.quad	.LVL202-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL202-.Ltext0
	.quad	.LVL203-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL203-.Ltext0
	.quad	.LFE37-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	0x0
	.quad	0x0
.LLST40:
	.quad	.LVL198-.Ltext0
	.quad	.LVL200-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL200-.Ltext0
	.quad	.LVL201-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL201-.Ltext0
	.quad	.LVL202-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL202-.Ltext0
	.quad	.LVL203-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST41:
	.quad	.LVL206-.Ltext0
	.quad	.LVL207-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL207-.Ltext0
	.quad	.LVL208-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL208-.Ltext0
	.quad	.LVL209-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL209-.Ltext0
	.quad	.LVL211-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL211-.Ltext0
	.quad	.LFE40-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	0x0
	.quad	0x0
.LLST42:
	.quad	.LVL206-.Ltext0
	.quad	.LVL210-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	0x0
	.quad	0x0
.LLST43:
	.quad	.LVL212-.Ltext0
	.quad	.LVL213-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	0x0
	.quad	0x0
.LLST44:
	.quad	.LVL214-.Ltext0
	.quad	.LVL215-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL215-.Ltext0
	.quad	.LVL217-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL217-.Ltext0
	.quad	.LVL218-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL218-.Ltext0
	.quad	.LVL219-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL219-.Ltext0
	.quad	.LVL220-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL220-.Ltext0
	.quad	.LVL222-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL222-.Ltext0
	.quad	.LFE42-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	0x0
	.quad	0x0
.LLST45:
	.quad	.LVL214-.Ltext0
	.quad	.LVL216-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL218-.Ltext0
	.quad	.LVL221-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	0x0
	.quad	0x0
.LLST46:
	.quad	.LVL223-.Ltext0
	.quad	.LVL224-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL224-.Ltext0
	.quad	.LVL227-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL227-.Ltext0
	.quad	.LVL228-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL228-.Ltext0
	.quad	.LVL229-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL229-.Ltext0
	.quad	.LVL230-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL230-.Ltext0
	.quad	.LVL232-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL232-.Ltext0
	.quad	.LVL234-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	.LVL234-.Ltext0
	.quad	.LVL236-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL236-.Ltext0
	.quad	.LFE43-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -8
	.quad	0x0
	.quad	0x0
.LLST47:
	.quad	.LVL223-.Ltext0
	.quad	.LVL225-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL225-.Ltext0
	.quad	.LVL227-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL227-.Ltext0
	.quad	.LVL228-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL228-.Ltext0
	.quad	.LVL229-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL229-.Ltext0
	.quad	.LVL230-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL230-.Ltext0
	.quad	.LVL232-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL234-.Ltext0
	.quad	.LVL236-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST48:
	.quad	.LVL238-.Ltext0
	.quad	.LVL239-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -56
	.quad	.LVL239-.Ltext0
	.quad	.LVL254-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL254-.Ltext0
	.quad	.LVL255-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -56
	.quad	.LVL255-.Ltext0
	.quad	.LFE45-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST49:
	.quad	.LVL238-.Ltext0
	.quad	.LVL242-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL242-.Ltext0
	.quad	.LVL249-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL249-.Ltext0
	.quad	.LVL255-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL255-.Ltext0
	.quad	.LVL256-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL258-.Ltext0
	.quad	.LVL261-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL261-.Ltext0
	.quad	.LVL263-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL263-.Ltext0
	.quad	.LVL265-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL265-.Ltext0
	.quad	.LFE45-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST50:
	.quad	.LVL238-.Ltext0
	.quad	.LVL240-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL240-.Ltext0
	.quad	.LVL252-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	.LVL252-.Ltext0
	.quad	.LVL255-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL255-.Ltext0
	.quad	.LFE45-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	0x0
	.quad	0x0
.LLST51:
	.quad	.LVL238-.Ltext0
	.quad	.LVL240-.Ltext0
	.value	0x1
	.byte	0x51
	.quad	.LVL240-.Ltext0
	.quad	.LVL251-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	.LVL251-.Ltext0
	.quad	.LVL255-.Ltext0
	.value	0x1
	.byte	0x51
	.quad	.LVL255-.Ltext0
	.quad	.LVL259-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	.LVL259-.Ltext0
	.quad	.LVL261-.Ltext0
	.value	0x1
	.byte	0x51
	.quad	.LVL261-.Ltext0
	.quad	.LVL263-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	.LVL263-.Ltext0
	.quad	.LVL266-.Ltext0
	.value	0x1
	.byte	0x51
	.quad	.LVL266-.Ltext0
	.quad	.LFE45-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	0x0
	.quad	0x0
.LLST52:
	.quad	.LVL238-.Ltext0
	.quad	.LVL240-.Ltext0
	.value	0x1
	.byte	0x52
	.quad	.LVL240-.Ltext0
	.quad	.LVL253-.Ltext0
	.value	0x1
	.byte	0x5f
	.quad	.LVL253-.Ltext0
	.quad	.LVL255-.Ltext0
	.value	0x1
	.byte	0x52
	.quad	.LVL255-.Ltext0
	.quad	.LFE45-.Ltext0
	.value	0x1
	.byte	0x5f
	.quad	0x0
	.quad	0x0
.LLST53:
	.quad	.LVL241-.Ltext0
	.quad	.LVL250-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL255-.Ltext0
	.quad	.LVL258-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL260-.Ltext0
	.quad	.LVL263-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL264-.Ltext0
	.quad	.LFE45-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST54:
	.quad	.LVL244-.Ltext0
	.quad	.LVL245-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL246-.Ltext0
	.quad	.LVL247-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL248-.Ltext0
	.quad	.LVL250-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL255-.Ltext0
	.quad	.LVL257-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL262-.Ltext0
	.quad	.LVL263-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	0x0
	.quad	0x0
.LLST55:
	.quad	.LVL267-.Ltext0
	.quad	.LVL269-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -88
	.quad	.LVL269-.Ltext0
	.quad	.LVL283-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL283-.Ltext0
	.quad	.LVL284-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -88
	.quad	.LVL284-.Ltext0
	.quad	.LFE44-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST56:
	.quad	.LVL267-.Ltext0
	.quad	.LVL271-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL271-.Ltext0
	.quad	.LVL274-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL284-.Ltext0
	.quad	.LVL286-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL288-.Ltext0
	.quad	.LVL291-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL293-.Ltext0
	.quad	.LFE44-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST57:
	.quad	.LVL267-.Ltext0
	.quad	.LVL271-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL271-.Ltext0
	.quad	.LFE44-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 16
	.quad	0x0
	.quad	0x0
.LLST58:
	.quad	.LVL267-.Ltext0
	.quad	.LVL271-.Ltext0
	.value	0x1
	.byte	0x51
	.quad	.LVL271-.Ltext0
	.quad	.LVL281-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	.LVL284-.Ltext0
	.quad	.LFE44-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	0x0
	.quad	0x0
.LLST59:
	.quad	.LVL267-.Ltext0
	.quad	.LVL271-.Ltext0
	.value	0x1
	.byte	0x52
	.quad	.LVL271-.Ltext0
	.quad	.LVL276-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.quad	.LVL284-.Ltext0
	.quad	.LVL287-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.quad	.LVL288-.Ltext0
	.quad	.LFE44-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 8
	.quad	0x0
	.quad	0x0
.LLST60:
	.quad	.LVL267-.Ltext0
	.quad	.LVL272-.Ltext0
	.value	0x1
	.byte	0x58
	.quad	.LVL272-.Ltext0
	.quad	.LVL279-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL284-.Ltext0
	.quad	.LVL289-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL289-.Ltext0
	.quad	.LVL290-.Ltext0
	.value	0x1
	.byte	0x58
	.quad	.LVL290-.Ltext0
	.quad	.LVL292-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL293-.Ltext0
	.quad	.LFE44-.Ltext0
	.value	0x1
	.byte	0x58
	.quad	0x0
	.quad	0x0
.LLST61:
	.quad	.LVL267-.Ltext0
	.quad	.LVL271-.Ltext0
	.value	0x1
	.byte	0x59
	.quad	.LVL271-.Ltext0
	.quad	.LFE44-.Ltext0
	.value	0x2
	.byte	0x91
	.sleb128 4
	.quad	0x0
	.quad	0x0
.LLST62:
	.quad	.LVL270-.Ltext0
	.quad	.LVL280-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	.LVL284-.Ltext0
	.quad	.LFE44-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	0x0
	.quad	0x0
.LLST63:
	.quad	.LVL273-.Ltext0
	.quad	.LVL278-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL285-.Ltext0
	.quad	.LVL288-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL291-.Ltext0
	.quad	.LVL293-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST64:
	.quad	.LVL275-.Ltext0
	.quad	.LVL277-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL287-.Ltext0
	.quad	.LVL288-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST65:
	.quad	.LVL268-.Ltext0
	.quad	.LVL282-.Ltext0
	.value	0x1
	.byte	0x5f
	.quad	.LVL284-.Ltext0
	.quad	.LFE44-.Ltext0
	.value	0x1
	.byte	0x5f
	.quad	0x0
	.quad	0x0
.LLST66:
	.quad	.LVL294-.Ltext0
	.quad	.LVL295-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -40
	.quad	.LVL295-.Ltext0
	.quad	.LVL309-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL309-.Ltext0
	.quad	.LVL310-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -40
	.quad	.LVL310-.Ltext0
	.quad	.LFE46-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST67:
	.quad	.LVL294-.Ltext0
	.quad	.LVL298-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL298-.Ltext0
	.quad	.LVL301-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL303-.Ltext0
	.quad	.LVL311-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL311-.Ltext0
	.quad	.LVL312-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL315-.Ltext0
	.quad	.LVL316-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL316-.Ltext0
	.quad	.LFE46-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST68:
	.quad	.LVL294-.Ltext0
	.quad	.LVL297-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL297-.Ltext0
	.quad	.LVL301-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	.LVL301-.Ltext0
	.quad	.LVL303-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL303-.Ltext0
	.quad	.LVL304-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	.LVL308-.Ltext0
	.quad	.LVL310-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL310-.Ltext0
	.quad	.LVL312-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	.LVL312-.Ltext0
	.quad	.LVL315-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL315-.Ltext0
	.quad	.LVL315-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	.LVL316-.Ltext0
	.quad	.LFE46-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	0x0
	.quad	0x0
.LLST69:
	.quad	.LVL294-.Ltext0
	.quad	.LVL297-.Ltext0
	.value	0x1
	.byte	0x51
	.quad	.LVL297-.Ltext0
	.quad	.LVL305-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL305-.Ltext0
	.quad	.LVL310-.Ltext0
	.value	0x1
	.byte	0x51
	.quad	.LVL310-.Ltext0
	.quad	.LVL314-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL315-.Ltext0
	.quad	.LFE46-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST70:
	.quad	.LVL296-.Ltext0
	.quad	.LVL307-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	.LVL310-.Ltext0
	.quad	.LFE46-.Ltext0
	.value	0x1
	.byte	0x5d
	.quad	0x0
	.quad	0x0
.LLST71:
	.quad	.LVL302-.Ltext0
	.quad	.LVL305-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL314-.Ltext0
	.quad	.LVL315-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST72:
	.quad	.LVL300-.Ltext0
	.quad	.LVL306-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL313-.Ltext0
	.quad	.LVL315-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL317-.Ltext0
	.quad	.LFE46-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	0x0
	.quad	0x0
.LLST73:
	.quad	.LVL318-.Ltext0
	.quad	.LVL319-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL319-.Ltext0
	.quad	.LVL328-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL328-.Ltext0
	.quad	.LVL329-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL329-.Ltext0
	.quad	.LFE47-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST74:
	.quad	.LVL318-.Ltext0
	.quad	.LVL322-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL322-.Ltext0
	.quad	.LVL325-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL325-.Ltext0
	.quad	.LVL330-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL330-.Ltext0
	.quad	.LFE47-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST75:
	.quad	.LVL318-.Ltext0
	.quad	.LVL321-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL321-.Ltext0
	.quad	.LVL327-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL327-.Ltext0
	.quad	.LVL329-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL329-.Ltext0
	.quad	.LFE47-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	0x0
	.quad	0x0
.LLST76:
	.quad	.LVL320-.Ltext0
	.quad	.LVL326-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL329-.Ltext0
	.quad	.LFE47-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST77:
	.quad	.LVL331-.Ltext0
	.quad	.LVL332-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL332-.Ltext0
	.quad	.LVL335-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL335-.Ltext0
	.quad	.LVL336-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL336-.Ltext0
	.quad	.LVL340-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL340-.Ltext0
	.quad	.LVL341-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL341-.Ltext0
	.quad	.LVL347-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL347-.Ltext0
	.quad	.LFE48-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	0x0
	.quad	0x0
.LLST78:
	.quad	.LVL331-.Ltext0
	.quad	.LVL337-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL337-.Ltext0
	.quad	.LVL338-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL341-.Ltext0
	.quad	.LVL342-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL342-.Ltext0
	.quad	.LVL345-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST79:
	.quad	.LVL331-.Ltext0
	.quad	.LVL333-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL333-.Ltext0
	.quad	.LVL334-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL334-.Ltext0
	.quad	.LVL336-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL336-.Ltext0
	.quad	.LVL339-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL339-.Ltext0
	.quad	.LVL341-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL341-.Ltext0
	.quad	.LVL346-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	0x0
	.quad	0x0
.LLST80:
	.quad	.LVL343-.Ltext0
	.quad	.LVL344-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL344-.Ltext0
	.quad	.LVL348-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	0x0
	.quad	0x0
.LLST81:
	.quad	.LVL349-.Ltext0
	.quad	.LVL350-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -216
	.quad	.LVL350-.Ltext0
	.quad	.LVL351-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -208
	.quad	.LVL351-.Ltext0
	.quad	.LVL352-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -200
	.quad	.LVL352-.Ltext0
	.quad	.LVL353-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -192
	.quad	.LVL353-.Ltext0
	.quad	.LVL354-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -184
	.quad	.LVL354-.Ltext0
	.quad	.LVL355-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -176
	.quad	.LVL355-.Ltext0
	.quad	.LVL356-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -168
	.quad	.LVL356-.Ltext0
	.quad	.LVL357-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL357-.Ltext0
	.quad	.LVL358-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -168
	.quad	.LVL358-.Ltext0
	.quad	.LVL359-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -176
	.quad	.LVL359-.Ltext0
	.quad	.LVL360-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -184
	.quad	.LVL360-.Ltext0
	.quad	.LVL361-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -192
	.quad	.LVL361-.Ltext0
	.quad	.LVL362-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -200
	.quad	.LVL362-.Ltext0
	.quad	.LVL363-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -208
	.quad	.LVL363-.Ltext0
	.quad	.LVL364-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -216
	.quad	.LVL364-.Ltext0
	.quad	.LVL380-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL380-.Ltext0
	.quad	.LVL381-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -168
	.quad	.LVL381-.Ltext0
	.quad	.LVL382-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -176
	.quad	.LVL382-.Ltext0
	.quad	.LVL383-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -184
	.quad	.LVL383-.Ltext0
	.quad	.LVL384-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -192
	.quad	.LVL384-.Ltext0
	.quad	.LVL385-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -200
	.quad	.LVL385-.Ltext0
	.quad	.LVL386-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -208
	.quad	.LVL386-.Ltext0
	.quad	.LVL387-.Ltext0
	.value	0x3
	.byte	0x77
	.sleb128 -216
	.quad	.LVL387-.Ltext0
	.quad	.LFE50-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST82:
	.quad	.LVL365-.Ltext0
	.quad	.LVL385-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	.LVL387-.Ltext0
	.quad	.LFE50-.Ltext0
	.value	0x1
	.byte	0x5e
	.quad	0x0
	.quad	0x0
.LLST83:
	.quad	.LVL369-.Ltext0
	.quad	.LVL370-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL378-.Ltext0
	.quad	.LVL381-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL391-.Ltext0
	.quad	.LVL393-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL394-.Ltext0
	.quad	.LFE50-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST84:
	.quad	.LVL370-.Ltext0
	.quad	.LVL373-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL377-.Ltext0
	.quad	.LVL378-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST85:
	.quad	.LVL367-.Ltext0
	.quad	.LVL379-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	.LVL387-.Ltext0
	.quad	.LFE50-.Ltext0
	.value	0x1
	.byte	0x5c
	.quad	0x0
	.quad	0x0
.LLST86:
	.quad	.LVL367-.Ltext0
	.quad	.LVL369-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL371-.Ltext0
	.quad	.LVL372-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL373-.Ltext0
	.quad	.LVL374-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL374-.Ltext0
	.quad	.LVL375-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL376-.Ltext0
	.quad	.LVL376-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL387-.Ltext0
	.quad	.LVL388-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL388-.Ltext0
	.quad	.LVL390-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL390-.Ltext0
	.quad	.LVL391-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL393-.Ltext0
	.quad	.LVL394-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL394-.Ltext0
	.quad	.LVL395-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	0x0
	.quad	0x0
.LLST87:
	.quad	.LVL389-.Ltext0
	.quad	.LVL392-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL393-.Ltext0
	.quad	.LVL395-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	0x0
	.quad	0x0
	.section	.debug_info
	.long	0xf4e
	.value	0x2
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.Ldebug_line0
	.quad	.Letext0
	.quad	.Ltext0
	.long	.LASF148
	.byte	0x1
	.long	.LASF149
	.long	.LASF150
	.uleb128 0x2
	.long	.LASF7
	.byte	0x5
	.byte	0xd6
	.long	0x38
	.uleb128 0x3
	.long	.LASF0
	.byte	0x8
	.byte	0x7
	.uleb128 0x4
	.string	"int"
	.byte	0x4
	.byte	0x5
	.uleb128 0x3
	.long	.LASF1
	.byte	0x8
	.byte	0x5
	.uleb128 0x3
	.long	.LASF2
	.byte	0x1
	.byte	0x8
	.uleb128 0x3
	.long	.LASF3
	.byte	0x2
	.byte	0x7
	.uleb128 0x3
	.long	.LASF4
	.byte	0x4
	.byte	0x7
	.uleb128 0x3
	.long	.LASF5
	.byte	0x1
	.byte	0x6
	.uleb128 0x3
	.long	.LASF6
	.byte	0x2
	.byte	0x5
	.uleb128 0x2
	.long	.LASF8
	.byte	0x8
	.byte	0x8f
	.long	0x46
	.uleb128 0x2
	.long	.LASF9
	.byte	0x8
	.byte	0x90
	.long	0x46
	.uleb128 0x3
	.long	.LASF0
	.byte	0x8
	.byte	0x7
	.uleb128 0x5
	.byte	0x8
	.uleb128 0x6
	.byte	0x8
	.long	0x95
	.uleb128 0x3
	.long	.LASF10
	.byte	0x1
	.byte	0x6
	.uleb128 0x2
	.long	.LASF11
	.byte	0x6
	.byte	0x2e
	.long	0xa7
	.uleb128 0x7
	.long	0x243
	.long	.LASF39
	.byte	0xd8
	.byte	0x6
	.byte	0x2e
	.uleb128 0x8
	.long	.LASF12
	.byte	0x7
	.value	0x10c
	.long	0x3f
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x8
	.long	.LASF13
	.byte	0x7
	.value	0x111
	.long	0x8f
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x8
	.long	.LASF14
	.byte	0x7
	.value	0x112
	.long	0x8f
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x8
	.long	.LASF15
	.byte	0x7
	.value	0x113
	.long	0x8f
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x8
	.long	.LASF16
	.byte	0x7
	.value	0x114
	.long	0x8f
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x8
	.long	.LASF17
	.byte	0x7
	.value	0x115
	.long	0x8f
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x8
	.long	.LASF18
	.byte	0x7
	.value	0x116
	.long	0x8f
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.uleb128 0x8
	.long	.LASF19
	.byte	0x7
	.value	0x117
	.long	0x8f
	.byte	0x2
	.byte	0x23
	.uleb128 0x38
	.uleb128 0x8
	.long	.LASF20
	.byte	0x7
	.value	0x118
	.long	0x8f
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0x8
	.long	.LASF21
	.byte	0x7
	.value	0x11a
	.long	0x8f
	.byte	0x2
	.byte	0x23
	.uleb128 0x48
	.uleb128 0x8
	.long	.LASF22
	.byte	0x7
	.value	0x11b
	.long	0x8f
	.byte	0x2
	.byte	0x23
	.uleb128 0x50
	.uleb128 0x8
	.long	.LASF23
	.byte	0x7
	.value	0x11c
	.long	0x8f
	.byte	0x2
	.byte	0x23
	.uleb128 0x58
	.uleb128 0x8
	.long	.LASF24
	.byte	0x7
	.value	0x11e
	.long	0x2af
	.byte	0x2
	.byte	0x23
	.uleb128 0x60
	.uleb128 0x8
	.long	.LASF25
	.byte	0x7
	.value	0x120
	.long	0x2b5
	.byte	0x2
	.byte	0x23
	.uleb128 0x68
	.uleb128 0x8
	.long	.LASF26
	.byte	0x7
	.value	0x122
	.long	0x3f
	.byte	0x2
	.byte	0x23
	.uleb128 0x70
	.uleb128 0x8
	.long	.LASF27
	.byte	0x7
	.value	0x126
	.long	0x3f
	.byte	0x2
	.byte	0x23
	.uleb128 0x74
	.uleb128 0x8
	.long	.LASF28
	.byte	0x7
	.value	0x128
	.long	0x70
	.byte	0x2
	.byte	0x23
	.uleb128 0x78
	.uleb128 0x8
	.long	.LASF29
	.byte	0x7
	.value	0x12c
	.long	0x54
	.byte	0x3
	.byte	0x23
	.uleb128 0x80
	.uleb128 0x8
	.long	.LASF30
	.byte	0x7
	.value	0x12d
	.long	0x62
	.byte	0x3
	.byte	0x23
	.uleb128 0x82
	.uleb128 0x8
	.long	.LASF31
	.byte	0x7
	.value	0x12e
	.long	0x2bb
	.byte	0x3
	.byte	0x23
	.uleb128 0x83
	.uleb128 0x8
	.long	.LASF32
	.byte	0x7
	.value	0x132
	.long	0x2cb
	.byte	0x3
	.byte	0x23
	.uleb128 0x88
	.uleb128 0x8
	.long	.LASF33
	.byte	0x7
	.value	0x13b
	.long	0x7b
	.byte	0x3
	.byte	0x23
	.uleb128 0x90
	.uleb128 0x8
	.long	.LASF34
	.byte	0x7
	.value	0x141
	.long	0x8d
	.byte	0x3
	.byte	0x23
	.uleb128 0x98
	.uleb128 0x8
	.long	.LASF35
	.byte	0x7
	.value	0x142
	.long	0x8d
	.byte	0x3
	.byte	0x23
	.uleb128 0xa0
	.uleb128 0x8
	.long	.LASF36
	.byte	0x7
	.value	0x144
	.long	0x3f
	.byte	0x3
	.byte	0x23
	.uleb128 0xa8
	.uleb128 0x8
	.long	.LASF37
	.byte	0x7
	.value	0x146
	.long	0x2d1
	.byte	0x3
	.byte	0x23
	.uleb128 0xac
	.byte	0x0
	.uleb128 0x6
	.byte	0x8
	.long	0x249
	.uleb128 0x9
	.long	0x4d
	.uleb128 0x6
	.byte	0x8
	.long	0x4d
	.uleb128 0x6
	.byte	0x8
	.long	0x25a
	.uleb128 0x9
	.long	0x95
	.uleb128 0x6
	.byte	0x8
	.long	0x265
	.uleb128 0xa
	.long	0x271
	.byte	0x1
	.uleb128 0xb
	.long	0x8d
	.byte	0x0
	.uleb128 0xc
	.long	.LASF38
	.byte	0x7
	.byte	0xb0
	.uleb128 0x7
	.long	0x2af
	.long	.LASF40
	.byte	0x18
	.byte	0x7
	.byte	0xb6
	.uleb128 0xd
	.long	.LASF41
	.byte	0x7
	.byte	0xb7
	.long	0x2af
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xd
	.long	.LASF42
	.byte	0x7
	.byte	0xb8
	.long	0x2b5
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0xd
	.long	.LASF43
	.byte	0x7
	.byte	0xbc
	.long	0x3f
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0x6
	.byte	0x8
	.long	0x278
	.uleb128 0x6
	.byte	0x8
	.long	0xa7
	.uleb128 0xe
	.long	0x2cb
	.long	0x95
	.uleb128 0xf
	.long	0x86
	.byte	0x0
	.byte	0x0
	.uleb128 0x6
	.byte	0x8
	.long	0x271
	.uleb128 0xe
	.long	0x2e1
	.long	0x95
	.uleb128 0xf
	.long	0x86
	.byte	0x2b
	.byte	0x0
	.uleb128 0x6
	.byte	0x8
	.long	0x9c
	.uleb128 0x2
	.long	.LASF44
	.byte	0x3
	.byte	0x5d
	.long	0x2f2
	.uleb128 0x7
	.long	0x329
	.long	.LASF44
	.byte	0x10
	.byte	0x3
	.byte	0x5d
	.uleb128 0x10
	.string	"val"
	.byte	0x1
	.byte	0x31
	.long	0x43e
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xd
	.long	.LASF45
	.byte	0x1
	.byte	0x32
	.long	0x45f
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0xd
	.long	.LASF46
	.byte	0x1
	.byte	0x33
	.long	0x4d
	.byte	0x2
	.byte	0x23
	.uleb128 0xc
	.byte	0x0
	.uleb128 0x11
	.long	0x3e0
	.byte	0x4
	.byte	0x3
	.byte	0x78
	.uleb128 0x12
	.long	.LASF47
	.sleb128 0
	.uleb128 0x12
	.long	.LASF48
	.sleb128 1
	.uleb128 0x12
	.long	.LASF49
	.sleb128 2
	.uleb128 0x12
	.long	.LASF50
	.sleb128 3
	.uleb128 0x12
	.long	.LASF51
	.sleb128 4
	.uleb128 0x12
	.long	.LASF52
	.sleb128 5
	.uleb128 0x12
	.long	.LASF53
	.sleb128 6
	.uleb128 0x12
	.long	.LASF54
	.sleb128 7
	.uleb128 0x12
	.long	.LASF55
	.sleb128 8
	.uleb128 0x12
	.long	.LASF56
	.sleb128 9
	.uleb128 0x12
	.long	.LASF57
	.sleb128 10
	.uleb128 0x12
	.long	.LASF58
	.sleb128 11
	.uleb128 0x12
	.long	.LASF59
	.sleb128 12
	.uleb128 0x12
	.long	.LASF60
	.sleb128 13
	.uleb128 0x12
	.long	.LASF61
	.sleb128 14
	.uleb128 0x12
	.long	.LASF62
	.sleb128 15
	.uleb128 0x12
	.long	.LASF63
	.sleb128 16
	.uleb128 0x12
	.long	.LASF64
	.sleb128 17
	.uleb128 0x12
	.long	.LASF65
	.sleb128 18
	.uleb128 0x12
	.long	.LASF66
	.sleb128 19
	.uleb128 0x12
	.long	.LASF67
	.sleb128 20
	.uleb128 0x12
	.long	.LASF68
	.sleb128 21
	.uleb128 0x12
	.long	.LASF69
	.sleb128 22
	.uleb128 0x12
	.long	.LASF70
	.sleb128 23
	.uleb128 0x12
	.long	.LASF71
	.sleb128 24
	.uleb128 0x12
	.long	.LASF72
	.sleb128 25
	.uleb128 0x12
	.long	.LASF73
	.sleb128 26
	.uleb128 0x12
	.long	.LASF74
	.sleb128 27
	.uleb128 0x12
	.long	.LASF75
	.sleb128 28
	.byte	0x0
	.uleb128 0x2
	.long	.LASF76
	.byte	0x3
	.byte	0x97
	.long	0x329
	.uleb128 0x6
	.byte	0x8
	.long	0x2e7
	.uleb128 0x2
	.long	.LASF77
	.byte	0x4
	.byte	0x18
	.long	0x5b
	.uleb128 0x2
	.long	.LASF78
	.byte	0x4
	.byte	0x31
	.long	0x407
	.uleb128 0x6
	.byte	0x8
	.long	0x3f1
	.uleb128 0x11
	.long	0x422
	.byte	0x4
	.byte	0x4
	.byte	0x55
	.uleb128 0x12
	.long	.LASF79
	.sleb128 0
	.uleb128 0x12
	.long	.LASF80
	.sleb128 1
	.byte	0x0
	.uleb128 0x2
	.long	.LASF81
	.byte	0x4
	.byte	0x55
	.long	0x40d
	.uleb128 0x2
	.long	.LASF82
	.byte	0x4
	.byte	0xd4
	.long	0x438
	.uleb128 0x13
	.long	.LASF82
	.byte	0x1
	.uleb128 0x14
	.long	0x45f
	.string	"val"
	.byte	0x8
	.byte	0x1
	.byte	0x2e
	.uleb128 0x15
	.string	"ul"
	.byte	0x1
	.byte	0x2f
	.long	0x38
	.uleb128 0x15
	.string	"bv"
	.byte	0x1
	.byte	0x30
	.long	0x3fc
	.byte	0x0
	.uleb128 0x11
	.long	0x474
	.byte	0x4
	.byte	0x1
	.byte	0x32
	.uleb128 0x12
	.long	.LASF83
	.sleb128 0
	.uleb128 0x12
	.long	.LASF84
	.sleb128 1
	.byte	0x0
	.uleb128 0x7
	.long	0x4c7
	.long	.LASF85
	.byte	0x20
	.byte	0x2
	.byte	0x27
	.uleb128 0xd
	.long	.LASF86
	.byte	0x2
	.byte	0x29
	.long	0x3f
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xd
	.long	.LASF87
	.byte	0x2
	.byte	0x2c
	.long	0x3f
	.byte	0x2
	.byte	0x23
	.uleb128 0x4
	.uleb128 0xd
	.long	.LASF88
	.byte	0x2
	.byte	0x2f
	.long	0x254
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0xd
	.long	.LASF89
	.byte	0x2
	.byte	0x32
	.long	0x38
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0xd
	.long	.LASF90
	.byte	0x2
	.byte	0x35
	.long	0x243
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.byte	0x0
	.uleb128 0x2
	.long	.LASF85
	.byte	0x2
	.byte	0x36
	.long	0x474
	.uleb128 0x16
	.byte	0x1
	.long	.LASF91
	.byte	0x1
	.byte	0x41
	.byte	0x1
	.quad	.LFB25
	.quad	.LFE25
	.long	.LLST0
	.uleb128 0x16
	.byte	0x1
	.long	.LASF92
	.byte	0x1
	.byte	0x4c
	.byte	0x1
	.quad	.LFB26
	.quad	.LFE26
	.long	.LLST1
	.uleb128 0x17
	.long	0x55f
	.byte	0x1
	.long	.LASF94
	.byte	0x1
	.byte	0x57
	.byte	0x1
	.long	0x3eb
	.quad	.LFB27
	.quad	.LFE27
	.long	.LLST2
	.uleb128 0x18
	.string	"str"
	.byte	0x1
	.byte	0x56
	.long	0x8f
	.long	.LLST3
	.uleb128 0x19
	.long	.LASF93
	.byte	0x1
	.byte	0x56
	.long	0x38
	.long	.LLST4
	.uleb128 0x1a
	.long	.LASF96
	.byte	0x1
	.byte	0x58
	.long	0x3eb
	.long	.LLST5
	.byte	0x0
	.uleb128 0x17
	.long	0x5b2
	.byte	0x1
	.long	.LASF95
	.byte	0x1
	.byte	0x6d
	.byte	0x1
	.long	0x3eb
	.quad	.LFB28
	.quad	.LFE28
	.long	.LLST6
	.uleb128 0x18
	.string	"str"
	.byte	0x1
	.byte	0x6c
	.long	0x8f
	.long	.LLST7
	.uleb128 0x19
	.long	.LASF93
	.byte	0x1
	.byte	0x6c
	.long	0x38
	.long	.LLST8
	.uleb128 0x1a
	.long	.LASF96
	.byte	0x1
	.byte	0x6e
	.long	0x3eb
	.long	.LLST9
	.byte	0x0
	.uleb128 0x17
	.long	0x605
	.byte	0x1
	.long	.LASF97
	.byte	0x1
	.byte	0x84
	.byte	0x1
	.long	0x3eb
	.quad	.LFB29
	.quad	.LFE29
	.long	.LLST10
	.uleb128 0x18
	.string	"str"
	.byte	0x1
	.byte	0x83
	.long	0x8f
	.long	.LLST11
	.uleb128 0x19
	.long	.LASF93
	.byte	0x1
	.byte	0x83
	.long	0x38
	.long	.LLST12
	.uleb128 0x1a
	.long	.LASF96
	.byte	0x1
	.byte	0x85
	.long	0x3eb
	.long	.LLST13
	.byte	0x0
	.uleb128 0x17
	.long	0x658
	.byte	0x1
	.long	.LASF98
	.byte	0x1
	.byte	0x9b
	.byte	0x1
	.long	0x3eb
	.quad	.LFB30
	.quad	.LFE30
	.long	.LLST14
	.uleb128 0x18
	.string	"str"
	.byte	0x1
	.byte	0x9a
	.long	0x8f
	.long	.LLST15
	.uleb128 0x19
	.long	.LASF93
	.byte	0x1
	.byte	0x9a
	.long	0x38
	.long	.LLST16
	.uleb128 0x1a
	.long	.LASF96
	.byte	0x1
	.byte	0x9c
	.long	0x3eb
	.long	.LLST17
	.byte	0x0
	.uleb128 0x17
	.long	0x6ba
	.byte	0x1
	.long	.LASF99
	.byte	0x1
	.byte	0xb3
	.byte	0x1
	.long	0x3eb
	.quad	.LFB31
	.quad	.LFE31
	.long	.LLST18
	.uleb128 0x18
	.string	"str"
	.byte	0x1
	.byte	0xb2
	.long	0x254
	.long	.LLST19
	.uleb128 0x19
	.long	.LASF93
	.byte	0x1
	.byte	0xb2
	.long	0x38
	.long	.LLST20
	.uleb128 0x1a
	.long	.LASF96
	.byte	0x1
	.byte	0xb4
	.long	0x3eb
	.long	.LLST21
	.uleb128 0x1b
	.string	"len"
	.byte	0x1
	.byte	0xb5
	.long	0x2d
	.long	.LLST22
	.byte	0x0
	.uleb128 0x17
	.long	0x6f8
	.byte	0x1
	.long	.LASF100
	.byte	0x1
	.byte	0xe6
	.byte	0x1
	.long	0x3eb
	.quad	.LFB32
	.quad	.LFE32
	.long	.LLST23
	.uleb128 0x18
	.string	"i"
	.byte	0x1
	.byte	0xe5
	.long	0x38
	.long	.LLST24
	.uleb128 0x1c
	.long	.LASF96
	.byte	0x1
	.byte	0xe7
	.long	0x3eb
	.byte	0x0
	.uleb128 0x17
	.long	0x736
	.byte	0x1
	.long	.LASF101
	.byte	0x1
	.byte	0xf2
	.byte	0x1
	.long	0x3eb
	.quad	.LFB33
	.quad	.LFE33
	.long	.LLST25
	.uleb128 0x18
	.string	"i"
	.byte	0x1
	.byte	0xf1
	.long	0x46
	.long	.LLST26
	.uleb128 0x1c
	.long	.LASF96
	.byte	0x1
	.byte	0xf3
	.long	0x3eb
	.byte	0x0
	.uleb128 0x1d
	.long	0x77b
	.byte	0x1
	.long	.LASF102
	.byte	0x1
	.value	0x107
	.byte	0x1
	.long	0x3eb
	.quad	.LFB34
	.quad	.LFE34
	.long	.LLST27
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x106
	.long	0x77b
	.long	.LLST28
	.uleb128 0x1f
	.string	"n"
	.byte	0x1
	.value	0x108
	.long	0x3eb
	.long	.LLST29
	.byte	0x0
	.uleb128 0x6
	.byte	0x8
	.long	0x781
	.uleb128 0x9
	.long	0x2e7
	.uleb128 0x20
	.long	0x7b9
	.byte	0x1
	.long	.LASF103
	.byte	0x1
	.value	0x11a
	.byte	0x1
	.quad	.LFB35
	.quad	.LFE35
	.long	.LLST30
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x119
	.long	0x3eb
	.long	.LLST31
	.byte	0x0
	.uleb128 0x20
	.long	0x84a
	.byte	0x1
	.long	.LASF104
	.byte	0x1
	.value	0x124
	.byte	0x1
	.quad	.LFB36
	.quad	.LFE36
	.long	.LLST32
	.uleb128 0x21
	.string	"acc"
	.byte	0x1
	.value	0x122
	.long	0x3eb
	.long	.LLST33
	.uleb128 0x21
	.string	"op"
	.byte	0x1
	.value	0x122
	.long	0x3e0
	.long	.LLST34
	.uleb128 0x1e
	.long	.LASF105
	.byte	0x1
	.value	0x122
	.long	0x3eb
	.long	.LLST35
	.uleb128 0x1e
	.long	.LASF93
	.byte	0x1
	.value	0x123
	.long	0x38
	.long	.LLST36
	.uleb128 0x22
	.long	.LASF106
	.byte	0x1
	.value	0x125
	.long	0x422
	.byte	0x2
	.byte	0x77
	.sleb128 20
	.uleb128 0x1f
	.string	"op1"
	.byte	0x1
	.value	0x126
	.long	0x3fc
	.long	.LLST37
	.uleb128 0x1f
	.string	"op2"
	.byte	0x1
	.value	0x126
	.long	0x3fc
	.long	.LLST38
	.byte	0x0
	.uleb128 0x20
	.long	0x87d
	.byte	0x1
	.long	.LASF107
	.byte	0x1
	.value	0x1c6
	.byte	0x1
	.quad	.LFB37
	.quad	.LFE37
	.long	.LLST39
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x1c5
	.long	0x3eb
	.long	.LLST40
	.byte	0x0
	.uleb128 0x23
	.long	0x8b1
	.byte	0x1
	.long	.LASF108
	.byte	0x1
	.value	0x1d0
	.byte	0x1
	.long	0x3f
	.quad	.LFB38
	.quad	.LFE38
	.byte	0x2
	.byte	0x77
	.sleb128 0
	.uleb128 0x24
	.long	.LASF96
	.byte	0x1
	.value	0x1cf
	.long	0x77b
	.byte	0x1
	.byte	0x55
	.byte	0x0
	.uleb128 0x23
	.long	0x8e5
	.byte	0x1
	.long	.LASF109
	.byte	0x1
	.value	0x1d6
	.byte	0x1
	.long	0x3f
	.quad	.LFB39
	.quad	.LFE39
	.byte	0x2
	.byte	0x77
	.sleb128 0
	.uleb128 0x24
	.long	.LASF96
	.byte	0x1
	.value	0x1d5
	.long	0x77b
	.byte	0x1
	.byte	0x55
	.byte	0x0
	.uleb128 0x1d
	.long	0x91c
	.byte	0x1
	.long	.LASF110
	.byte	0x1
	.value	0x1dc
	.byte	0x1
	.long	0x3f
	.quad	.LFB40
	.quad	.LFE40
	.long	.LLST41
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x1db
	.long	0x77b
	.long	.LLST42
	.byte	0x0
	.uleb128 0x23
	.long	0x952
	.byte	0x1
	.long	.LASF111
	.byte	0x1
	.value	0x1e2
	.byte	0x1
	.long	0x3f
	.quad	.LFB41
	.quad	.LFE41
	.byte	0x2
	.byte	0x77
	.sleb128 0
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x1e1
	.long	0x77b
	.long	.LLST43
	.byte	0x0
	.uleb128 0x1d
	.long	0x989
	.byte	0x1
	.long	.LASF112
	.byte	0x1
	.value	0x1ee
	.byte	0x1
	.long	0x38
	.quad	.LFB42
	.quad	.LFE42
	.long	.LLST44
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x1ed
	.long	0x77b
	.long	.LLST45
	.byte	0x0
	.uleb128 0x1d
	.long	0x9d3
	.byte	0x1
	.long	.LASF113
	.byte	0x1
	.value	0x1fd
	.byte	0x1
	.long	0x46
	.quad	.LFB43
	.quad	.LFE43
	.long	.LLST46
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x1fc
	.long	0x77b
	.long	.LLST47
	.uleb128 0x25
	.long	.Ldebug_ranges0+0x0
	.uleb128 0x26
	.string	"ul"
	.byte	0x1
	.value	0x207
	.long	0x38
	.byte	0x1
	.byte	0x50
	.byte	0x0
	.byte	0x0
	.uleb128 0x1d
	.long	0xa8e
	.byte	0x1
	.long	.LASF114
	.byte	0x1
	.value	0x262
	.byte	0x1
	.long	0x3f
	.quad	.LFB45
	.quad	.LFE45
	.long	.LLST48
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x260
	.long	0x77b
	.long	.LLST49
	.uleb128 0x1e
	.long	.LASF115
	.byte	0x1
	.value	0x260
	.long	0x2d
	.long	.LLST50
	.uleb128 0x1e
	.long	.LASF116
	.byte	0x1
	.value	0x260
	.long	0x2d
	.long	.LLST51
	.uleb128 0x1e
	.long	.LASF117
	.byte	0x1
	.value	0x261
	.long	0x3f
	.long	.LLST52
	.uleb128 0x1f
	.string	"val"
	.byte	0x1
	.value	0x263
	.long	0x3fc
	.long	.LLST53
	.uleb128 0x27
	.long	0xa6b
	.quad	.LBB5
	.quad	.LBE5
	.uleb128 0x28
	.long	.LASF118
	.byte	0x1
	.value	0x276
	.long	0x3f
	.byte	0x0
	.uleb128 0x29
	.quad	.LBB6
	.quad	.LBE6
	.uleb128 0x2a
	.long	.LASF119
	.byte	0x1
	.value	0x27e
	.long	0x3f
	.long	.LLST54
	.byte	0x0
	.byte	0x0
	.uleb128 0x20
	.long	0xb8c
	.byte	0x1
	.long	.LASF120
	.byte	0x1
	.value	0x220
	.byte	0x1
	.quad	.LFB44
	.quad	.LFE44
	.long	.LLST55
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x21d
	.long	0x77b
	.long	.LLST56
	.uleb128 0x21
	.string	"ptr"
	.byte	0x1
	.value	0x21d
	.long	0x24e
	.long	.LLST57
	.uleb128 0x1e
	.long	.LASF121
	.byte	0x1
	.value	0x21e
	.long	0x2d
	.long	.LLST58
	.uleb128 0x1e
	.long	.LASF122
	.byte	0x1
	.value	0x21e
	.long	0x2d
	.long	.LLST59
	.uleb128 0x1e
	.long	.LASF123
	.byte	0x1
	.value	0x21e
	.long	0x3f
	.long	.LLST60
	.uleb128 0x1e
	.long	.LASF124
	.byte	0x1
	.value	0x21f
	.long	0x3f
	.long	.LLST61
	.uleb128 0x24
	.long	.LASF125
	.byte	0x1
	.value	0x21f
	.long	0x3f
	.byte	0x3
	.byte	0x77
	.sleb128 96
	.uleb128 0x24
	.long	.LASF93
	.byte	0x1
	.value	0x21f
	.long	0x38
	.byte	0x3
	.byte	0x77
	.sleb128 104
	.uleb128 0x1f
	.string	"op1"
	.byte	0x1
	.value	0x221
	.long	0x3fc
	.long	.LLST62
	.uleb128 0x1f
	.string	"op2"
	.byte	0x1
	.value	0x221
	.long	0x3fc
	.long	.LLST63
	.uleb128 0x1f
	.string	"buf"
	.byte	0x1
	.value	0x222
	.long	0x24e
	.long	.LLST64
	.uleb128 0x26
	.string	"len"
	.byte	0x1
	.value	0x223
	.long	0x5b
	.byte	0x2
	.byte	0x77
	.sleb128 36
	.uleb128 0x2a
	.long	.LASF116
	.byte	0x1
	.value	0x224
	.long	0x2d
	.long	.LLST65
	.uleb128 0x28
	.long	.LASF118
	.byte	0x1
	.value	0x225
	.long	0x3f
	.byte	0x0
	.uleb128 0x1d
	.long	0xc1d
	.byte	0x1
	.long	.LASF126
	.byte	0x1
	.value	0x28f
	.byte	0x1
	.long	0x38
	.quad	.LFB46
	.quad	.LFE46
	.long	.LLST66
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x28e
	.long	0x77b
	.long	.LLST67
	.uleb128 0x21
	.string	"ptr"
	.byte	0x1
	.value	0x28e
	.long	0x24e
	.long	.LLST68
	.uleb128 0x1e
	.long	.LASF86
	.byte	0x1
	.value	0x28e
	.long	0x3f
	.long	.LLST69
	.uleb128 0x1f
	.string	"val"
	.byte	0x1
	.value	0x290
	.long	0x3fc
	.long	.LLST70
	.uleb128 0x1f
	.string	"i"
	.byte	0x1
	.value	0x291
	.long	0x38
	.long	.LLST71
	.uleb128 0x2a
	.long	.LASF115
	.byte	0x1
	.value	0x291
	.long	0x38
	.long	.LLST72
	.uleb128 0x28
	.long	.LASF127
	.byte	0x1
	.value	0x292
	.long	0x24e
	.byte	0x0
	.uleb128 0x1d
	.long	0xc74
	.byte	0x1
	.long	.LASF128
	.byte	0x1
	.value	0x2bd
	.byte	0x1
	.long	0x38
	.quad	.LFB47
	.quad	.LFE47
	.long	.LLST73
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x2bc
	.long	0x77b
	.long	.LLST74
	.uleb128 0x1e
	.long	.LASF86
	.byte	0x1
	.value	0x2bc
	.long	0x3f
	.long	.LLST75
	.uleb128 0x1f
	.string	"val"
	.byte	0x1
	.value	0x2be
	.long	0x3fc
	.long	.LLST76
	.byte	0x0
	.uleb128 0x20
	.long	0xcc3
	.byte	0x1
	.long	.LASF129
	.byte	0x1
	.value	0x2df
	.byte	0x1
	.quad	.LFB48
	.quad	.LFE48
	.long	.LLST77
	.uleb128 0x1e
	.long	.LASF96
	.byte	0x1
	.value	0x2de
	.long	0x77b
	.long	.LLST78
	.uleb128 0x21
	.string	"f"
	.byte	0x1
	.value	0x2de
	.long	0x2e1
	.long	.LLST79
	.uleb128 0x1f
	.string	"s"
	.byte	0x1
	.value	0x2e0
	.long	0x24e
	.long	.LLST80
	.byte	0x0
	.uleb128 0x2b
	.long	0xd20
	.long	.LASF151
	.byte	0x2
	.byte	0x54
	.byte	0x1
	.long	0x3f
	.byte	0x1
	.uleb128 0x2c
	.long	.LASF130
	.byte	0x2
	.byte	0x53
	.long	0xd20
	.uleb128 0x1c
	.long	.LASF131
	.byte	0x2
	.byte	0x55
	.long	0x8f
	.uleb128 0x1c
	.long	.LASF96
	.byte	0x2
	.byte	0x56
	.long	0x3eb
	.uleb128 0x1c
	.long	.LASF115
	.byte	0x2
	.byte	0x57
	.long	0x38
	.uleb128 0x2d
	.string	"i"
	.byte	0x2
	.byte	0x57
	.long	0x38
	.uleb128 0x2d
	.string	"out"
	.byte	0x2
	.byte	0x58
	.long	0xd26
	.uleb128 0x2d
	.string	"bad"
	.byte	0x2
	.byte	0x59
	.long	0x3f
	.byte	0x0
	.uleb128 0x6
	.byte	0x8
	.long	0x4c7
	.uleb128 0xe
	.long	0xd36
	.long	0x4d
	.uleb128 0xf
	.long	0x86
	.byte	0x63
	.byte	0x0
	.uleb128 0x17
	.long	0xde8
	.byte	0x1
	.long	.LASF132
	.byte	0x2
	.byte	0x86
	.byte	0x1
	.long	0x3f
	.quad	.LFB50
	.quad	.LFE50
	.long	.LLST81
	.uleb128 0x2e
	.string	"nf"
	.byte	0x2
	.byte	0x87
	.long	0x3f
	.byte	0x2
	.byte	0x77
	.sleb128 44
	.uleb128 0x1c
	.long	.LASF133
	.byte	0x2
	.byte	0x88
	.long	0x3f
	.uleb128 0x1b
	.string	"i"
	.byte	0x2
	.byte	0x89
	.long	0x3f
	.long	.LLST82
	.uleb128 0x25
	.long	.Ldebug_ranges0+0x40
	.uleb128 0x1a
	.long	.LASF134
	.byte	0x2
	.byte	0x92
	.long	0x3f
	.long	.LLST83
	.uleb128 0x2f
	.long	0xcc3
	.quad	.LBB8
	.quad	.LBE8
	.uleb128 0x30
	.long	0xcd4
	.uleb128 0x25
	.long	.Ldebug_ranges0+0xb0
	.uleb128 0x31
	.long	0xcdf
	.long	.LLST84
	.uleb128 0x31
	.long	0xcea
	.long	.LLST85
	.uleb128 0x31
	.long	0xcf5
	.long	.LLST86
	.uleb128 0x32
	.long	0xd00
	.uleb128 0x33
	.long	0xd09
	.byte	0x2
	.byte	0x77
	.sleb128 48
	.uleb128 0x31
	.long	0xd14
	.long	.LLST87
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.uleb128 0x34
	.long	.LASF135
	.byte	0x1
	.byte	0x37
	.long	0x3fc
	.byte	0x9
	.byte	0x3
	.quad	conv_bv
	.uleb128 0x34
	.long	.LASF90
	.byte	0x1
	.byte	0x3a
	.long	0x3fc
	.byte	0x9
	.byte	0x3
	.quad	result
	.uleb128 0x34
	.long	.LASF136
	.byte	0x1
	.byte	0x3a
	.long	0x3fc
	.byte	0x9
	.byte	0x3
	.quad	spare
	.uleb128 0x34
	.long	.LASF137
	.byte	0x1
	.byte	0x3a
	.long	0x3fc
	.byte	0x9
	.byte	0x3
	.quad	op1static
	.uleb128 0x34
	.long	.LASF138
	.byte	0x1
	.byte	0x3a
	.long	0x3fc
	.byte	0x9
	.byte	0x3
	.quad	op2static
	.uleb128 0x34
	.long	.LASF139
	.byte	0x1
	.byte	0x3c
	.long	0xe66
	.byte	0x9
	.byte	0x3
	.quad	from_dec_data
	.uleb128 0x6
	.byte	0x8
	.long	0x42d
	.uleb128 0xe
	.long	0xe7c
	.long	0x4c7
	.uleb128 0xf
	.long	0x86
	.byte	0xf
	.byte	0x0
	.uleb128 0x34
	.long	.LASF140
	.byte	0x2
	.byte	0x38
	.long	0xe6c
	.byte	0x9
	.byte	0x3
	.quad	tests
	.uleb128 0xe
	.long	0xea2
	.long	0x95
	.uleb128 0x35
	.long	0x86
	.value	0x3e7
	.byte	0x0
	.uleb128 0x34
	.long	.LASF141
	.byte	0x2
	.byte	0x4f
	.long	0xe91
	.byte	0x9
	.byte	0x3
	.quad	failed
	.uleb128 0xe
	.long	0xec7
	.long	0x95
	.uleb128 0xf
	.long	0x86
	.byte	0x63
	.byte	0x0
	.uleb128 0x34
	.long	.LASF142
	.byte	0x2
	.byte	0x50
	.long	0xeb7
	.byte	0x9
	.byte	0x3
	.quad	failmsg
	.uleb128 0x36
	.long	.LASF143
	.byte	0x6
	.byte	0x8e
	.long	0x2b5
	.byte	0x1
	.byte	0x1
	.uleb128 0x36
	.long	.LASF144
	.byte	0x6
	.byte	0x8f
	.long	0x2b5
	.byte	0x1
	.byte	0x1
	.uleb128 0x37
	.long	0xf06
	.byte	0x1
	.long	0x8d
	.uleb128 0xb
	.long	0x2d
	.byte	0x0
	.uleb128 0x38
	.long	.LASF145
	.byte	0x3
	.value	0x117
	.long	0xf14
	.byte	0x1
	.byte	0x1
	.uleb128 0x6
	.byte	0x8
	.long	0xef6
	.uleb128 0x38
	.long	.LASF146
	.byte	0x3
	.value	0x132
	.long	0x25f
	.byte	0x1
	.byte	0x1
	.uleb128 0xa
	.long	0xf3e
	.byte	0x1
	.uleb128 0xb
	.long	0x254
	.uleb128 0xb
	.long	0x5b
	.uleb128 0xb
	.long	0x254
	.byte	0x0
	.uleb128 0x36
	.long	.LASF147
	.byte	0x9
	.byte	0x3f
	.long	0xf4b
	.byte	0x1
	.byte	0x1
	.uleb128 0x6
	.byte	0x8
	.long	0xf28
	.byte	0x0
	.section	.debug_abbrev
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x10
	.uleb128 0x6
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.byte	0x0
	.byte	0x0
	.uleb128 0x2
	.uleb128 0x16
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x4
	.uleb128 0x24
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x5
	.uleb128 0xf
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x6
	.uleb128 0xf
	.byte	0x0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x7
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x8
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x9
	.uleb128 0x26
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xa
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x27
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.uleb128 0xb
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xc
	.uleb128 0x16
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0xd
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0xe
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xf
	.uleb128 0x21
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x10
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x11
	.uleb128 0x4
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x12
	.uleb128 0x28
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1c
	.uleb128 0xd
	.byte	0x0
	.byte	0x0
	.uleb128 0x13
	.uleb128 0x13
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.uleb128 0x14
	.uleb128 0x17
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x15
	.uleb128 0xd
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x16
	.uleb128 0x2e
	.byte	0x0
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x17
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x18
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x19
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x1a
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x1b
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x1c
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x1d
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x1e
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x1f
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x20
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x21
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x22
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x23
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.uleb128 0x40
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x24
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x25
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x55
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x26
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x27
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.byte	0x0
	.byte	0x0
	.uleb128 0x28
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x29
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.byte	0x0
	.byte	0x0
	.uleb128 0x2a
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x2b
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x20
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0x2c
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x2d
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x2e
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x2f
	.uleb128 0x1d
	.byte	0x1
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x1
	.byte	0x0
	.byte	0x0
	.uleb128 0x30
	.uleb128 0x5
	.byte	0x0
	.uleb128 0x31
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x31
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x6
	.byte	0x0
	.byte	0x0
	.uleb128 0x32
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x31
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x33
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x31
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x34
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0xa
	.byte	0x0
	.byte	0x0
	.uleb128 0x35
	.uleb128 0x21
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0x5
	.byte	0x0
	.byte	0x0
	.uleb128 0x36
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.uleb128 0x37
	.uleb128 0x15
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x27
	.uleb128 0xc
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0x38
	.uleb128 0x34
	.byte	0x0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0x5
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3f
	.uleb128 0xc
	.uleb128 0x3c
	.uleb128 0xc
	.byte	0x0
	.byte	0x0
	.byte	0x0
	.section	.debug_pubnames,"",@progbits
	.long	0x27a
	.value	0x2
	.long	.Ldebug_info0
	.long	0xf52
	.long	0x4d2
	.string	"yasm_intnum_initialize"
	.long	0x4ef
	.string	"yasm_intnum_cleanup"
	.long	0x50c
	.string	"yasm_intnum_create_dec"
	.long	0x55f
	.string	"yasm_intnum_create_bin"
	.long	0x5b2
	.string	"yasm_intnum_create_oct"
	.long	0x605
	.string	"yasm_intnum_create_hex"
	.long	0x658
	.string	"yasm_intnum_create_charconst_nasm"
	.long	0x6ba
	.string	"yasm_intnum_create_uint"
	.long	0x6f8
	.string	"yasm_intnum_create_int"
	.long	0x736
	.string	"yasm_intnum_copy"
	.long	0x786
	.string	"yasm_intnum_destroy"
	.long	0x7b9
	.string	"yasm_intnum_calc"
	.long	0x84a
	.string	"yasm_intnum_zero"
	.long	0x87d
	.string	"yasm_intnum_is_zero"
	.long	0x8b1
	.string	"yasm_intnum_is_pos1"
	.long	0x8e5
	.string	"yasm_intnum_is_neg1"
	.long	0x91c
	.string	"yasm_intnum_sign"
	.long	0x952
	.string	"yasm_intnum_get_uint"
	.long	0x989
	.string	"yasm_intnum_get_int"
	.long	0x9d3
	.string	"yasm_intnum_check_size"
	.long	0xa8e
	.string	"yasm_intnum_get_sized"
	.long	0xb8c
	.string	"yasm_intnum_get_leb128"
	.long	0xc1d
	.string	"yasm_intnum_size_leb128"
	.long	0xc74
	.string	"yasm_intnum_print"
	.long	0xd36
	.string	"main"
	.long	0x0
	.section	.debug_aranges,"",@progbits
	.long	0x2c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x8
	.byte	0x0
	.value	0x0
	.value	0x0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	0x0
	.quad	0x0
	.section	.debug_ranges,"",@progbits
.Ldebug_ranges0:
	.quad	.LBB2-.Ltext0
	.quad	.LBE2-.Ltext0
	.quad	.LBB4-.Ltext0
	.quad	.LBE4-.Ltext0
	.quad	.LBB3-.Ltext0
	.quad	.LBE3-.Ltext0
	.quad	0x0
	.quad	0x0
	.quad	.LBB7-.Ltext0
	.quad	.LBE7-.Ltext0
	.quad	.LBB16-.Ltext0
	.quad	.LBE16-.Ltext0
	.quad	.LBB15-.Ltext0
	.quad	.LBE15-.Ltext0
	.quad	.LBB14-.Ltext0
	.quad	.LBE14-.Ltext0
	.quad	.LBB11-.Ltext0
	.quad	.LBE11-.Ltext0
	.quad	.LBB10-.Ltext0
	.quad	.LBE10-.Ltext0
	.quad	0x0
	.quad	0x0
	.quad	.LBB9-.Ltext0
	.quad	.LBE9-.Ltext0
	.quad	.LBB18-.Ltext0
	.quad	.LBE18-.Ltext0
	.quad	.LBB13-.Ltext0
	.quad	.LBE13-.Ltext0
	.quad	0x0
	.quad	0x0
	.section	.debug_str,"MS",@progbits,1
.LASF150:
	.string	"/home/pete/yasm"
.LASF139:
	.string	"from_dec_data"
.LASF31:
	.string	"_shortbuf"
.LASF137:
	.string	"op1static"
.LASF97:
	.string	"yasm_intnum_create_oct"
.LASF140:
	.string	"tests"
.LASF38:
	.string	"_IO_lock_t"
.LASF20:
	.string	"_IO_buf_end"
.LASF51:
	.string	"YASM_EXPR_DIV"
.LASF128:
	.string	"yasm_intnum_size_leb128"
.LASF46:
	.string	"origsize"
.LASF127:
	.string	"ptr_orig"
.LASF18:
	.string	"_IO_write_end"
.LASF4:
	.string	"unsigned int"
.LASF70:
	.string	"YASM_EXPR_GE"
.LASF12:
	.string	"_flags"
.LASF122:
	.string	"valsize"
.LASF67:
	.string	"YASM_EXPR_GT"
.LASF147:
	.string	"yasm_internal_error_"
.LASF24:
	.string	"_markers"
.LASF75:
	.string	"YASM_EXPR_SEGOFF"
.LASF6:
	.string	"short int"
.LASF73:
	.string	"YASM_EXPR_SEG"
.LASF95:
	.string	"yasm_intnum_create_bin"
.LASF72:
	.string	"YASM_EXPR_NONNUM"
.LASF78:
	.string	"wordptr"
.LASF74:
	.string	"YASM_EXPR_WRT"
.LASF102:
	.string	"yasm_intnum_copy"
.LASF125:
	.string	"warn"
.LASF120:
	.string	"yasm_intnum_get_sized"
.LASF43:
	.string	"_pos"
.LASF151:
	.string	"run_test"
.LASF144:
	.string	"stdout"
.LASF115:
	.string	"size"
.LASF100:
	.string	"yasm_intnum_create_uint"
.LASF123:
	.string	"shift"
.LASF65:
	.string	"YASM_EXPR_LNOT"
.LASF106:
	.string	"carry"
.LASF92:
	.string	"yasm_intnum_cleanup"
.LASF90:
	.string	"result"
.LASF71:
	.string	"YASM_EXPR_NE"
.LASF101:
	.string	"yasm_intnum_create_int"
.LASF112:
	.string	"yasm_intnum_get_uint"
.LASF149:
	.string	"libyasm/tests/leb128_test.c"
.LASF98:
	.string	"yasm_intnum_create_hex"
.LASF105:
	.string	"operand"
.LASF22:
	.string	"_IO_backup_base"
.LASF118:
	.string	"carry_in"
.LASF33:
	.string	"_offset"
.LASF119:
	.string	"retval"
.LASF26:
	.string	"_fileno"
.LASF7:
	.string	"size_t"
.LASF86:
	.string	"sign"
.LASF84:
	.string	"INTNUM_BV"
.LASF15:
	.string	"_IO_read_base"
.LASF124:
	.string	"bigendian"
.LASF23:
	.string	"_IO_save_end"
.LASF41:
	.string	"_next"
.LASF121:
	.string	"destsize"
.LASF47:
	.string	"YASM_EXPR_IDENT"
.LASF99:
	.string	"yasm_intnum_create_charconst_nasm"
.LASF135:
	.string	"conv_bv"
.LASF16:
	.string	"_IO_write_base"
.LASF50:
	.string	"YASM_EXPR_MUL"
.LASF56:
	.string	"YASM_EXPR_NOT"
.LASF54:
	.string	"YASM_EXPR_SIGNMOD"
.LASF36:
	.string	"_mode"
.LASF69:
	.string	"YASM_EXPR_LE"
.LASF111:
	.string	"yasm_intnum_sign"
.LASF94:
	.string	"yasm_intnum_create_dec"
.LASF66:
	.string	"YASM_EXPR_LT"
.LASF13:
	.string	"_IO_read_ptr"
.LASF126:
	.string	"yasm_intnum_get_leb128"
.LASF60:
	.string	"YASM_EXPR_NOR"
.LASF145:
	.string	"yasm_xmalloc"
.LASF52:
	.string	"YASM_EXPR_SIGNDIV"
.LASF59:
	.string	"YASM_EXPR_XOR"
.LASF104:
	.string	"yasm_intnum_calc"
.LASF40:
	.string	"_IO_marker"
.LASF138:
	.string	"op2static"
.LASF80:
	.string	"true"
.LASF21:
	.string	"_IO_save_base"
.LASF91:
	.string	"yasm_intnum_initialize"
.LASF62:
	.string	"YASM_EXPR_SHR"
.LASF87:
	.string	"negate"
.LASF83:
	.string	"INTNUM_UL"
.LASF133:
	.string	"numtests"
.LASF34:
	.string	"__pad1"
.LASF35:
	.string	"__pad2"
.LASF0:
	.string	"long unsigned int"
.LASF85:
	.string	"Test_Entry"
.LASF88:
	.string	"input"
.LASF107:
	.string	"yasm_intnum_zero"
.LASF30:
	.string	"_vtable_offset"
.LASF63:
	.string	"YASM_EXPR_LOR"
.LASF108:
	.string	"yasm_intnum_is_zero"
.LASF57:
	.string	"YASM_EXPR_OR"
.LASF76:
	.string	"yasm_expr_op"
.LASF141:
	.string	"failed"
.LASF48:
	.string	"YASM_EXPR_ADD"
.LASF142:
	.string	"failmsg"
.LASF10:
	.string	"char"
.LASF14:
	.string	"_IO_read_end"
.LASF114:
	.string	"yasm_intnum_check_size"
.LASF77:
	.string	"N_word"
.LASF143:
	.string	"stdin"
.LASF1:
	.string	"long int"
.LASF96:
	.string	"intn"
.LASF55:
	.string	"YASM_EXPR_NEG"
.LASF129:
	.string	"yasm_intnum_print"
.LASF49:
	.string	"YASM_EXPR_SUB"
.LASF131:
	.string	"valstr"
.LASF146:
	.string	"yasm_xfree"
.LASF32:
	.string	"_lock"
.LASF28:
	.string	"_old_offset"
.LASF39:
	.string	"_IO_FILE"
.LASF42:
	.string	"_sbuf"
.LASF136:
	.string	"spare"
.LASF103:
	.string	"yasm_intnum_destroy"
.LASF45:
	.string	"type"
.LASF109:
	.string	"yasm_intnum_is_pos1"
.LASF2:
	.string	"unsigned char"
.LASF89:
	.string	"outsize"
.LASF93:
	.string	"line"
.LASF130:
	.string	"test"
.LASF17:
	.string	"_IO_write_ptr"
.LASF79:
	.string	"false"
.LASF117:
	.string	"rangetype"
.LASF116:
	.string	"rshift"
.LASF58:
	.string	"YASM_EXPR_AND"
.LASF134:
	.string	"fail"
.LASF8:
	.string	"__off_t"
.LASF82:
	.string	"BitVector_from_Dec_static_data"
.LASF5:
	.string	"signed char"
.LASF3:
	.string	"short unsigned int"
.LASF110:
	.string	"yasm_intnum_is_neg1"
.LASF61:
	.string	"YASM_EXPR_SHL"
.LASF132:
	.string	"main"
.LASF25:
	.string	"_chain"
.LASF113:
	.string	"yasm_intnum_get_int"
.LASF11:
	.string	"FILE"
.LASF27:
	.string	"_flags2"
.LASF148:
	.string	"GNU C 4.0.2 (Debian 4.0.2-2)"
.LASF68:
	.string	"YASM_EXPR_EQ"
.LASF29:
	.string	"_cur_column"
.LASF64:
	.string	"YASM_EXPR_LAND"
.LASF9:
	.string	"__off64_t"
.LASF37:
	.string	"_unused2"
.LASF19:
	.string	"_IO_buf_base"
.LASF53:
	.string	"YASM_EXPR_MOD"
.LASF81:
	.string	"boolean"
.LASF44:
	.string	"yasm_intnum"
	.ident	"GCC: (GNU) 4.0.2 (Debian 4.0.2-2)"
	.section	.note.GNU-stack,"",@progbits
