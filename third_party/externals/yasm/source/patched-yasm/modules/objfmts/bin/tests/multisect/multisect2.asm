org 100h
[map all]

section .bss ; follows=.data
     buffer resb 123h
section .data
     msg db "this is a message", 0
section .text
     mov ax, msg
     call showax
     mov ax, buffer
     call showax
     ret

;-----------------
showax:
     push cx
     push dx

     mov cx, 4           ; four digits to show

.top
     rol ax, 4           ; rotate one digit into position
     mov dl, al          ; make a copy to process
     and dl, 0Fh         ; mask off a single (hex) digit
     cmp dl, 9           ; is it in the "A" to "F" range?
     jbe .dec_dig        ; no, skip it
     add dl, 7           ; adjust
.dec_dig:
     add dl, 30h         ; convert to character

     push ax
     mov ah, 2
     int 21h
     pop ax

     loop .top

     pop dx
     pop cx
     ret
;--------------------------
