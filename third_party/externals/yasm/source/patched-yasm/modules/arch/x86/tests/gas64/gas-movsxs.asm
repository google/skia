movsbl %al, %eax
movsbw %al, %ax
movswl %ax, %eax
movsbq %al, %rax
movswq %ax, %rax
movslq %eax, %rax
# Intel formats - untested for now
#movsxw %ax, %eax
#movsxb %al, %ax
#movsxb %al, %rax
#movsxw %ax, %rax
#movsxl %eax, %rax

movzbl %al, %eax
movzbw %al, %ax
movzwl %ax, %eax
movzbq %al, %rax
movzwq %ax, %rax

movsbw 5,%ax
movsbl 5,%eax
movswl 5,%eax
movsbq 5,%rax
movswq 5,%rax
movsx 5,%eax
movzbw 5,%ax
movzbl 5,%eax
movzwl 5,%eax
movzbq 5,%rax
movzwq 5,%rax
movzx 5,%eax

