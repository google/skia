; program to test retrieval of and linkage to modules in libraries by
; ldrdf

[SECTION .text]
[GLOBAL _main]
[EXTERN _strcmp]

_main:
	push dword string1
	push dword string2
	call _strcmp
	add esp,8		; doh! clear up stack ;-)
	ret

[SECTION .data]

string1:	db 'abc',0	; try changing these strings and see
string2:	db 'abd',0	; what happens!
