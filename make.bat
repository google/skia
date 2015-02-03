@rem    Copyright 2011 Google Inc.
@rem
@rem    Use of this source code is governed by a BSD-style license that can be
@rem    found in the LICENSE file.
@ECHO OFF

rem Launches make.py on Windows, after setting Visual Studio environment variables.
rem See https://skia.org/user/quick/windows.

rem Skip environment setup on bots.
if "%CHROME_HEADLESS%"=="1" goto run_python

if "%DevEnvDir%"=="" goto setup_env_vars

:run_python
rem Run make.py and propagate its return value.
python make.py %*
exit /B %ERRORLEVEL%

:setup_env_vars
rem Visual Studio environment variables aren't set yet, so run vcvars32.bat
if DEFINED VS110COMNTOOLS (
    call "%VS110COMNTOOLS%..\..\VC\bin\vcvars32.bat"
) else if DEFINED VS100COMNTOOLS (
    call "%VS100COMNTOOLS%..\..\VC\bin\vcvars32.bat"
) else (
    goto error_no_VS
)
if %ERRORLEVEL% neq 0 exit /B %ERRORLEVEL%
goto run_python

:error_no_VS
echo ERROR: Neither VS100COMNTOOLS nor VS110COMNTOOLS environment variable is set.
echo Are you sure Visual Studio 2010 or 2012 is installed?
exit /B 1
