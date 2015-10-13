set(expect "TEST_HOST = \"[^;\"]*Tests/RunCMake/XcodeProject/XcodeAttributeGenex-build/[^;\"/]*/some\"")
file(STRINGS ${RunCMake_TEST_BINARY_DIR}/XcodeAttributeGenex.xcodeproj/project.pbxproj actual
     REGEX "TEST_HOST = .*;" LIMIT_COUNT 1)
if(NOT "${actual}" MATCHES "${expect}")
  message(SEND_ERROR "The actual project contains the line:\n ${actual}\n"
    "which does not match expected regex:\n ${expect}\n")
endif()
