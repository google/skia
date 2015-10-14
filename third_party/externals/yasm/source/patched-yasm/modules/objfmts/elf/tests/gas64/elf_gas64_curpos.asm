.globl bar
.globl foo

.section .bar
bar:
.4byte foo-.
.4byte baz-.
call foo
call baz
foo:

.section .data
baz:
.4byte foo-.
#.4byte .-foo	# illegal
.4byte baz-.
.4byte .-baz
.4byte foo+4-.		# with constant
.4byte .-baz+foo+4-.	# both local and cross-segment (legal)
#.4byte baz+foo+4-.-.	# ditto, slightly different - GAS gets confused on this
#.4byte (bar-.)+(foo-.)	# illegal (too many cross-segment)
.4byte baz-.+baz-.	# two from same segment

.section .text
movl $5, foo-.
movl $(foo-.), %eax
call foo
