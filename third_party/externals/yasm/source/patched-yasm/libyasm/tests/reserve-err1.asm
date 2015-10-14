; Test res* family errors
a:
resb -5
resw 1.2
resd -1.2
resq 0xffffffff
rest a

[section .bss]
resb -5
resw 1.2
resd -1.2
resq 0xffffffff
rest a

