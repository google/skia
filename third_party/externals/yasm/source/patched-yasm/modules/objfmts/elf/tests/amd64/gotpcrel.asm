var:
mov rax, [var wrt ..got]
mov rax, [var wrt ..gotpcrel]		; should be error/warning?
mov rax, [rel var wrt ..got]		; automatic promotion to GOTPCREL
mov rax, [rel var wrt ..gotpcrel]

