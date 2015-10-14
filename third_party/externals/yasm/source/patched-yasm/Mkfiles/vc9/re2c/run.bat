cd ..\..\..\
%1 -s -o lc3bid.c modules\arch\lc3b\lc3bid.re
%1 -b -o nasm-token.c modules\parsers\nasm\nasm-token.re
%1 -b -o gas-token.c modules\parsers\gas\gas-token.re
