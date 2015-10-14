%macro TESTMAC 0
label:
	mov ax, 5
	mov dx, 4
	mov cx, 3
%endmacro

db 6

db 7

TESTMAC

db 8
db 9
TESTMAC
db 10
TESTMAC
