.section .rodata
L2586:
.ascii "myWindowClass\0"
.globl _g_szClassName
_g_szClassName:
.byte 109
.byte 121
.byte 87
.byte 105
.byte 110
.byte 100
.byte 111
.byte 119
.byte 67
.byte 108
.byte 97
.byte 115
.byte 115
.byte 0
.text
.align 4
.globl _WndProc@16
_WndProc@16:
pushl %ebp
movl %esp,%ebp
subl $8,%esp
L2588:
L2590:
movl 12(%ebp),%eax
movl %eax,-4(%ebp)
jmp L2592
L2593:
pushl 8(%ebp)
call _DestroyWindow@4
jmp L2591
L2594:
pushl $0
call _PostQuitMessage@4
jmp L2591
L2595:
pushl 20(%ebp)
pushl 16(%ebp)
pushl 12(%ebp)
pushl 8(%ebp)
call _DefWindowProcA@16
movl %eax,-8(%ebp)
jmp L2589
L2592:
cmpl $2,-4(%ebp)
je L2594
cmpl $16,-4(%ebp)
je L2593
jmp L2595
L2591:
movl $0,-8(%ebp)
jmp L2589
L2589:
movl -8(%ebp),%eax
leave
ret $16
.section .rodata
L2600:
.ascii "Window Registration Failed!\0"
L2601:
.ascii "Error!\0"
L2602:
.ascii "The title of my window\0"
L2604:
.ascii "Window Creation Failed!\0"
.text
.align 4
.globl _WinMain@16
_WinMain@16:
pushl %ebp
movl %esp,%ebp
subl $84,%esp
L2596:
L2598:
movl $48,-48(%ebp)
movl $0,-44(%ebp)
movl $_WndProc@16,-40(%ebp)
movl $0,-36(%ebp)
movl $0,-32(%ebp)
movl 8(%ebp),%eax
movl %eax,-28(%ebp)
pushl $32512
pushl $0
call _LoadIconA@8
movl %eax,-24(%ebp)
pushl $32512
pushl $0
call _LoadCursorA@8
movl %eax,-20(%ebp)
movl $6,-16(%ebp)
movl $0,-12(%ebp)
movl $_g_szClassName,-8(%ebp)
pushl $32512
pushl $0
call _LoadIconA@8
movl %eax,-4(%ebp)
leal -48(%ebp),%edx
pushl %edx
call _RegisterClassExA@4
cmpw $0,%ax
jne L2599
pushl $48
pushl $L2601
pushl $L2600
pushl $0
call _MessageBoxA@16
movl $0,-84(%ebp)
jmp L2597
L2599:
pushl $0
pushl 8(%ebp)
pushl $0
pushl $0
pushl $120
pushl $240
pushl $-2147483648
pushl $-2147483648
pushl $13565952
pushl $L2602
pushl $_g_szClassName
pushl $512
call _CreateWindowExA@48
movl %eax,-52(%ebp)
cmpl $0,-52(%ebp)
jne L2603
pushl $48
pushl $L2601
pushl $L2604
pushl $0
call _MessageBoxA@16
movl $0,-84(%ebp)
jmp L2597
L2603:
pushl 20(%ebp)
pushl -52(%ebp)
call _ShowWindow@8
pushl -52(%ebp)
call _UpdateWindow@4
L2605:
pushl $0
pushl $0
pushl $0
leal -80(%ebp),%edx
pushl %edx
call _GetMessageA@16
cmpl $0,%eax
jle L2606
leal -80(%ebp),%eax
pushl %eax
call _TranslateMessage@4
leal -80(%ebp),%eax
pushl %eax
call _DispatchMessageA@4
jmp L2605
L2606:
movl -72(%ebp),%eax
movl %eax,-84(%ebp)
jmp L2597
L2597:
movl -84(%ebp),%eax
leave
ret $16
.ident "PCC: pcc 0.9.9 (win32)"
