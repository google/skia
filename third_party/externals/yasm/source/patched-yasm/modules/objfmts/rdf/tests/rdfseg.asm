	;; program to test inter-segment production and linkage of RDF objects

	;; [1] should produce segment base ref
	;; [2] should produce standard relocation
	
[GLOBAL _main]
[EXTERN _puts:	far]
[BITS 16]
	
_main:
	mov ax, seg _message	; 0000 [1]
	mov ds, ax		; 0003
	mov dx, _message	; 0005 [2]
	call far _puts		; 0008 [2][1]
	xor ax,ax		; 000D
	int 21h			; 000F
	
[SECTION .data]
_message:	db 'Hello, World', 10, 13, 0
	