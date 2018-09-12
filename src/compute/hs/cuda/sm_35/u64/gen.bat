@ECHO OFF

SET HS_GEN=..\..\..\..\..\spinel\bin\x64\Debug\hs_gen

:: --- 64-bit keys

:: %HS_GEN% -v -a "cuda" -D HS_NVIDIA_SM35 -t 2 -w 32 -r 32 -s 49152 -S 65536 -b 16 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -p 1 -P 1 -z
%HS_GEN% -v -a "cuda" -D HS_NVIDIA_SM35 -t 2 -w 32 -r 8 -s 32768 -S 32768 -b 16 -m 1 -M 1 -p 1 -P 1 -f 0 -F 0 -c 0 -C 0 -z

::
:: remove trailing whitespace from generated files
::

SET HS_SRC=hs_cuda_u64.cu

clang-format -style=Mozilla -i %HS_SRC%

sed -i 's/[[:space:]]*$//'     hs_cuda_config.h
sed -i 's/[[:space:]]*$//'     %HS_SRC%

::
:: preprocess and build kernels
::
:: if you're debugging you'll want to preprocess all the macros
::

REM SET HS_SRC=hs_cuda.pre.cu

REM cl -I . -EP hs_cuda.cu -P -Fi"%HS_SRC%"   || goto :error
REM clang-format -style=Mozilla -i %HS_SRC%   || goto :error
REM dos2unix -q %HS_SRC%                      || goto :error

::
:: build kernels
::

@ECHO ON

nvcc -I ../../../.. -use_fast_math -res-usage -cubin -arch sm_35 %HS_SRC%
