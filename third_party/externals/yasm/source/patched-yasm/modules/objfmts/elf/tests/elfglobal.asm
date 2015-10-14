; Note: you should be able to link elfreloc.o with elfglobal.o to make a
; program that calls function with eax=constant, thus exiting err=0
GLOBAL constant
GLOBAL function

constant EQU 48

function:
	sub	eax, constant
	ret
