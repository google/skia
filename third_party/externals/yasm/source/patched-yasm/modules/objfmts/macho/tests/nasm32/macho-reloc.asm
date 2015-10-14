[SECTION .data]

uhoh db 5

[GLOBAL blah]

blah dw 5
[SECTION .text]

[EXTERN hi]
[EXTERN hi]
[EXTERN bye]
	mov eax, hi+2
	mov eax, bye
	mov eax, [hi]
	mov eax, [bye+2]
	mov eax, $$
	mov eax, $
	mov eax, $+4
	mov eax, $-$$
;mov eax, uhoh wrt $$
;mov eax, hi+bye
;mov eax, bye+$
;mov eax, hi-$
