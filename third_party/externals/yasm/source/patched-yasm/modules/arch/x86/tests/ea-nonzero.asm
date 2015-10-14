; Ticket #58
; Also tests that this isn't seen as a circular reference.
[bits 64]
a:
lea rbp,[rsi+rbp*1+(b-a)]
b:
