[map all]
; Memory below 0800h is reserved for the BIOS and the MBR
BSS_START       equ 0800h

; PXELINUX needs lots of BSS, so it relocates itself on startup
;%if IS_PXELINUX
TEXT_START      equ 9000h
;%else
;TEXT_START      equ 7C00h
;%endif

;
; The various sections and their relationship
;
org TEXT_START

times 0x100 db 0x3
 
section .earlybss nobits start=BSS_START
resb 0x100
section .bcopy32  align=16 follows=.data vfollows=.earlybss
times 0x100 db 0x1
section .bss      nobits align=256 vfollows=.bcopy32
resb 0x100
 
section .text     start=TEXT_START
section .data     align=16 follows=.text
times 0x100 db 0x2

section .latebss  nobits align=16 follows=.bcopy32
resb 0x100
