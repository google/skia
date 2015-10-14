;uInt longest_match_x64(
;    deflate_state *s,
;    IPos cur_match);                             /* current match */

; gvmat64.asm -- Asm portion of the optimized longest_match for 32 bits x86
; Copyright (C) 1995-2005 Jean-loup Gailly, Brian Raiter and Gilles Vollant.
;
; File written by Gilles Vollant, by converting to assembly the longest_match
;  from Jean-loup Gailly in deflate.c of zLib and infoZip zip.
;
;  and by taking inspiration on asm686 with masm, optimised assembly code
;        from Brian Raiter, written 1998
;
;         http://www.zlib.net
;         http://www.winimage.com/zLibDll
;         http://www.muppetlabs.com/~breadbox/software/assembly.html
;
; to compile this file for infozip Zip, I use option:
;   ml64.exe /Flgvmat64 /c /Zi /DINFOZIP gvmat64.asm
;
; to compile this file for zLib, I use option:
;   ml64.exe /Flgvmat64 /c /Zi gvmat64.asm
; Be carrefull to adapt zlib1222add below to your version of zLib
;   (if you use a version of zLib before 1.0.4 or after 1.2.2.2, change
;    value of zlib1222add later)
;
; This file compile with Microsoft Macro Assembler (x64) for AMD64
;
;   ml64.exe is given with Visual Studio 2005 and Windows 2003 server DDK
;
;   (you can get Windows 2003 server DDK with ml64 and cl for AMD64 from
;      http://www.microsoft.com/whdc/devtools/ddk/default.mspx for low price)
;


;uInt longest_match(s, cur_match)
;    deflate_state *s;
;    IPos cur_match;                             /* current match */
bits 64
	section .text
	
longest_match:	; PROC


;%define LocalVarsSize  88
 %define LocalVarsSize  72

; register used : rax,rbx,rcx,rdx,rsi,rdi,r8,r9,r10,r11,r12
; free register :  r14,r15
; register can be saved : rsp

 %define chainlenwmask   rsp + 8 - LocalVarsSize    ; high word: current chain len
                                                 ; low word: s->wmask
;%define window   rsp + xx - LocalVarsSize   ; local copy of s->window ; stored in r10
;%define windowbestlen   rsp + xx - LocalVarsSize   ; s->window + bestlen , use r10+r11
;%define scanstart   rsp + xx - LocalVarsSize   ; first two bytes of string ; stored in r12w
;%define scanend   rsp + xx - LocalVarsSize   ; last two bytes of string use ebx
;%define scanalign   rsp + xx - LocalVarsSize   ; dword-misalignment of string r13
;%define bestlen   rsp + xx - LocalVarsSize   ; size of best match so far -> r11d
;%define scan   rsp + xx - LocalVarsSize   ; ptr to string wanting match -> r9
%ifdef INFOZIP
%else
 %define nicematch   (rsp + 16 - LocalVarsSize) ; a good enough match size
%endif

%define save_rdi   rsp + 24 - LocalVarsSize
%define save_rsi   rsp + 32 - LocalVarsSize
%define save_rbx   rsp + 40 - LocalVarsSize
%define save_rbp   rsp + 48 - LocalVarsSize
%define save_r12   rsp + 56 - LocalVarsSize
%define save_r13   rsp + 64 - LocalVarsSize
;%define save_r14   rsp + 72 - LocalVarsSize
;%define save_r15   rsp + 80 - LocalVarsSize



;  all the +4 offsets are due to the addition of pending_buf_size (in zlib
;  in the deflate_state structure since the asm code was first written
;  (if you compile with zlib 1.0.4 or older, remove the +4).
;  Note : these value are good with a 8 bytes boundary pack structure


    %define MAX_MATCH      258
    %define MIN_MATCH      3
    %define MIN_LOOKAHEAD      (MAX_MATCH+MIN_MATCH+1)


;;; Offsets for fields in the deflate_state structure. These numbers
;;; are calculated from the definition of deflate_state, with the
;;; assumption that the compiler will dword-align the fields. (Thus,
;;; changing the definition of deflate_state could easily cause this
;;; program to crash horribly, without so much as a warning at
;;; compile time. Sigh.)

;  all the +zlib1222add offsets are due to the addition of fields
;  in zlib in the deflate_state structure since the asm code was first written
;  (if you compile with zlib 1.0.4 or older, use "%define zlib1222add  (-4)").
;  (if you compile with zlib between 1.0.5 and 1.2.2.1, use "%define zlib1222add  0").
;  if you compile with zlib 1.2.2.2 or later , use "%define zlib1222add  8").

%ifdef INFOZIP
	section .data

extern    window_size:DWORD
; WMask ; 7fff
extern    window:BYTE:010040H
extern    prev:WORD:08000H
; MatchLen : unused
; PrevMatch : unused
extern    strstart:DWORD
extern    match_start:DWORD
; Lookahead : ignore
extern    prev_length:DWORD ; PrevLen
extern    max_chain_length:DWORD
extern    good_match:DWORD
extern    nice_match:DWORD
%define prev_ad  OFFSET prev
%define window_ad  OFFSET window
%define nicematch  nice_match

%define WMask  07fffh

%else

  %ifndef zlib1222add
    %define zlib1222add  8
  %endif
%define dsWSize  56+zlib1222add+(zlib1222add/2)
%define dsWMask  64+zlib1222add+(zlib1222add/2)
%define dsWindow  72+zlib1222add
%define dsPrev  88+zlib1222add
%define dsMatchLen  128+zlib1222add
%define dsPrevMatch  132+zlib1222add
%define dsStrStart  140+zlib1222add
%define dsMatchStart  144+zlib1222add
%define dsLookahead  148+zlib1222add
%define dsPrevLen  152+zlib1222add
%define dsMaxChainLen  156+zlib1222add
%define dsGoodMatch  172+zlib1222add
%define dsNiceMatch  176+zlib1222add

%define window_size  [ rcx + dsWSize]
%define WMask  [ rcx + dsWMask]
%define window_ad  [ rcx + dsWindow]
%define prev_ad  [ rcx + dsPrev]
%define strstart  [ rcx + dsStrStart]
%define match_start  [ rcx + dsMatchStart]
%define Lookahead  [ rcx + dsLookahead] ; 0ffffffffh on infozip
%define prev_length  [ rcx + dsPrevLen]
%define max_chain_length  [ rcx + dsMaxChainLen]
%define good_match  [ rcx + dsGoodMatch]
%define nice_match  [ rcx + dsNiceMatch]
%endif

; parameter 1 in r8(deflate state s), param 2 in rdx (cur match)

; see http://weblogs.asp.net/oldnewthing/archive/2004/01/14/58579.aspx and
; http://msdn.microsoft.com/library/en-us/kmarch/hh/kmarch/64bitAMD_8e951dd2-ee77-4728-8702-55ce4b5dd24a.xml.asp
;
; All registers must be preserved across the call, except for
;   rax, rcx, rdx, r8, r9, r10, and r11, which are scratch.

	section .text

;;; Save registers that the compiler may be using, and adjust esp to
;;; make room for our stack frame.


;;; Retrieve the function arguments. r8d will hold cur_match
;;; throughout the entire function. edx will hold the pointer to the
;;; deflate_state structure during the function's setup (before
;;; entering the main loop.

; parameter 1 in rcx (deflate_state* s), param 2 in edx -> r8 (cur match)

; this clear high 32 bits of r8, which can be garbage in both r8 and rdx

        mov [save_rdi],rdi
        mov [save_rsi],rsi
        mov [save_rbx],rbx
        mov [save_rbp],rbp
%ifdef INFOZIP
        mov r8d,ecx
%else
        mov r8d,edx
%endif
        mov [save_r12],r12
        mov [save_r13],r13
;        mov [save_r14],r14
;        mov [save_r15],r15


;;; uInt wmask = s->w_mask;
;;; unsigned chain_length = s->max_chain_length;
;;; if (s->prev_length >= s->good_match) {
;;;     chain_length >>= 2;
;;; }

        mov edi, prev_length
        mov esi, good_match
        mov eax, WMask
        mov ebx, max_chain_length
        cmp edi, esi
        jl  LastMatchGood
        shr ebx, 2
LastMatchGood:

;;; chainlen is decremented once beforehand so that the function can
;;; use the sign flag instead of the zero flag for the exit test.
;;; It is then shifted into the high word, to make room for the wmask
;;; value, which it will always accompany.

        dec ebx
        shl ebx, 16
        or  ebx, eax

;;; on zlib only
;;; if ((uInt)nice_match > s->lookahead) nice_match = s->lookahead;

%ifdef INFOZIP
        mov [chainlenwmask], ebx
; on infozip nice_match = [nice_match]
%else
        mov eax, nice_match
        mov [chainlenwmask], ebx
        mov r10d, Lookahead
        cmp r10d, eax
        cmovnl r10d, eax
        mov [nicematch],r10d
%endif

;;; register Bytef *scan = s->window + s->strstart;
        mov r10, window_ad
        mov ebp, strstart
        lea r13, [r10 + rbp]

;;; Determine how many bytes the scan ptr is off from being
;;; dword-aligned.

         mov r9,r13
         neg r13
         and r13,3

;;; IPos limit = s->strstart > (IPos)MAX_DIST(s) ?
;;;     s->strstart - (IPos)MAX_DIST(s) : NIL;
%ifdef INFOZIP
        mov eax,07efah ; MAX_DIST = (WSIZE-MIN_LOOKAHEAD) (0x8000-(3+8+1))
%else
        mov eax, window_size
        sub eax, MIN_LOOKAHEAD
%endif
        xor edi,edi
        sub ebp, eax

        mov r11d, prev_length

        cmovng ebp,edi

;;; int best_len = s->prev_length;


;;; Store the sum of s->window + best_len in esi locally, and in esi.

       lea  rsi,[r10+r11]

;;; register ush scan_start = *(ushf*)scan;
;;; register ush scan_end   = *(ushf*)(scan+best_len-1);
;;; Posf *prev = s->prev;

        movzx r12d,word [r9]
        movzx ebx, word [r9 + r11 - 1]

        mov rdi, prev_ad

;;; Jump into the main loop.

        mov edx, [chainlenwmask]

        cmp bx,word [rsi + r8 - 1]
        jz  LookupLoopIsZero

LookupLoop1:
        and r8d, edx

        movzx   r8d, word [rdi + r8*2]
        cmp r8d, ebp
        jbe LeaveNow
        sub edx, 00010000h
        js  LeaveNow

LoopEntry1:
        cmp bx,word [rsi + r8 - 1]
        jz  LookupLoopIsZero

LookupLoop2:
        and r8d, edx

        movzx   r8d, word [rdi + r8*2]
        cmp r8d, ebp
        jbe LeaveNow
        sub edx, 00010000h
        js  LeaveNow

LoopEntry2:
        cmp bx,word [rsi + r8 - 1]
        jz  LookupLoopIsZero

LookupLoop4:
        and r8d, edx

        movzx   r8d, word [rdi + r8*2]
        cmp r8d, ebp
        jbe LeaveNow
        sub edx, 00010000h
        js  LeaveNow

LoopEntry4:

        cmp bx,word [rsi + r8 - 1]
        jnz LookupLoop1
        jmp LookupLoopIsZero


;;; do {
;;;     match = s->window + cur_match;
;;;     if (*(ushf*)(match+best_len-1) != scan_end ||
;;;         *(ushf*)match != scan_start) continue;
;;;     [...]
;;; } while ((cur_match = prev[cur_match & wmask]) > limit
;;;          && --chain_length != 0);
;;;
;;; Here is the inner loop of the function. The function will spend the
;;; majority of its time in this loop, and majority of that time will
;;; be spent in the first ten instructions.
;;;
;;; Within this loop:
;;; ebx = scanend
;;; r8d = curmatch
;;; edx = chainlenwmask - i.e., ((chainlen << 16) | wmask)
;;; esi = windowbestlen - i.e., (window + bestlen)
;;; edi = prev
;;; ebp = limit

LookupLoop:
        and r8d, edx

        movzx   r8d, word [rdi + r8*2]
        cmp r8d, ebp
        jbe LeaveNow
        sub edx, 00010000h
        js  LeaveNow

LoopEntry:

        cmp bx,word [rsi + r8 - 1]
        jnz LookupLoop1
LookupLoopIsZero:
        cmp     r12w, word [r10 + r8]
        jnz LookupLoop1


;;; Store the current value of chainlen.
        mov [chainlenwmask], edx

;;; Point edi to the string under scrutiny, and esi to the string we
;;; are hoping to match it up with. In actuality, esi and edi are
;;; both pointed (MAX_MATCH_8 - scanalign) bytes ahead, and edx is
;;; initialized to -(MAX_MATCH_8 - scanalign).

        lea rsi,[r8+r10]
        mov rdx, 0fffffffffffffef8h; -(MAX_MATCH_8)
        lea rsi, [rsi + r13 + 0108h] ;MAX_MATCH_8]
        lea rdi, [r9 + r13 + 0108h] ;MAX_MATCH_8]

        prefetcht1 [rsi+rdx]
        prefetcht1 [rdi+rdx]


;;; Test the strings %define for ality, 8 bytes at a time. At the end,
;;; adjust rdx so that it is offset to the exact byte that mismatched.
;;;
;;; We already know at this point that the first three bytes of the
;;; strings match each other, and they can be safely passed over before
;;; starting the compare loop. So what this code does is skip over 0-3
;;; bytes, as much as necessary in order to dword-align the edi
;;; pointer. (rsi will still be misaligned three times out of four.)
;;;
;;; It should be confessed that this loop usually does not represent
;;; much of the total running time. Replacing it with a more
;;; straightforward "rep cmpsb" would not drastically degrade
;;; performance.

LoopCmps:
        mov rax, [rsi + rdx]
        xor rax, [rdi + rdx]
        jnz LeaveLoopCmps

        mov rax, [rsi + rdx + 8]
        xor rax, [rdi + rdx + 8]
        jnz LeaveLoopCmps8


        mov rax, [rsi + rdx + 8+8]
        xor rax, [rdi + rdx + 8+8]
        jnz LeaveLoopCmps16

        add rdx,8+8+8

        jmp short LoopCmps
LeaveLoopCmps16: add rdx,8
LeaveLoopCmps8: add rdx,8
LeaveLoopCmps:

        test    eax, 0000FFFFh
        jnz LenLower

        test eax,0ffffffffh

        jnz LenLower32

        add rdx,4
        shr rax,32
        or ax,ax
        jnz LenLower

LenLower32:
        shr eax,16
        add rdx,2
LenLower:   sub al, 1
        adc rdx, 0
;;; Calculate the length of the match. If it is longer than MAX_MATCH,
;;; then automatically accept it as the best possible match and leave.

        lea rax, [rdi + rdx]
        sub rax, r9
        cmp eax, MAX_MATCH
        jge LenMaximum

;;; If the length of the match is not longer than the best match we
;;; have so far, then forget it and return to the lookup loop.
;///////////////////////////////////

        cmp eax, r11d
        jg  LongerMatch

        lea rsi,[r10+r11]

        mov rdi, prev_ad
        mov edx, [chainlenwmask]
        jmp LookupLoop

;;;         s->match_start = cur_match;
;;;         best_len = len;
;;;         if (len >= nice_match) break;
;;;         scan_end = *(ushf*)(scan+best_len-1);

LongerMatch:
        mov r11d, eax
        mov match_start, r8d
        cmp eax, [nicematch]
        jge LeaveNow

        lea rsi,[r10+rax]

        movzx   ebx, word [r9 + rax - 1]
        mov rdi, prev_ad
        mov edx, [chainlenwmask]
        jmp LookupLoop

;;; Accept the current string, with the maximum possible length.

LenMaximum:
        mov r11d,MAX_MATCH
        mov match_start, r8d
;;; if ((uInt)best_len <= s->lookahead) return (uInt)best_len;
;;; return s->lookahead;

LeaveNow:
%ifdef INFOZIP
        mov eax,r11d
%else
        mov eax, Lookahead
        cmp r11d, eax
        cmovng eax, r11d
%endif

;;; Restore the stack and return from whence we came.

        mov rsi,[save_rsi]
        mov rdi,[save_rdi]
        mov rbx,[save_rbx]
        mov rbp,[save_rbp]
        mov r12,[save_r12]
        mov r13,[save_r13]
;        mov r14,[save_r14]
;        mov r15,[save_r15]


        ret 0
; please don't remove this string !
; Your can freely use gvmat64 in any free or externercial app
; but it is far better don't remove the string in the binary!
    db     0dh,0ah,"asm686 with masm, optimised assembly code from Brian Raiter, written 1998, converted to amd 64 by Gilles Vollant 2005",0dh,0ah,0
%if 0
longest_match   ENDP
%endif

match_init:	; PROC
  ret 0
 %if 0
match_init ENDP
%endif

END
