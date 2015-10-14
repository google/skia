[absolute 0f0000000h]
foo: resb 1
[section .data]
bar dd foo
baz db (foo>>24)
