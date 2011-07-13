@rem    Copyright 2011 Google Inc.
@rem
@rem    Licensed under the Apache License, Version 2.0 (the "License");
@rem    you may not use this file except in compliance with the License.
@rem    You may obtain a copy of the License at
@rem
@rem         http://www.apache.org/licenses/LICENSE-2.0
@rem
@rem    Unless required by applicable law or agreed to in writing, software
@rem    distributed under the License is distributed on an "AS IS" BASIS,
@rem    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
@rem    See the License for the specific language governing permissions and
@rem    limitations under the License.

@rem Launches make.py on Windows, after setting Visual Studio environment variables.
@rem See http://code.google.com/p/skia/wiki/GettingStartedOnWindows

@if "%DevEnvDir%"=="" goto setup_env_vars

:run_python
@rem Run make.py and propagate its return value.
python make.py %*
@exit /B %ERRORLEVEL%

:setup_env_vars
@rem Visual Studio environment variables aren't set yet, so run vcvars32.bat
@if "%VS100COMNTOOLS%"=="" goto error_no_VS100COMNTOOLS
call "%VS100COMNTOOLS%..\..\VC\bin\vcvars32.bat"
@if %ERRORLEVEL% neq 0 exit /B %ERRORLEVEL%
@goto run_python

:error_no_VS100COMNTOOLS
@echo ERROR: VS100COMNTOOLS environment variable not set.
@echo Are you sure Visual Studio 2010 is installed?
@exit /B 1
