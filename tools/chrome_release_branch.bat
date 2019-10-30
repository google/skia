@echo off
:: Copyright 2019 Google Inc.
::
:: Use of this source code is governed by a BSD-style license that can be
:: found in the LICENSE file.
setlocal
python -u "%~dp0\chrome_release_branch.py" %*
