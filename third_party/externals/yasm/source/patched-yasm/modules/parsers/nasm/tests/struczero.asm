struc MYSTRUC
.zero resd 0
endstruc

foo:
mov eax, [ecx+MYSTRUC.zero]
ret
