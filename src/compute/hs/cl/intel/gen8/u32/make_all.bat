@ECHO OFF

SET HS_GEN=..\..\..\..\..\..\spinel\bin\x64\Debug\hs_gen

:: --- 32-bit keys ---

::
:: There appears to be an Intel compiler bug when using more than
:: 16 registers per lane so try a wider subgroup and narrower merging kernels
::
:: The current crop of Intel compilers are spilling way too much...
::

%HS_GEN% -v -a "opencl" -D HS_INTEL_GEN8 -t 1 -w 16 -r 8 -s 21504 -S 65536 -b 16 -B 48 -m 1 -M 1 -f 0 -F 0 -c 0 -C 0 -z

::
:: This should be the proper mapping onto the Intel GEN8+ subslices but the compiler is spilling
::
:: %HS_GEN% -v -a "opencl" -D HS_INTEL_GEN8 -t 1 -w 8 -r 32 -s 32768 -S 65536 -b 28 -B 56 -m 1 -M 1 -f 0 -F 0 -c 0 -C 0 -z
::

::
:: remove trailing whitespace from generated files
::

sed -i 's/[[:space:]]*$//' hs_config.h

::
:: preprocess and build kernels
::

make_inl_cl.bat hs_kernels.cl
