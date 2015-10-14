[bits 32]
mov ax,[eax+ebx+ecx-eax]
mov ax,[eax+ecx+ebx-eax]
lea edi,[edi*8+eax+label]
lea edi,[eax+edi*8+label]
mov eax,[eax*2]
mov eax,[nosplit eax*2]
mov eax,[esi+ebp]
mov eax,[ebp+esi]
mov eax,[esi*1+ebp]
mov eax,[ebp*1+esi]
mov eax,[byte eax]
mov eax,[dword eax]
label
dd 5
label2
;mov ax,[eax+ebx*(label2-label)]	; not supported
mov ax,[eax*2+ebx*2-ebx]
