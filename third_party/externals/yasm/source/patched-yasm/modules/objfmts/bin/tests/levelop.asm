test1:
mov word [0x0010 + (test2 - test1)], 0x0000
mov word [0x0010 + test2 - test1], 0x0000
test2:
