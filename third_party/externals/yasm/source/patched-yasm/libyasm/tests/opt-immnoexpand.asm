label1:
times 2 nop
je label3
times 123 nop
label2:
je label4
label3:
times 128 nop
label4:
push label2-label1
