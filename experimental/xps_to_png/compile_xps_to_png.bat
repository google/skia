@rem Copyright 2016 Google Inc.
@rem
@rem Use of this source code is governed by a BSD-style license that can be
@rem found in the LICENSE file.

@set CSC="C:\Program Files (x86)\MSBuild\14.0\Bin\amd64\csc.exe"
@set LIB="C:\Program Files (x86)\Reference Assemblies\Microsoft\Framework\.NETFramework\v4.5.2"

%CSC% /lib:%LIB% ^
/reference:"PresentationCore.dll" ^
/reference:"PresentationFramework.dll" ^
/reference:"ReachFramework.dll" ^
/reference:"WindowsBase.dll" ^
"%~dp0%xps_to_png.cs"
