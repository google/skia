label:
mov [label/2+1], ax
mov ax, label*2
mov [label+5], ax
mov ax, label wrt foo
foo:
dd label
dd label<<5
dd label>>2
