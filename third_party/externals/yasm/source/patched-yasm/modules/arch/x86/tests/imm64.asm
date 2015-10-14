bits 64
default abs
;extern label1
label1:
label2:

mov rax, 0x1000              ; 32-bit imm
mov rax, 0x1122334455667788  ; 64-bit imm (larger than signed 32-bit)
;mov rax, 0x80000000          ; 64-bit imm (larger than signed 32-bit)
mov rax, label1              ; 32-bit imm  <--- not 64-bit!
mov rax, label2              ; 32-bit imm  <--- not 64-bit!
mov rax, qword 0x1000        ; 64-bit imm
mov rax, qword label1        ; 64-bit imm
mov rax, qword label2        ; 64-bit imm

mov qword [rax], 0x1000      ; 32-bit imm
mov qword [rax], 0x1122334455667788  ; 32-bit imm, overflow warning
;mov qword [rax], 0x80000000  ; 32-bit imm, overflow warning
mov qword [rax], label1      ; 32-bit imm (matches default above)
mov qword [rax], label2      ; 32-bit imm (matches default above)

add rax, 0x1000              ; 32-bit imm
add rax, 0x1122334455667788  ; 32-bit imm, overflow warning
;add rax, 0x80000000          ; 32-bit imm, overflow warning
add rax, label1              ; 32-bit imm (matches default above)
add rax, label2              ; 32-bit imm (matches default above)

mov [0x1000], rax            ; 32-bit disp
mov [abs 0x1122334455667788], rax ; 64-bit disp
mov [label1], rax            ; 32-bit disp
mov [label2], rax            ; 32-bit disp
mov [qword 0x1000], rax      ; 64-bit disp
mov [qword label1], rax      ; 64-bit disp
mov [qword label2], rax      ; 64-bit disp

