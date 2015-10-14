;; ######################################################################## 
;; Macros
;; ########################################################################

; descriptor type, base, limit, p_dpl_s, g_db_a
%macro descriptor 5 
	dw (%3 & 0xFFFF)       			; Limit 0-15
	dw (%2 & 0xFFFF) 			; Base  0-15
	db ((%2 & 0xFF0000) >> 16)		; Base  16-23
	db ((%4 & 0xF) << 4) | (%1 & 0xF )	; p_dpl_s_type  
	db (%5 << 4) | ((%3 & 0xF0000) >> 16) 	; g_db_a limit 19:16
	db ((%2 & 0xFF000000) >> 24)    	; Base  24-31	
%endmacro

; cdesc32 base, limit, dpl
%macro cdesc32 3
       descriptor 0xB, %1, %2, 0x9 | (%3 & 0x3) << 1, 0xD
%endmacro
	
; ddesc32 base, limit, dpl
%macro ddesc32 3
       descriptor 0x3, %1, %2, 0x9 | (%3 & 0x3) << 1, 0xD
%endmacro

; gates type, offset, selector, p_dpl_s
%macro gates 4 
	dw %2 		      			; Offset 0-15
	dw (%3 & 0xFFFF) 			; Selector 0-15
	db 0					; Reserved
	db ((%4 & 0xF) << 4) | (%1 & 0xF) 	; p_dpl_s_type  
	dw 0					; Offset 16-31
%endmacro
	
%macro idesc32 3
       gates 0xE, %1, %2, 0x8 | ((%3 & 0x3) << 1)
%endmacro
	
	
;; ######################################################################## 
;; Code Section
;; ########################################################################

SECTION CODE ABSOLUTE=0x00400000 FLAT USE32

test_code:	

	;; Your Code Goes Here
	hlt			 
		
;; ######################################################################## 
;; Setup Section
;; ########################################################################

SECTION SETUP ALIGN=16 FLAT USE16

setup:	

	mov edx, cr0
        or  dl,  0x21		; Protect Mode On, Int 16 for FPU
	and edx, 0x9FFFFFFF     ; Turn Caches on
	mov cr0, edx

	xor edx, edx		; Enable Var MTRRs
	mov eax, 0x0806		; WriteBack
	mov ecx, 0x2FF		
	wrmsr

	lgdt [pgdt]		; Set GDT
	lidt [pidt]		; Set IDT
	
	jmp 0x8:protect_mode
	
protect_mode:		

        BITS 32

	mov esp, 0x01000000	; Get some stack space
	mov ax,  0x0010		; Set data selectors
	mov ss, ax
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	jmp test_code		; Jmp to test code
		
;; ######################################################################## 
;; Protect Mode IDT 
;; ########################################################################
	 
SECTION IDTP ALIGN=16 FLAT USE32

	;; cdesc32 base, limit, dpl
gdt0:	dq 0.0				; 0x0000 - Null descriptor
     	cdesc32 0x00000000, 0xFFFFF, 0	; 0x0008 - Code Selector 
     	ddesc32 0x00000000, 0xFFFFF, 0	; 0x0010 - Data Selector
     	cdesc32 0x00000000, 0xFFFFF, 3  ; 0x0018 - Code Select Ring 3 
     	ddesc32 0x00000000, 0xFFFFF, 3	; 0x0020 - Data Select Ring 3
	
	;; idesc32 offset, selector, dpl
idt0:	idesc32 isrP, 0x0008, 0		; 0x00,  0   #DE, Divide Error	
     	idesc32 isrP, 0x0008, 0		; 0x01,  1   #DB, Debug Fault
     	idesc32 isrP, 0x0008, 0		; 0x02,  2,  ---, NMI
     	idesc32 isrP, 0x0008, 0		; 0x03,  3,  #BP, Breakpoint
     	idesc32 isrP, 0x0008, 0		; 0x04,  4,  #OF, INTO detected Overflow
     	idesc32 isrP, 0x0008, 0	        ; 0x05,  5,  #BR, Bound Range Exceeded
     	idesc32 isrP, 0x0008, 0		; 0x06,  6,  #UD, Invalid Opcode
     	idesc32 isrP, 0x0008, 0		; 0x07,  7,  #NM, Device Not Available
     	idesc32 isrP, 0x0008, 0		; 0x08,  8,  #DF, Double Fault
     	idesc32 isrP, 0x0008, 0		; 0x09   9,  ---, Coprocessor Segment Overrun
      	idesc32 isrP, 0x0008, 0		; 0x0A,  10, #TS, Invalid TSS 
      	idesc32 isrP, 0x0008, 0		; 0x0B,  11, #NP, Segment Not Present
      	idesc32 isrP, 0x0008, 0		; 0x0C,  12, #SS, Stack Fault
      	idesc32 isrP, 0x0008, 0	        ; 0x0D,  13, #GP, General Protection Fault
      	idesc32 isrP, 0x0008, 0		; 0x0E,  14, #PF, Page Fault
      	idesc32 isrP, 0x0008, 0		; 0x0F,  15, ---, Reserved
      	idesc32 isrP, 0x0008, 0		; 0x10,  16, #MF, Floating Point Fault
      	idesc32 isrP, 0x0008, 0		; 0x11,  17, #AC, Alignment Check
      	idesc32 isrP, 0x0008, 0		; 0x12   18, #MC, Machine Check
      	idesc32 isrP, 0x0008, 0		; 0x13,  19, #XF, SSE Fault

pgdt:   dw 6 * 8			; Limit 
	dd gdt0			        ; base
	
pidt:   dw 20 * 8			; Limit 
	dd idt0			        ; base

isrP:	mov eax, 0xDEADBEEF ; Default Real Mode Interrupt Handler 
	out 0x80, eax
	hlt
			
;; ######################################################################## 
;; Real Mode IDT 
;; ########################################################################
	
SECTION IDTR ABSOLUTE=0x00000000 FLAT USE16 

	;; FORMAT IP:CS 
	dw isrR, 0			; 0x00,  0   #DE, Divide Error	
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

isrR:	mov eax, 0xDEADBEEF ; Default Real Mode Interrupt Handler 
	out 0x80, eax
	hlt
	
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
