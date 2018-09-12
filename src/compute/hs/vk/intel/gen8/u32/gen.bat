::
:: Copyright 2018 Google Inc.
::
:: Use of this source code is governed by a BSD-style license that can
:: be found in the LICENSE file.
::

@ECHO OFF

::
:: delete the previous images
::

del *.comp
del *.spv
del *.xxd

::
::
::

set HS_GEN=..\..\..\..\..\..\spinel\bin\x64\Debug\hs_gen

::
:: There appears to be an Intel compiler bug when using more than
:: 16 registers per lane so try a wider subgroup and narrower merging kernels
::
:: The current crop of Intel compilers are spilling way too much...
::

%HS_GEN% -v -a "glsl" -D HS_INTEL_GEN8 -t 1 -w 16 -r 8 -s 21504 -S 65536 -b 16 -B 48 -m 1 -M 1 -f 0 -F 0 -c 0 -C 0 -z

::
:: This should be the proper mapping onto the Intel GEN8+ subslices but the compiler is spilling
::
:: %HS_GEN% -v -a "glsl" -D HS_INTEL_GEN8 -t 1 -w 8 -r 32 -s 32768 -S 65536 -b 28 -B 56 -m 1 -M 1 -f 0 -F 0 -c 0 -C 0 -z
::

::
:: remove trailing whitespace from generated files
::

sed -i 's/[[:space:]]*$//' hs_config.h
sed -i 's/[[:space:]]*$//' hs_modules.h

::
::
::

where glslangValidator

::
:: FIXME -- convert this to a bash script
::
:: Note that we can use xargs instead of the cmd for/do
::

for %%f in (*.comp) do (
    dos2unix %%f
    clang-format -style=Mozilla -i %%f                                   || goto :error
    cl -I ../.. -I ../../.. -EP %%f -P -Fi%%~nf.pre.comp                 || goto :error
    clang-format -style=Mozilla -i %%~nf.pre.comp                        || goto :error
    glslangValidator --target-env vulkan1.1 -o %%~nf.spv %%~nf.pre.comp  || goto :error
    spirv-opt -O %%~nf.spv -o %%~nf.spv                                  || goto :error
REM spirv-remap ...                                                      || goto :error
    xxd -i < %%~nf.spv > %%~nf.spv.xxd                                   || goto :error
    for /f %%A in ('wc -c %%~nf.spv') do (
        echo %%~nf.spv %%A
        printf "%%.8x" %%A | xxd -r -p | xxd -i > %%~nf.len.xxd          || goto :error
    )
)

::
:: dump a binary
::

cl -I ../../.. -I ../../../../.. /DHS_DUMP /Fe:hs_dump.exe *.c
hs_dump

::
:: delete temporary files
::

del *.pre.comp
del *.comp
del *.spv
REM del *.obj
REM del *.exe

exit /b 0

:error

exit /b %errorlevel%
