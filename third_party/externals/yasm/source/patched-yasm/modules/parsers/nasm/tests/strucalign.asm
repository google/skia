struc bug
times (64-$) resb 1
.member:
times (128-($-$$)) resb 1
.member2:
alignb 256
.member3:
[align 512]
endstruc
dd bug
dd bug.member
dd bug.member2
dd bug.member3
dd bug_size
