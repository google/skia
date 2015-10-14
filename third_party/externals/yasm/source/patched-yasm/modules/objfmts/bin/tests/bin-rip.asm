bits 64
org 0x100
foo_equ equ 0x12345678

section .text
nop
foo_text:
nop
mov rax,[foo_equ wrt rip]
mov rax,[foo_text wrt rip]
mov rax,[foo_data wrt rip]
mov rbx,[foo_equ wrt rip]
mov rbx,[foo_text wrt rip]
mov rbx,[foo_data wrt rip]

section .data
db 0
foo_data:
db 0

