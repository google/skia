# test source file for assembling to MACH-O 
# build with :
#    yasm -f macho machotest.asm
#    gcc -o machotest machotest.c machotest.o

# This file should test the following:
# [1] Define and export a global text-section symbol
# [2] Define and export a global data-section symbol
# [3] Define and export a global BSS-section symbol
# [4] Define a non-global text-section symbol
# [5] Define a non-global data-section symbol
# [6] Define a non-global BSS-section symbol
# [7] Define a COMMON symbol
# [8] Define a NASM local label
# [9] Reference a NASM local label
# [10] Import an external symbol (note: printf replaced by another call)
# [11] Make a PC-relative call to an external symbol
# [12] Reference a text-section symbol in the text section
# [13] Reference a data-section symbol in the text section
# [14] Reference a BSS-section symbol in the text section
# [15] Reference a text-section symbol in the data section
# [16] Reference a data-section symbol in the data section
# [17] Reference a BSS-section symbol in the data section

.globl _lrotate	# [1]
.globl _greet		# [1]
.globl _asmstr		# [2]
.globl _textptr	# [2]
.globl _selfptr	# [2]
.globl _integer	# [3]
#.extern _druck		# [10]
.comm _commvar, 4	# [7]

.text

# prototype: long lrotate(long x, int num);
_lrotate:			# [1]
	  pushl %ebp
	  movl %esp, %ebp
	  movl 8(%ebp), %eax
	  movl 12(%ebp), %ecx
Llabel:	  roll %eax		# [4] [8]
	  loop Llabel		# [9] [12]
	  movl %ebp, %esp
	  popl %ebp
	  ret

# prototype: void greet(void);
_greet:
	  movl _integer, %eax	# [14]
	  incl %eax
	  movl %eax, localint	# [14]
	  pushl _commvar
	  movl localptr, %eax	# [13]
	  pushl (%eax)
	  pushl _integer	# [1] [14]
	  pushl _printfstr	# [13]
	  calll _druck		# [11]
	  addl 16, %esp
	  ret

.data

# a string
_asmstr: .asciz "hello, world"	# [2]

# a string for Printf 
_printfstr: .asciz "integer==%d, localint==%d, commvar=%d\n"

# some pointers
localptr:  .long localint	# [5] [17]
_textptr:  .long _greet		# [15]
_selfptr:  .long _selfptr	# [16]

# an integer
.lcomm _integer, 4		# [3]

# a local integer
.lcomm localint, 4		# [6]
