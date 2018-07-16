@ECHO OFF

del *.comp
del *.pre.comp
del *.spv

REM
REM
REM

set HS_GEN=..\..\..\..\..\..\spinel\bin\x64\Debug\hs_gen

REM --- 32-bit keys ---

REM CMD /C %HS_GEN% -v -a "glsl" -t 1 -w 8 -r 24 -s 32768 -S 65536 -b 28 -B 56 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -z
REM CMD /C %HS_GEN% -v -a "glsl" -t 1 -w 8 -r 32 -s 21504 -S 65536 -b 16 -B 48 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -z
REM CMD /C %HS_GEN% -v -a "glsl" -t 1 -w 8 -r 32 -s 8192  -S 65536 -b 8  -B 56 -m 1 -M 1 -f 0 -F 0 -c 0 -C 0 -z

REM --- 64-bit keys

CMD /C %HS_GEN% -v -a "glsl" -t 2 -w 8 -r 16 -s 21504 -S 65536 -b 16 -B 48 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -z
REM CMD /C %HS_GEN% -v -a "glsl" -t 2 -w 8 -r 16 -s 32768 -S 65536 -b 28 -B 56 -m 1 -M 1 -f 0 -F 0 -c 0 -C 0 -z

REM CMD /C make_inl_cl.bat hs_cl.cl

for %%f in (*.comp) do (
    echo %%~nf
    dos2unix %%f
    clang-format -style=Mozilla -i %%f                                                            || goto :error
    cl -I . -EP %%f -P -Fi%%~nf.pre.comp                                                          || goto :error
    clang-format -style=Mozilla -i %%~nf.pre.comp                                                 || goto :error
    glslc --target-env=vulkan1.1 -std=450 -fshader-stage=compute -I . %%~nf.pre.comp -o %%~nf.spv || goto :error
    spirv-opt -O %%~nf.spv -o %%~nf.spv                                                           || goto :error
    xxd -i < %%~nf.spv > %%~nf.spv.xxd                                                            || goto :error
    for /f %%A in ('wc -c %%~nf.spv') do (
      printf "%%.8x" %%A | xxd -r -p | xxd -i > %%~nf.len.xxd
    )
)

del *.comp
del *.pre.comp
del *.spv

exit /b 0

:error

exit /b %errorlevel%
