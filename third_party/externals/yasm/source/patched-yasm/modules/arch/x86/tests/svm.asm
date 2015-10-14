[bits 64]
rdtscp
clgi
invlpga
invlpga [rax], ecx
invlpga [eax], ecx
; invlpga [ax], ecx ; invalid
skinit
; skinit [rax] ; invalid
skinit [eax]
stgi
vmload
vmload [rax]
vmload [eax]
vmmcall
vmrun
vmrun [rax]
vmrun [eax]
vmsave
vmsave [rax]
vmsave [eax]

[bits 32]
invlpga
invlpga [eax], ecx
invlpga [ax], ecx
skinit
skinit [eax]
; skinit [ax] ; invalid
vmload
vmload [eax]
vmload [ax]
vmrun
vmrun [eax]
vmrun [ax]
vmsave
vmsave [eax]
vmsave [ax]

