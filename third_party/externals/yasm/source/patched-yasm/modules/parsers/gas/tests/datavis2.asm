	.section .eh_frame, "",@progbits
.EHCIE:
	.4byte 0x10
	.4byte 0x0
	.2byte 1
	.8byte 4
	.byte	0x01, 0x00, 0x01, 0x78, 0x10, 0x00, 0x0c, 0x07
	.byte	0x08, 0x90, 0x01, 0x00

	.section .debug_line, ""
	.section	.note.GNU-stack,"",@progbits
	.ident	"# : compiled with : "
	.ident	"second ident"
