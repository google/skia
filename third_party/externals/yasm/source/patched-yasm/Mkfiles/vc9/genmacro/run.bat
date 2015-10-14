cd ..\..\..
%1 nasm-version.c nasm_version_mac version.mac
%1 nasm-macros.c nasm_standard_mac modules\parsers\nasm\nasm-std.mac
%1 win64-nasm.c win64_nasm_stdmac modules\objfmts\coff\win64-nasm.mac
%1 win64-gas.c win64_gas_stdmac modules\objfmts\coff\win64-gas.mac
