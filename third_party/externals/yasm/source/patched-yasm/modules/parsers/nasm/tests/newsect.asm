[absolute 0]
mytype:
.long resd 1
mytype_size:
[section .text]
lbl:
..@6.strucstart:
times mytype.long-($-..@6.strucstart) db 0
dd 'ABCD'
times mytype_size-($-..@6.strucstart) db 0
