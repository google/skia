@ECHO OFF

::
:: delete the previous images
::

del *.pre.comp
del *.comp
del *.spv
del *.xxd

::
::
::

set HS_GEN=..\..\..\..\..\..\spinel\bin\x64\Debug\hs_gen

:: --- 32-bit keys ---

:: CMD /C %HS_GEN% -v -a "glsl" -t 1 -w 8 -r 24 -s 32768 -S 65536 -b 28 -B 56 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -z
:: CMD /C %HS_GEN% -v -a "glsl" -t 1 -w 8 -r 32 -s 21504 -S 65536 -b 16 -B 48 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -z
:: CMD /C %HS_GEN% -v -a "glsl" -t 1 -w 8 -r 32 -s 8192  -S 65536 -b 8  -B 56 -m 1 -M 1 -f 0 -F 0 -c 0 -C 0 -z

:: --- 64-bit keys

%HS_GEN% -v -a "glsl" -t 2 -w 8 -r 16 -s 21504 -S 65536 -b 16 -B 48 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -z
:: CMD /C %HS_GEN% -v -a "glsl" -t 2 -w 8 -r 16 -s 32768 -S 65536 -b 28 -B 56 -m 1 -M 1 -f 0 -F 0 -c 0 -C 0 -z

::
:: remove trailing whitespace from generated files
::

sed -i 's/[[:space:]]*$//' hs_glsl.h
sed -i 's/[[:space:]]*$//' hs_kernels.h

::
:: FIXME -- convert this to a bash script
::
:: Note that we can use xargs instead of the cmd for/do
::

for %%f in (*.comp) do (
    dos2unix %%f
    clang-format -style=Mozilla -i %%f                                      || goto :error
    cl -I . -EP %%f -P -Fi%%~nf.pre.comp                                    || goto :error
    clang-format -style=Mozilla -i %%~nf.pre.comp                           || goto :error
:: glslangValidator -V110 -o %%~nf.spv %%~nf.pre.comp                       || goto :error
    glslc --target-env=vulkan1.1 -std=460 -I . -o %%~nf.spv %%~nf.pre.comp  || goto :error
    spirv-opt -O %%~nf.spv -o %%~nf.spv                                     || goto :error
:: spirv-remap ...                                                          || goto :error
    xxd -i < %%~nf.spv > %%~nf.spv.xxd                                      || goto :error
    for /f %%A in ('wc -c %%~nf.spv') do (
        echo %%~nf.spv %%A
        printf "%%.8x" %%A | xxd -r -p | xxd -i > %%~nf.len.xxd             || goto :error
    )
)

::
:: dump a binary
::

cl /DHS_DUMP /Fe:hs_dump.exe /Tchs_target.h
hs_dump

::
:: delete temporary files
::

:: del *.pre.comp
del *.comp
del *.spv
del *.obj
del *.exe

exit /b 0

:error

exit /b %errorlevel%
