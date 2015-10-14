; This worked under NASM.  Due to the once-parse model used by YASM, this no
; longer works.  However, it should error, not crash!
teststring db "Hello, world"
teststringlen equ $-teststring
%if teststringlen>100
 db '5'
%endif
