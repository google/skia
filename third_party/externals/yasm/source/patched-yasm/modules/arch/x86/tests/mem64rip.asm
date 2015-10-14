[bits 64]
mov eax, [rip]
mov eax, [rip+2]
mov eax, [rip+sym]
mov eax, [sym wrt rip]
sym:
mov eax, [sym wrt rip]
call sym
