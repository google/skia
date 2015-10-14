%line 1+1 elfabssect.asm
[absolute 0]
%line 1+0 elfabssect.asm
teststruc:
%line 2+1 elfabssect.asm
 .testlabel resw 1
teststruc_size:
%line 3+0 elfabssect.asm
[section .text]
;global teststruc
global teststruc.testlabel
global teststruc_size
