[bits 32]
add [byte  ebp*8+06h],ecx ;db 01,0c,0ed,06 probably wrong
dd 90909090h
add [dword ebp*8+06h],ecx ;db 01,0c,0ed,06,0,0,0  OK
dd 90909090h
add ecx,[byte  ebp*8+06h] ;db 03,0c,0ed,06 probably wrong
dd 90909090h
add ecx,[dword ebp*8+06h]
dd 90909090h
add ecx,[ebp*8+06h]
dd 90909090h
add ecx,[byte ebx*8+06h]  ;db 03,0c,0dd,06 probably wrong
dd 90909090h
add ecx,[dword ebx*8+06h]
dd 90909090h
add ecx,[ebx*8+06h]
dd 90909090h

