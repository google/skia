[bits 16]
foo: a32 loop foo	; 67 E2 FD
bar: loop bar, ecx	; 67 E2 FD

[bits 32]
baz: a16 loop baz	; 67 E2 FD
qux: loop qux, cx	; 67 E2 FD
