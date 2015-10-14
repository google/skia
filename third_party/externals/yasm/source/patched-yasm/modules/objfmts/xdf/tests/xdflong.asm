;; ######################################################################## 
;; Macros
;; ########################################################################

; descriptor type, base, limit, p_dpl_s, g_db_a
%macro descriptor 5 
	dw %3		       			; Limit 15-0
	dw %2            			; Base  15-0
	db %2 >> 16				; Base  23-16
	db ((%4 & 0xF) << 4) | (%1 & 0xF )	; p_dpl_s_type  
	db (%5 << 4) | ((%3 & 0xF0000) >> 16) 	; g_db_a limit 19:16
	db %2 >> 24				; Base  31-24	
%endmacro

; cdesc64 base, limit, dpl
%macro cdesc64 3
       descriptor 0xB, %1, %2, 0x9 | (%3 & 0x3) << 1, 0xD
%endmacro
	
; gates type, offset, selector, p_dpl_s
%macro gates 4 
	dw %2 		      			; Offset 15-00
	dw %3					; Selector 
	db 0					; Ist
	db ((%4 & 0xF) << 4) | (%1 & 0xF) 	; p_dpl_s_type  
	dw %2 >> 16				; Offset 31-16
	dd %2 >> 32				; Offset 63-32 
%endmacro

; idesc64 offset, selector, ring
%macro idesc64 3
       gates 0xE, %1, %2, 0x8 | ((%3 & 0x3) << 1)
%endmacro

	
; pageDirectory2M addr, nx, a, pcd, pwt, u, w, p
%macro pageDirectory2M 8
	db %2 << 7
	dw %1 >> 40
	dd %1 >> 8
	db (%3 << 5) | (%4 << 4) | (%5 << 3) | (%6 << 2) | (%7 << 1) | %8
%endmacro
	
; pageEntry2M addr, nx, pat, g, d, a, pcd, pwt, u, w, p 
%macro pageEntry2M 11
	db %2 << 7
	db %1 >> 48
	dd %1 >> 16
	dw (%3 << 12) | (%4 << 8) | (%5 << 6) | (%6 << 5) | (%7 << 4) | (%8 << 3) | (%9 << 2) | (%10 << 1) | %11 | 0x80
%endmacro
			
		
;; ######################################################################## 
;; Code Section
;; ########################################################################

SECTION CODE ABSOLUTE=0xFFFFFFFF00000000 FLAT USE64

test_code:	

	;; Your Code Goes Here
	add r8, r15
	
	hlt			 
		
;; ######################################################################## 
;; Setup Section
;; ########################################################################

SECTION SETUP ALIGN=16 FLAT USE16

setup:	

	xor edx, edx		; Enable Var MTRRs
	mov eax, 0x0806		; WriteBack
	mov ecx, 0x2FF		
	wrmsr

	mov ebx, cr0
        or  ebx, 0x00000021	; Protect Mode On, Int 16 for FPU
	and ebx, 0x9FFFFFFF     ; Turn Caches on
	mov cr0, ebx

	mov edx, cr4		
	or  edx, 0x00000620     ; Enable PAE, SSE OSFXSR, SEE OSXMMEXCPT
	mov cr4, edx

	mov edx, pageMapL4	; load pagetables
	mov cr3, edx

	mov ecx, 0x80000080
	rdmsr			; Read EFER
	bts eax, 8		; Enable Long Mode (LME=1)
	wrmsr			; Write EFER
	
	bts ebx, 31		; Enable Paging (PG=1)
	mov cr0, ebx		
	
	;; At this point LME=1, PAE=1, PG=1, CS.L=0, CS.D=0	
		
	lgdt [pgdt]		; Set GDT
	lidt [pidt]		; Set IDT
	
	jmp 0x8 : long_mode
	
long_mode:		

        BITS 64

	mov rax, qword test_code	; jmp to testcode
	jmp [rax]
	
	
			
;; ######################################################################## 
;; Long Mode IDT 
;; ########################################################################
	 
SECTION IDTP ALIGN=16 FLAT USE64

	;; cdesc32 base, limit, dpl
gdt0:	dq 0				; 0x0000 - Null descriptor
     	cdesc64 zero, 0xFFFFF, 0	; 0x0008 - Code Selector 
gdt_:	
		
	;; idesc64 offset, selector, dpl
idt0:	idesc64 isrL, 0x0008, 0		; 0x00,  0   #DE, Divide Error	
     	idesc64 isrL, 0x0008, 0		; 0x01,  1   #DB, Debug Fault
     	idesc64 isrL, 0x0008, 0		; 0x02,  2,  ---, NMI
     	idesc64 isrL, 0x0008, 0		; 0x03,  3,  #BP, Breakpoint
     	idesc64 isrL, 0x0008, 0		; 0x04,  4,  #OF, INTO detected Overflow
     	idesc64 isrL, 0x0008, 0	        ; 0x05,  5,  #BR, Bound Range Exceeded
     	idesc64 isrL, 0x0008, 0		; 0x06,  6,  #UD, Invalid Opcode
     	idesc64 isrL, 0x0008, 0		; 0x07,  7,  #NM, Device Not Available
     	idesc64 isrL, 0x0008, 0		; 0x08,  8,  #DF, Double Fault
     	idesc64 isrL, 0x0008, 0		; 0x09   9,  ---, Coprocessor Segment Overrun
      	idesc64 isrL, 0x0008, 0		; 0x0A,  10, #TS, Invalid TSS 
      	idesc64 isrL, 0x0008, 0		; 0x0B,  11, #NP, Segment Not Present
      	idesc64 isrL, 0x0008, 0		; 0x0C,  12, #SS, Stack Fault
      	idesc64 isrL, 0x0008, 0	        ; 0x0D,  13, #GP, General Protection Fault
      	idesc64 isrL, 0x0008, 0		; 0x0E,  14, #PF, Page Fault
      	idesc64 isrL, 0x0008, 0		; 0x0F,  15, ---, Reserved
      	idesc64 isrL, 0x0008, 0		; 0x10,  16, #MF, Floating Point Fault
      	idesc64 isrL, 0x0008, 0		; 0x11,  17, #AC, Alignment Check
      	idesc64 isrL, 0x0008, 0		; 0x12   18, #MC, Machine Check
      	idesc64 isrL, 0x0008, 0		; 0x13,  19, #XF, SSE Fault
idt_:	
	
pgdt:   dw (gdt_ - gdt0)		; Limit 
	dd gdt0			        ; base
	
pidt:   dw (idt_ - idt0)		; Limit 
	dd idt0			        ; base

isrL:	mov eax, 0xDEADBEEF             ; Default Interrupt Handler 
	out 0x80, eax
	hlt
			
;; ######################################################################## 
;; Real Mode IDT 
;; ########################################################################
	
SECTION IDTR ABSOLUTE=0x00000000 FLAT USE16 

	;; FORMAT IP:CS 
zero:	dw isrR, 0			; 0x00,  0   #DE, Divide Error	
	dw isrR, 0			; 0x01,  1   #DB, Debug Fault
	dw isrR, 0			; 0x02,  2,  ---, NMI
	dw isrR, 0			; 0x03,  3,  #BP, Breakpoint
	dw isrR, 0			; 0x04,  4,  #OF, INTO detected Overflow
	dw isrR, 0			; 0x05,  5,  #BR, Bound Range Exceeded
	dw isrR, 0			; 0x06,  6,  #UD, Invalid Opcode
	dw isrR, 0			; 0x07,  7,  #NM, Device Not Available
	dw isrR, 0			; 0x08,  8,  #DF, Double Fault
	dw isrR, 0			; 0x09   9,  ---, Coprocessor Segment Overrun
	dw isrR, 0			; 0x0A,  10, #TS, Invalid TSS 
	dw isrR, 0			; 0x0B,  11, #NP, Segment Not Present
	dw isrR, 0			; 0x0C,  12, #SS, Stack Fault
	dw isrR, 0			; 0x0D,  13, #GP, General Protection Fault
	dw isrR, 0			; 0x0E,  14, #PF, Page Fault
	dw isrR, 0			; 0x0F,  15, ---, Reserved
	dw isrR, 0			; 0x10,  16, #MF, Floating Point Fault
	dw isrR, 0			; 0x11,  17, #AC, Alignment Check
	dw isrR, 0			; 0x12   18, #MC, Machine Check
	dw isrR, 0			; 0x13,  19, #XF, SSE Fault

isrR:	mov eax, 0xDEADBEEF             ; Default Real Interrupt Handler 
	out 0x80, eax
	hlt

;; ######################################################################## 
;; 2 Meg Page Tables                                                         
;; ########################################################################

SECTION PAGE ALIGN=4096 FLAT

pageDirE:
%assign addr 0
%rep 512			         
	; pageEntry addr, nx, pat, g, d, a, pcd, pwt, u, w, p 
	pageEntry2M addr,  0,   0, 0, 1, 1,   0,   0, 1, 1, 1  ; Accessed, WB, User, Writable, Present
%assign addr addr + 0x200000
%endrep
		
pageDirP:
%rep 512			         
	; pageDirPointer     addr, nx, a, pcd, pwt, u, w, p
	pageDirectory2M  pageDirE,  0, 1,   0,   0, 1, 1, 1    ; Accessed, WB, User, Writable, Present
%endrep
		
pageMapL4:	
%rep 512			         
	; pageDirectory      addr, nx, a, pcd, pwt, u, w, p
	pageDirectory2M  pageDirP,  0, 1,   0,   0, 1, 1, 1    ; Accessed, WB, User, Writable, Present
%endrep
			
;; ######################################################################## 
;; SMM Handler
;; ########################################################################

SECTION SMM ABSOLUTE=0x00038000 USE16
	
	rsm
	
;; ######################################################################## 
;; Reset Vector
;; ########################################################################

SECTION RESET ABSOLUTE=0xFFFFFFF0 USE16
	
	jmp far setup
