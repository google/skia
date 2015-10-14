; all of these should be legal and should just result in the offset portion

foo equ 1:2
jmp far[foo]
mov ax,[foo]
push dword [foo]
mov ax,foo
