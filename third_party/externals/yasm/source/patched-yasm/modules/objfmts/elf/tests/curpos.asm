global bar
global foo

section .bar
bar:
dd foo-$
dd baz-$
call foo
call baz
foo:

section .data
baz:
dd foo-$
;dd $-foo	; illegal
dd baz-$
dd $-baz
dd foo+4-$		; with constant
dd $-baz+foo+4-$	; both local and cross-segment (legal)
dd baz+foo+4-$-$	; ditto, slightly different
;dd (bar-$)+(foo-$)	; illegal (too many cross-segment)
dd baz-$+baz-$		; two from same segment

section .text
mov dword [foo-$], 5
mov eax, foo-$
call foo
