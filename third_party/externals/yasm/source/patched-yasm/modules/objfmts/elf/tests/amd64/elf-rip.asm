[bits 64]
[extern sym]
mov eax, [rip]
mov eax, [rip+2]
mov eax, [rip+sym]
mov eax, [sym wrt rip]
call sym
