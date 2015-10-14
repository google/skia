module thismodule
module $thismodule
global foo:export
global bar:export proc
global bar2:export function
global baz:export data
global baz2:export object
extern extvar:import
extern func:proc
extern farfunc:far
library alib.rdl
common cvar 16:32

foo:
dd 0
bar:
dd 0
bar2:
dd 0
call func
call farfunc		; generates a near call!
call far farfunc

mov ax, seg farfunc
mov ax, farfunc
mov eax, farfunc

mov eax, cvar

section .data
baz:
dd 0
baz2:
dd 0

section .bss
resb 4

;section a null

section b text
dd 0

section c code
dd 0

section d data
dd 0

section e comment,5		; after comma is reserved value
dd 0

section f lcomment
dd 0

section g pcomment,8
dd 0

section h symdebug
dd 0

section i linedebug
dd 0
