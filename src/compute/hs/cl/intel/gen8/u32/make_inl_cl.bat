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

SET PRE_CL=%~n1
SET PRE_CL=%PRE_CL%.pre.cl

SET PRE_SRC_INL=%~n1
SET PRE_SRC_INL=%PRE_SRC_INL%.pre.src.inl

SET PRE_BIN_IR=%~n1
SET PRE_BIN_IR=%PRE_BIN_IR%.pre.ir

SET PRE_BIN_INL=%~n1
SET PRE_BIN_INL=%PRE_BIN_INL%.pre.bin.inl

::
:: *.pre.cl
:: *.pre.src.inl
::

CMD /C clang-format -style=Mozilla -i %1
CMD /C cl -I . -I "%INTELOCLSDKROOT%\include" -D %OPENCL_PRE% -EP %1 -P -Fi"%PRE_CL%"
CMD /C clang-format -style=Mozilla -i %PRE_CL%
CMD /C dos2unix -q %PRE_CL%
CMD /C xxd -i %PRE_CL% %PRE_SRC_INL%

echo %PRE_CL%
echo %PRE_SRC_INL%

::
:: *.pre.cl
:: *.pre.src.inl
::

CMD /C touch %PRE_BIN_IR%
ECHO ON
@CMD /C %IOC% -cmd=build -bo="%IOC_IR_OPTS%" -device=gpu -input=%PRE_CL% -ir=%PRE_BIN_IR%
@ECHO OFF
CMD /C xxd -i %PRE_BIN_IR% %PRE_BIN_INL%

echo %PRE_BIN_IR%
echo %PRE_BIN_INL%


