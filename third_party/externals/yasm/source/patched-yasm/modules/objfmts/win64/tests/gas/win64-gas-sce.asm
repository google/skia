PROC_FRAME sample
rex_push_reg %rbp
rex_push_eflags
alloc_stack 16
save_reg %rsi, 0x18
save_xmm128 %xmm7, 0x20
push_frame 16
set_frame %rdi
set_frame %rdi, 16
END_PROLOGUE
ENDPROC_FRAME
