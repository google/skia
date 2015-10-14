extern var4
extern farlabel
global equ2

equ1 equ 4000
equ2 equ 5000

[section foo]
mov ax, bar
mov ds, ax
mov ax, [var2 wrt bar]

mov ax, seg var4
mov es, ax
mov ax, [es:var4]

[section bar]
jmp far farlabel

[section baz]
var2 dw 2

[section bss bss]
var3 resd 4
