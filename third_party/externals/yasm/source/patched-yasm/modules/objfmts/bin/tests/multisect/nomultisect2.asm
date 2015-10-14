org 0x100

[map all]

[section .bss]
resb 0x100

[section .data]
times 0x100 db 1

[section .text]
times 0x100 db 1
