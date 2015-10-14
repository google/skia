%define BLA16 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16

%macro SUPERBLA16 16
mov eax, %1
mov eax, %2
mov eax, %3
mov eax, %4
mov eax, %5
mov eax, %6
mov eax, %7
mov eax, %8
mov eax, %9
mov eax, %10
mov eax, %11
mov eax, %12
mov eax, %13
mov eax, %14
mov eax, %15
mov eax, %16
%endmacro

SECTION .text

yoyo:

SUPERBLA16 BLA16

