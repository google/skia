@rem Copyright 2016 Google Inc.
@rem
@rem Use of this source code is governed by a BSD-style license that can be
@rem found in the LICENSE file.

"C:\PROGRA~2\MSBUILD\14.0\BIN\AMD64\CSC.EXE" ^
/lib:"\PROGRA~2\REFERE~1\MICROS~1\FRAMEW~1\NETFRA~1\V4.5.2" ^
/reference:"ReachFramework.dll" ^
/reference:"WindowsBase.dll" ^
/reference:"PresentationCore.dll" ^
/reference:"PresentationFramework.dll" ^
"%~dp0%xps_to_png.cs"


