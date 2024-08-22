:: Copyright 2024 Google LLC
::
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.

external\clang_windows_amd64\bin\llvm-ar.exe %*
if %errorlevel% neq 0 exit /b %errorlevel%
