struc TST1
    .a resd 2
endstruc

struc TST2
    .b resb TST1_size
endstruc

tst2:
istruc TST2
at TST2.b

	istruc TST1

	at TST1.a
		dd 1, 2

	iend

iend

dw TST1_size
dw TST2_size
