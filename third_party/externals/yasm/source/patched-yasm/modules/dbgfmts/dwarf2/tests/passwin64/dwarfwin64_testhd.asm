	.file	"test_hd.c"
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.text
.Ltext0:
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"Usage: %s <file>\n"
.LC1:
	.string	"rb"
.LC2:
	.string	"Could not open `%s'.\n"
.LC3:
	.string	"%02x \n"
.LC4:
	.string	"Error reading from `%s'.\n"
	.text
	.p2align 4,,15
.globl main
	.type	main, @function
main:
.LFB26:
	.file 1 "test_hd.c"
	.loc 1 33 0
.LVL0:
	movq	%rbp, -16(%rsp)
.LCFI0:
	movq	%rbx, -24(%rsp)
.LCFI1:
	movq	%rsi, %rbp
	movq	%r12, -8(%rsp)
.LCFI2:
	subq	$24, %rsp
.LCFI3:
.LVL1:
	.loc 1 37 0
	cmpl	$2, %edi
	je	.L2
.LVL2:
	.loc 1 38 0
	movq	(%rsi), %rdx
	movq	stderr(%rip), %rdi
.LVL3:
	movl	$.LC0, %esi
	xorl	%eax, %eax
	call	fprintf
	movl	$1, %eax
.LVL4:
.L4:
	.loc 1 59 0
	movq	(%rsp), %rbx
.LVL5:
	movq	8(%rsp), %rbp
.LVL6:
	movq	16(%rsp), %r12
	addq	$24, %rsp
.LVL7:
	ret
.LVL8:
	.p2align 4,,7
.L2:
	.loc 1 42 0
	movq	8(%rbp), %rdi
.LVL9:
	leaq	8(%rsi), %r12
	movl	$.LC1, %esi
	call	fopen
.LVL10:
	.loc 1 44 0
	testq	%rax, %rax
.LVL11:
	.loc 1 42 0
	movq	%rax, %rbx
.LVL12:
	.loc 1 44 0
	jne	.L12
	jmp	.L16
.LVL13:
	.p2align 4,,7
.L7:
	.loc 1 50 0
	movl	%eax, %esi
	movl	$.LC3, %edi
	xorl	%eax, %eax
.LVL14:
	call	printf
.LVL15:
.L12:
	.loc 1 49 0
	movq	%rbx, %rdi
	call	fgetc
.LVL16:
	cmpl	$-1, %eax
	jne	.L7
	.loc 1 52 0
	movq	%rbx, %rdi
	call	ferror
.LVL17:
	testl	%eax, %eax
	.p2align 4,,2
	jne	.L15
	.loc 1 57 0
	movq	%rbx, %rdi
	call	fclose
	.loc 1 59 0
	movq	(%rsp), %rbx
.LVL18:
	movq	8(%rsp), %rbp
.LVL19:
	.loc 1 57 0
	xorl	%eax, %eax
	.loc 1 59 0
	movq	16(%rsp), %r12
	addq	$24, %rsp
.LVL20:
	ret
.LVL21:
.L15:
	.loc 1 53 0
	movq	(%r12), %rdx
	movq	stderr(%rip), %rdi
	movl	$.LC4, %esi
	xorl	%eax, %eax
	call	fprintf
	.loc 1 59 0
	movq	(%rsp), %rbx
.LVL22:
	movq	8(%rsp), %rbp
.LVL23:
	.loc 1 53 0
	movl	$1, %eax
	.loc 1 59 0
	movq	16(%rsp), %r12
	addq	$24, %rsp
.LVL24:
	ret
.LVL25:
.L16:
	.loc 1 45 0
	movq	8(%rbp), %rdx
	movq	stderr(%rip), %rdi
	movl	$.LC2, %esi
	xorl	%eax, %eax
	call	fprintf
	movl	$1, %eax
	jmp	.L4
.LFE26:
	.size	main, .-main
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
	.quad	.LFB26
	.quad	.LFE26-.LFB26
	.byte	0x4
	.long	.LCFI1-.LFB26
	.byte	0x83
	.uleb128 0x4
	.byte	0x86
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI3-.LCFI1
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.align 8
.LEFDE0:
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
	.quad	.LFB26
	.quad	.LFE26-.LFB26
	.byte	0x4
	.long	.LCFI1-.LFB26
	.byte	0x83
	.uleb128 0x4
	.byte	0x86
	.uleb128 0x3
	.byte	0x4
	.long	.LCFI3-.LCFI1
	.byte	0xe
	.uleb128 0x20
	.byte	0x8c
	.uleb128 0x2
	.align 8
.LEFDE1:
	.file 2 "/usr/include/stdio.h"
	.file 3 "/usr/include/libio.h"
	.file 4 "/usr/include/bits/types.h"
	.text
.Letext0:
	.section	.debug_loc,"",@progbits
.Ldebug_loc0:
.LLST0:
	.quad	.LVL0-.Ltext0
	.quad	.LVL1-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL1-.Ltext0
	.quad	.LVL7-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL7-.Ltext0
	.quad	.LVL8-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL8-.Ltext0
	.quad	.LVL20-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL20-.Ltext0
	.quad	.LVL21-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL21-.Ltext0
	.quad	.LVL24-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	.LVL24-.Ltext0
	.quad	.LVL25-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 -24
	.quad	.LVL25-.Ltext0
	.quad	.LFE26-.Ltext0
	.value	0x2
	.byte	0x77
	.sleb128 0
	.quad	0x0
	.quad	0x0
.LLST1:
	.quad	.LVL0-.Ltext0
	.quad	.LVL3-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	.LVL8-.Ltext0
	.quad	.LVL9-.Ltext0
	.value	0x1
	.byte	0x55
	.quad	0x0
	.quad	0x0
.LLST2:
	.quad	.LVL0-.Ltext0
	.quad	.LVL2-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL2-.Ltext0
	.quad	.LVL6-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL8-.Ltext0
	.quad	.LVL19-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL21-.Ltext0
	.quad	.LVL23-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	.LVL25-.Ltext0
	.quad	.LFE26-.Ltext0
	.value	0x1
	.byte	0x56
	.quad	0x0
	.quad	0x0
.LLST3:
	.quad	.LVL4-.Ltext0
	.quad	.LVL5-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL10-.Ltext0
	.quad	.LVL11-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL12-.Ltext0
	.quad	.LVL18-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL21-.Ltext0
	.quad	.LVL22-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	.LVL25-.Ltext0
	.quad	.LFE26-.Ltext0
	.value	0x1
	.byte	0x53
	.quad	0x0
	.quad	0x0
.LLST4:
	.quad	.LVL13-.Ltext0
	.quad	.LVL14-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	.LVL14-.Ltext0
	.quad	.LVL15-.Ltext0
	.value	0x1
	.byte	0x54
	.quad	.LVL16-.Ltext0
	.quad	.LVL17-.Ltext0
	.value	0x1
	.byte	0x50
	.quad	0x0
	.quad	0x0
	.section	.debug_info
	.long	0x347
	.value	0x2
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.Ldebug_line0
	.quad	.Letext0
	.quad	.Ltext0
	.long	.LASF51
	.byte	0x1
	.long	.LASF52
	.long	.LASF53
	.uleb128 0x2
	.long	.LASF0
	.byte	0x8
	.byte	0x7
	.uleb128 0x2
	.long	.LASF1
	.byte	0x1
	.byte	0x8
	.uleb128 0x2
	.long	.LASF2
	.byte	0x2
	.byte	0x7
	.uleb128 0x2
	.long	.LASF3
	.byte	0x4
	.byte	0x7
	.uleb128 0x2
	.long	.LASF4
	.byte	0x1
	.byte	0x6
	.uleb128 0x2
	.long	.LASF5
	.byte	0x2
	.byte	0x5
	.uleb128 0x3
	.string	"int"
	.byte	0x4
	.byte	0x5
	.uleb128 0x2
	.long	.LASF6
	.byte	0x8
	.byte	0x5
	.uleb128 0x4
	.long	.LASF7
	.byte	0x4
	.byte	0x8f
	.long	0x5e
	.uleb128 0x4
	.long	.LASF8
	.byte	0x4
	.byte	0x90
	.long	0x5e
	.uleb128 0x2
	.long	.LASF0
	.byte	0x8
	.byte	0x7
	.uleb128 0x5
	.byte	0x8
	.uleb128 0x6
	.byte	0x8
	.long	0x8a
	.uleb128 0x2
	.long	.LASF9
	.byte	0x1
	.byte	0x6
	.uleb128 0x4
	.long	.LASF10
	.byte	0x2
	.byte	0x2e
	.long	0x9c
	.uleb128 0x7
	.long	0x238
	.long	.LASF38
	.byte	0xd8
	.byte	0x2
	.byte	0x2e
	.uleb128 0x8
	.long	.LASF11
	.byte	0x3
	.value	0x10c
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0x8
	.long	.LASF12
	.byte	0x3
	.value	0x111
	.long	0x84
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0x8
	.long	.LASF13
	.byte	0x3
	.value	0x112
	.long	0x84
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.uleb128 0x8
	.long	.LASF14
	.byte	0x3
	.value	0x113
	.long	0x84
	.byte	0x2
	.byte	0x23
	.uleb128 0x18
	.uleb128 0x8
	.long	.LASF15
	.byte	0x3
	.value	0x114
	.long	0x84
	.byte	0x2
	.byte	0x23
	.uleb128 0x20
	.uleb128 0x8
	.long	.LASF16
	.byte	0x3
	.value	0x115
	.long	0x84
	.byte	0x2
	.byte	0x23
	.uleb128 0x28
	.uleb128 0x8
	.long	.LASF17
	.byte	0x3
	.value	0x116
	.long	0x84
	.byte	0x2
	.byte	0x23
	.uleb128 0x30
	.uleb128 0x8
	.long	.LASF18
	.byte	0x3
	.value	0x117
	.long	0x84
	.byte	0x2
	.byte	0x23
	.uleb128 0x38
	.uleb128 0x8
	.long	.LASF19
	.byte	0x3
	.value	0x118
	.long	0x84
	.byte	0x2
	.byte	0x23
	.uleb128 0x40
	.uleb128 0x8
	.long	.LASF20
	.byte	0x3
	.value	0x11a
	.long	0x84
	.byte	0x2
	.byte	0x23
	.uleb128 0x48
	.uleb128 0x8
	.long	.LASF21
	.byte	0x3
	.value	0x11b
	.long	0x84
	.byte	0x2
	.byte	0x23
	.uleb128 0x50
	.uleb128 0x8
	.long	.LASF22
	.byte	0x3
	.value	0x11c
	.long	0x84
	.byte	0x2
	.byte	0x23
	.uleb128 0x58
	.uleb128 0x8
	.long	.LASF23
	.byte	0x3
	.value	0x11e
	.long	0x276
	.byte	0x2
	.byte	0x23
	.uleb128 0x60
	.uleb128 0x8
	.long	.LASF24
	.byte	0x3
	.value	0x120
	.long	0x27c
	.byte	0x2
	.byte	0x23
	.uleb128 0x68
	.uleb128 0x8
	.long	.LASF25
	.byte	0x3
	.value	0x122
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x70
	.uleb128 0x8
	.long	.LASF26
	.byte	0x3
	.value	0x126
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x74
	.uleb128 0x8
	.long	.LASF27
	.byte	0x3
	.value	0x128
	.long	0x65
	.byte	0x2
	.byte	0x23
	.uleb128 0x78
	.uleb128 0x8
	.long	.LASF28
	.byte	0x3
	.value	0x12c
	.long	0x3b
	.byte	0x3
	.byte	0x23
	.uleb128 0x80
	.uleb128 0x8
	.long	.LASF29
	.byte	0x3
	.value	0x12d
	.long	0x49
	.byte	0x3
	.byte	0x23
	.uleb128 0x82
	.uleb128 0x8
	.long	.LASF30
	.byte	0x3
	.value	0x12e
	.long	0x282
	.byte	0x3
	.byte	0x23
	.uleb128 0x83
	.uleb128 0x8
	.long	.LASF31
	.byte	0x3
	.value	0x132
	.long	0x292
	.byte	0x3
	.byte	0x23
	.uleb128 0x88
	.uleb128 0x8
	.long	.LASF32
	.byte	0x3
	.value	0x13b
	.long	0x70
	.byte	0x3
	.byte	0x23
	.uleb128 0x90
	.uleb128 0x8
	.long	.LASF33
	.byte	0x3
	.value	0x141
	.long	0x82
	.byte	0x3
	.byte	0x23
	.uleb128 0x98
	.uleb128 0x8
	.long	.LASF34
	.byte	0x3
	.value	0x142
	.long	0x82
	.byte	0x3
	.byte	0x23
	.uleb128 0xa0
	.uleb128 0x8
	.long	.LASF35
	.byte	0x3
	.value	0x144
	.long	0x57
	.byte	0x3
	.byte	0x23
	.uleb128 0xa8
	.uleb128 0x8
	.long	.LASF36
	.byte	0x3
	.value	0x146
	.long	0x298
	.byte	0x3
	.byte	0x23
	.uleb128 0xac
	.byte	0x0
	.uleb128 0x9
	.long	.LASF37
	.byte	0x3
	.byte	0xb0
	.uleb128 0x7
	.long	0x276
	.long	.LASF39
	.byte	0x18
	.byte	0x3
	.byte	0xb6
	.uleb128 0xa
	.long	.LASF40
	.byte	0x3
	.byte	0xb7
	.long	0x276
	.byte	0x2
	.byte	0x23
	.uleb128 0x0
	.uleb128 0xa
	.long	.LASF41
	.byte	0x3
	.byte	0xb8
	.long	0x27c
	.byte	0x2
	.byte	0x23
	.uleb128 0x8
	.uleb128 0xa
	.long	.LASF42
	.byte	0x3
	.byte	0xbc
	.long	0x57
	.byte	0x2
	.byte	0x23
	.uleb128 0x10
	.byte	0x0
	.uleb128 0x6
	.byte	0x8
	.long	0x23f
	.uleb128 0x6
	.byte	0x8
	.long	0x9c
	.uleb128 0xb
	.long	0x292
	.long	0x8a
	.uleb128 0xc
	.long	0x7b
	.byte	0x0
	.byte	0x0
	.uleb128 0x6
	.byte	0x8
	.long	0x238
	.uleb128 0xb
	.long	0x2a8
	.long	0x8a
	.uleb128 0xc
	.long	0x7b
	.byte	0x2b
	.byte	0x0
	.uleb128 0x2
	.long	.LASF43
	.byte	0x8
	.byte	0x7
	.uleb128 0x2
	.long	.LASF44
	.byte	0x8
	.byte	0x5
	.uleb128 0xd
	.long	0x317
	.byte	0x1
	.long	.LASF54
	.byte	0x1
	.byte	0x21
	.byte	0x1
	.long	0x57
	.quad	.LFB26
	.quad	.LFE26
	.long	.LLST0
	.uleb128 0xe
	.long	.LASF45
	.byte	0x1
	.byte	0x20
	.long	0x57
	.long	.LLST1
	.uleb128 0xe
	.long	.LASF46
	.byte	0x1
	.byte	0x20
	.long	0x317
	.long	.LLST2
	.uleb128 0xf
	.long	.LASF47
	.byte	0x1
	.byte	0x22
	.long	0x31d
	.long	.LLST3
	.uleb128 0x10
	.string	"ch"
	.byte	0x1
	.byte	0x23
	.long	0x57
	.long	.LLST4
	.byte	0x0
	.uleb128 0x6
	.byte	0x8
	.long	0x84
	.uleb128 0x6
	.byte	0x8
	.long	0x91
	.uleb128 0x11
	.long	.LASF48
	.byte	0x2
	.byte	0x8e
	.long	0x27c
	.byte	0x1
	.byte	0x1
	.uleb128 0x11
	.long	.LASF49
	.byte	0x2
	.byte	0x8f
	.long	0x27c
	.byte	0x1
	.byte	0x1
	.uleb128 0x11
	.long	.LASF50
	.byte	0x2
	.byte	0x90
	.long	0x27c
	.byte	0x1
	.byte	0x1
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
	.uleb128 0x3
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
	.uleb128 0x4
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
	.uleb128 0xa
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
	.uleb128 0xb
	.uleb128 0x1
	.byte	0x1
	.uleb128 0x1
	.uleb128 0x13
	.uleb128 0x49
	.uleb128 0x13
	.byte	0x0
	.byte	0x0
	.uleb128 0xc
	.uleb128 0x21
	.byte	0x0
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2f
	.uleb128 0xb
	.byte	0x0
	.byte	0x0
	.uleb128 0xd
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
	.uleb128 0xe
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
	.uleb128 0xf
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
	.uleb128 0x10
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
	.uleb128 0x11
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
	.byte	0x0
	.section	.debug_pubnames,"",@progbits
	.long	0x17
	.value	0x2
	.long	.Ldebug_info0
	.long	0x34b
	.long	0x2b6
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
	.section	.debug_str,"MS",@progbits,1
.LASF7:
	.string	"__off_t"
.LASF12:
	.string	"_IO_read_ptr"
.LASF24:
	.string	"_chain"
.LASF30:
	.string	"_shortbuf"
.LASF4:
	.string	"signed char"
.LASF18:
	.string	"_IO_buf_base"
.LASF43:
	.string	"long long unsigned int"
.LASF44:
	.string	"long long int"
.LASF47:
	.string	"bfile"
.LASF25:
	.string	"_fileno"
.LASF13:
	.string	"_IO_read_end"
.LASF6:
	.string	"long int"
.LASF11:
	.string	"_flags"
.LASF19:
	.string	"_IO_buf_end"
.LASF28:
	.string	"_cur_column"
.LASF27:
	.string	"_old_offset"
.LASF32:
	.string	"_offset"
.LASF54:
	.string	"main"
.LASF48:
	.string	"stdin"
.LASF3:
	.string	"unsigned int"
.LASF0:
	.string	"long unsigned int"
.LASF52:
	.string	"test_hd.c"
.LASF16:
	.string	"_IO_write_ptr"
.LASF41:
	.string	"_sbuf"
.LASF2:
	.string	"short unsigned int"
.LASF31:
	.string	"_lock"
.LASF49:
	.string	"stdout"
.LASF26:
	.string	"_flags2"
.LASF35:
	.string	"_mode"
.LASF10:
	.string	"FILE"
.LASF20:
	.string	"_IO_save_base"
.LASF17:
	.string	"_IO_write_end"
.LASF53:
	.string	"/home/pete/yasm"
.LASF37:
	.string	"_IO_lock_t"
.LASF38:
	.string	"_IO_FILE"
.LASF39:
	.string	"_IO_marker"
.LASF42:
	.string	"_pos"
.LASF23:
	.string	"_markers"
.LASF1:
	.string	"unsigned char"
.LASF5:
	.string	"short int"
.LASF29:
	.string	"_vtable_offset"
.LASF9:
	.string	"char"
.LASF51:
	.string	"GNU C 4.0.2 (Debian 4.0.2-2)"
.LASF40:
	.string	"_next"
.LASF8:
	.string	"__off64_t"
.LASF14:
	.string	"_IO_read_base"
.LASF22:
	.string	"_IO_save_end"
.LASF33:
	.string	"__pad1"
.LASF34:
	.string	"__pad2"
.LASF36:
	.string	"_unused2"
.LASF50:
	.string	"stderr"
.LASF46:
	.string	"argv"
.LASF21:
	.string	"_IO_backup_base"
.LASF45:
	.string	"argc"
.LASF15:
	.string	"_IO_write_base"
	.ident	"GCC: (GNU) 4.0.2 (Debian 4.0.2-2)"
	.section	.note.GNU-stack,"",@progbits
