[bits 64]
l1:
inc dword [l2]
l2 equ 4-(l1-$$)
