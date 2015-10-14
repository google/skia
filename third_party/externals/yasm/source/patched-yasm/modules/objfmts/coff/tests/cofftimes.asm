[section .text]
mov eax, eax
mov ebx, ebx
[section .data]
times 0x1 mov eax, eax
mov ebx, ebx
[section .foo]
times 0x10 mov eax, eax
mov ebx, ebx
[section .bar]
times 0x10 mov eax, eax
times 0x10 mov ebx, ebx
