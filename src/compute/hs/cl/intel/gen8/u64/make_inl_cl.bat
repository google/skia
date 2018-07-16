@ECHO OFF

::
::
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

SET PRE_DIR=%~p1

CD %PRE_DIR%

SET PRE_SRC=%~n1.pre.cl
SET PRE_SRC_XXD=%~n1.src.xxd
SET PRE_SRC_LEN_XXD=%~n1.src.len.xxd

SET PRE_BIN=%~n1.bin
SET PRE_BIN_XXD=%~n1.bin.xxd
SET PRE_BIN_LEN_XXD=%~n1.bin.len.xxd

::
:: *.pre.cl
::

clang-format -style=Mozilla -i %1                                               || goto :error
cl -I . -I "%INTELOCLSDKROOT%\include" -D %OPENCL_PRE% -EP %1 -P -Fi"%PRE_SRC%" || goto :error
clang-format -style=Mozilla -i %PRE_SRC%                                        || goto :error
dos2unix -q %PRE_SRC%                                                           || goto :error

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

cl /DHS_DUMP /Fe:hs_dump.exe /Tchs_target.h
hs_dump

::
:: delete temporary files
::

:: del *.pre.cl
del *.obj
del *.exe

exit /b 0

:error

exit /b %errorlevel%
