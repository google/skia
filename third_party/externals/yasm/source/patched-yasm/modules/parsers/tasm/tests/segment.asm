data segment
  a db 0
data ends

assume es:data
code segment
  mov byte ptr [a],1
code ends
