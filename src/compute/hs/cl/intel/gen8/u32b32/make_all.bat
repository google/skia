@ECHO OFF

SET HS_GEN=..\..\..\..\..\spinel\bin\x64\Debug\hs_gen 

REM --- 32-bit keys ---

REM CMD /C %HS_GEN% -v -a "opencl" -t 1 -w 8 -r 24 -s 32768 -S 65536 -b 28 -B 56 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -z
REM CMD /C %HS_GEN% -v -a "opencl" -t 1 -w 8 -r 32 -s 21504 -S 65536 -b 16 -B 48 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -z
REM CMD /C %HS_GEN% -v -a "opencl" -t 1 -w 8 -r 32 -s 8192  -S 65536 -b 8  -B 56 -m 1 -M 1 -f 0 -F 0 -c 0 -C 0 -z

REM --- 64-bit keys

CMD /C %HS_GEN% -v -a "opencl" -t 2 -w 8 -r 16 -s 21504 -S 65536 -b 16 -B 48 -m 1 -M 1 -f 1 -F 1 -c 1 -C 1 -z
REM CMD /C %HS_GEN% -v -a "opencl" -t 2 -w 8 -r 16 -s 32768 -S 65536 -b 28 -B 56 -m 1 -M 1 -f 0 -F 0 -c 0 -C 0 -z 

CMD /C make_inl_cl.bat hs_cl.cl
