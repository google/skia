@ECHO OFF

SET HS_GEN=..\..\..\..\..\..\spinel\bin\x64\Debug\hs_gen

:: --- 64-bit keys ---

::
:: This is not a great mapping to GEN but the compiler is not cooperating
::

%HS_GEN% -v -a "opencl" -D HS_INTEL_GEN8 -t 2 -w 8 -r 16 -s 21504 -S 65536 -b 16 -B 48 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -z

::
:: This should be the proper mapping onto the Intel GEN8+ subslices but the compiler is spilling
::
:: %HS_GEN% -v -a "opencl" -D HS_INTEL_GEN8 -t 2 -w 8 -r 16 -s 32768 -S 65536 -b 28 -B 56 -m 1 -M 1 -f 0 -F 0 -c 0 -C 0 -z
::

::
:: remove trailing whitespace from generated files
::

sed -i 's/[[:space:]]*$//' hs_config.h

::
:: preprocess and build kernels
::

make_inl_cl.bat hs_kernels.cl
