include(RunCMake)

run_cmake(XcodeFileType)
run_cmake(XcodeAttributeGenex)
run_cmake(XcodeAttributeGenexError)
run_cmake(XcodeObjectNeedsQuote)
if (NOT XCODE_VERSION VERSION_LESS 6)
  run_cmake(XcodePlatformFrameworks)
endif()
