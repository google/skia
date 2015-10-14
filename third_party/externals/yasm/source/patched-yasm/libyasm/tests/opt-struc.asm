[absolute 0]
HOSTENT:
.Name		resd	1
.Aliases	resd	1

.AddrList	resd	1

HOSTENT_size:

[section .bss]
STRING_MAX		equ	256
HOSTENT_ALIASES_MAX	equ	16
HOSTENT_ADDRLIST_MAX	equ	16

HostEnt_Name_static	resb	STRING_MAX
HostEnt_Aliases_static	resd	HOSTENT_ALIASES_MAX
HostEnt_AddrList_static	resd	HOSTENT_ADDRLIST_MAX
HostEnt_Aliases_data	resb	STRING_MAX*HOSTENT_ALIASES_MAX
HostEnt_AddrList_data	resd	HOSTENT_ADDRLIST_MAX

[section .data]
HostEnt_static	:
..@44.strucstart:
times HOSTENT.Name-($-..@44.strucstart) db 0
dd	HostEnt_Name_static
times HOSTENT.Aliases-($-..@44.strucstart) db 0
dd	HostEnt_Aliases_static
times HOSTENT.AddrList-($-..@44.strucstart) db 0
dd	HostEnt_AddrList_static
times HOSTENT_size-($-..@44.strucstart) db 0

HostEnt_static2	:
..@45.strucstart:
times HOSTENT.Name-($-..@45.strucstart) db 0
dd	HostEnt_Name_static
times HOSTENT.Aliases-($-..@45.strucstart) db 0
dd	HostEnt_Aliases_static
times HOSTENT_size-($-..@45.strucstart) db 0

