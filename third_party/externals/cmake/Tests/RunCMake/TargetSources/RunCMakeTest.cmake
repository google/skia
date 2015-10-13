include(RunCMake)

if(RunCMake_GENERATOR MATCHES "Visual Studio|Xcode" AND NOT XCODE_BELOW_2)
  run_cmake(ConfigNotAllowed)
  run_cmake(OriginDebugIDE)
else()
  run_cmake(OriginDebug)
endif()

run_cmake(CMP0026-LOCATION)
run_cmake(RelativePathInInterface)
run_cmake(ExportBuild)
