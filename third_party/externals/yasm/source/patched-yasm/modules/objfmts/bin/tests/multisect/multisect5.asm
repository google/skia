org 0x100
[map all]
section sect1 start=0x100 vstart=0x2000
times 0x100 db 0
section sect2 follows=sect1
times 0x100 db 0

section sect3 start=0x300 vstart=0x4000
times 0x100 db 0
section sect4 follows=sect3
times 0x100 db 0
section sect5 vfollows=sect3
times 0x100 db 0

section sect6 start=0x600 vstart=0x6000
times 0x11 db 0
section sect7 follows=sect6 valign=16
times 0x104 db 0
section sect8 follows=sect7 valign=16	; NASM bug - sect7 and sect8 overlap
times 0x100 db 0

section sect9 vfollows=sect8
