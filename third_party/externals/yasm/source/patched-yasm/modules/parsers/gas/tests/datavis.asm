#.file "string"
.extern extsym2
.global label1
.globl label2
.text
	call	extsym
	call	extsym2
.data
label1:
.ascii "baz"
.asciz "foo"
.string "bar", "str2"
.byte 1, 2, 070, 0x12
.byte

.section .xx, "", @ progbits
.hword 0
.short 5
.word 10
.long 15
.int 30
.quad 20
.octa 40

.section .yy, "w", @nobits
.skip 4
.skip 4, 1

.bss
label2:
.skip 4
.skip 4, 1

.section .data2
.float 0E1e10
.double 0E-1.11
.skip 4
.skip 4, 1
.tfloat 0E+3.14e2
