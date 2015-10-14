%MACRO TEST_GENERIC 5
;global _test_ %+ %1 %+ _ %+ %4
;global test_ %+ %1 %+ _ %+ %4
_test_ %+ %1 %+ _ %+ %4:
test_ %+ %1 %+ _ %+ %4:
   mov         edx, [ esp + 4 ]
   mov         eax, [ esp + 8 ]
   %2          %3, [ edx ]
   %2          %5, [ eax ]
   %1          %3, %5
   %2          [ edx ], %3
   ret
%ENDMACRO

TEST_GENERIC pabsb, movq, mm0, mmx, mm1
TEST_GENERIC pabsw, movq, mm0, mmx, mm1
TEST_GENERIC pabsd, movq, mm0, mmx, mm1

TEST_GENERIC pabsb, movdqu, xmm0, xmm, xmm1
TEST_GENERIC pabsw, movdqu, xmm0, xmm, xmm1
TEST_GENERIC pabsd, movdqu, xmm0, xmm, xmm1

TEST_GENERIC psignb, movq, mm0, mmx, mm1
TEST_GENERIC psignw, movq, mm0, mmx, mm1
TEST_GENERIC psignd, movq, mm0, mmx, mm1
          
TEST_GENERIC psignb, movdqu, xmm0, xmm, xmm1
TEST_GENERIC psignw, movdqu, xmm0, xmm, xmm1
TEST_GENERIC psignd, movdqu, xmm0, xmm, xmm1

TEST_GENERIC phaddw, movq, mm0, mmx, mm1
TEST_GENERIC phaddsw, movq, mm0, mmx, mm1
TEST_GENERIC phaddd, movq, mm0, mmx, mm1

TEST_GENERIC phaddw, movdqu, xmm0, xmm, xmm1
TEST_GENERIC phaddsw, movdqu, xmm0, xmm, xmm1
TEST_GENERIC phaddd, movdqu, xmm0, xmm, xmm1

TEST_GENERIC phsubw, movq, mm0, mmx, mm1
TEST_GENERIC phsubsw, movq, mm0, mmx, mm1
TEST_GENERIC phsubd, movq, mm0, mmx, mm1

TEST_GENERIC phsubw, movdqu, xmm0, xmm, xmm1
TEST_GENERIC phsubsw, movdqu, xmm0, xmm, xmm1
TEST_GENERIC phsubd, movdqu, xmm0, xmm, xmm1

TEST_GENERIC pmulhrsw, movq, mm0, mmx, mm1
TEST_GENERIC pmulhrsw, movdqu, xmm0, xmm, xmm1

TEST_GENERIC pmaddubsw, movq, mm0, mmx, mm1
TEST_GENERIC pmaddubsw, movdqu, xmm0, xmm, xmm1

TEST_GENERIC pshufb, movq, mm0, mmx, mm1
TEST_GENERIC pshufb, movdqu, xmm0, xmm, xmm1

%MACRO TEST_ALIGNR 5
;global _test_ %+ %1 %+ _ %+ %4
;global test_ %+ %1 %+ _ %+ %4
_test_ %+ %1 %+ _ %+ %4:
test_ %+ %1 %+ _ %+ %4:
   mov         edx, [ esp + 4 ]
   mov         eax, [ esp + 8 ]
   %2          %3, [ edx ]
   %2          %5, [ eax ]
   %1          %3, %5, 3
   %2          [ edx ], %3
   ret
%ENDMACRO

TEST_ALIGNR palignr, movq, mm0, mmx, mm1
TEST_ALIGNR palignr, movdqu, xmm0, xmm, xmm1
