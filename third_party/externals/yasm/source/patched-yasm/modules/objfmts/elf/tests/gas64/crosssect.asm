.section .rodata
	.align 4
_sys_srt:
	.long 0

.text
	.align 4,0x90
	.long _sys_srt-.
	.long _sys_srt-_sys_info
	#.long _sys_info-_sys_srt
	#.long -(_sys_srt-_sys_info)
	.long _sys_info-.
	.long .-_sys_info
	.long (_sys_srt-.)+(.-_sys_info) # GAS cannot handle this but we can
	.long 0
	.long 65558
_sys_info:
	.long 0
