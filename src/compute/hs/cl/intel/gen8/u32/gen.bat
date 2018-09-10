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

SET OPENCL_STD=-cl-std=CL1.2
SET OPENCL_PRE=__OPENCL_C_VERSION__=120

:: SET OPENCL_STD=-cl-std=CL2.0
:: SET OPENCL_PRE=__OPENCL_C_VERSION__=200

::
::
::

SET IOC=ioc64

::
::
::

SET IOC_IR_OPTS_OPT=%OPENCL_STD% -cl-single-precision-constant -cl-denorms-are-zero -cl-mad-enable -cl-no-signed-zeros -cl-fast-relaxed-math -cl-kernel-arg-info

SET IOC_IR_OPTS_DBG=%OPENCL_STD% -cl-kernel-arg-info -g

SET IOC_IR_OPTS=%IOC_IR_OPTS_OPT%

::
::
::

SET PRE_SRC=hs_kernels.pre.cl
SET PRE_SRC_XXD=hs_kernels.src.xxd
SET PRE_SRC_LEN_XXD=hs_kernels.src.len.xxd

SET PRE_BIN=hs_kernels.bin
SET PRE_BIN_XXD=hs_kernels.bin.xxd
SET PRE_BIN_LEN_XXD=hs_kernels.bin.len.xxd

::
:: *.pre.cl
::

clang-format -style=Mozilla -i hs_kernels.cl                                                   || goto :error
cl -I ..\.. -I "%INTELOCLSDKROOT%\include" -D %OPENCL_PRE% -EP hs_kernels.cl -P -Fi"%PRE_SRC%" || goto :error
clang-format -style=Mozilla -i %PRE_SRC%                                                       || goto :error
dos2unix -q %PRE_SRC%                                                                          || goto :error

echo %PRE_SRC%

::
:: *.src.xxd
:: *.src.len.xxd
::

xxd -i < %PRE_SRC% > %PRE_SRC_XXD%  || goto :error

for /f %%A in ('wc -c %PRE_SRC%') do (
    echo %PRE_SRC% %%A
    printf "%%.8x" %%A | xxd -r -p | xxd -i > %PRE_SRC_LEN_XXD%  || goto :error
)

echo %PRE_SRC_XXD%
echo %PRE_SRC_LEN_XXD%

::
:: *.pre.bin
::

%IOC% -cmd=build -bo="%IOC_IR_OPTS%" -device=gpu -input=%PRE_SRC% -ir=%PRE_BIN%  || goto :error

echo %PRE_BIN%

::
:: *.bin.xxd
:: *.bin.len.xxd
::

xxd -i < %PRE_BIN% > %PRE_BIN_XXD%  || goto :error

for /f %%A in ('wc -c %PRE_BIN%') do (
    echo %PRE_BIN% %%A
    printf "%%.8x" %%A | xxd -r -p | xxd -i > %PRE_BIN_LEN_XXD%  || goto :error
)

echo %PRE_BIN_XXD%
echo %PRE_BIN_LEN_XXD%

::
:: dump a binary
::

cl -I ../../../../.. -I ../../.. /DHS_DUMP /Fe:hs_dump.exe *.c
hs_dump

::
:: delete temporary files
::

:: del hs_target.bin
:: del *.pre.cl
del *.obj
del *.exe

exit /b 0

:error

exit /b %errorlevel%
