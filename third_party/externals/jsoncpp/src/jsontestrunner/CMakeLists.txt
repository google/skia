FIND_PACKAGE(PythonInterp 2.6 REQUIRED)

IF(JSONCPP_LIB_BUILD_SHARED)
  ADD_DEFINITIONS( -DJSON_DLL )
ENDIF(JSONCPP_LIB_BUILD_SHARED)

ADD_EXECUTABLE(jsontestrunner_exe
               main.cpp
               )
TARGET_LINK_LIBRARIES(jsontestrunner_exe jsoncpp_lib)
SET_TARGET_PROPERTIES(jsontestrunner_exe PROPERTIES OUTPUT_NAME jsontestrunner_exe)

IF(PYTHONINTERP_FOUND)
    # Run end to end parser/writer tests
    GET_PROPERTY(JSONTESTRUNNER_EXE_PATH TARGET jsontestrunner_exe PROPERTY LOCATION)
    SET(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../test)
    SET(RUNJSONTESTS_PATH ${TEST_DIR}/runjsontests.py)
    ADD_CUSTOM_TARGET(jsoncpp_readerwriter_tests ALL
                      "${PYTHON_EXECUTABLE}" -B "${RUNJSONTESTS_PATH}" "${JSONTESTRUNNER_EXE_PATH}" "${TEST_DIR}/data"
                      DEPENDS jsontestrunner_exe jsoncpp_test
                      )
    ADD_CUSTOM_TARGET(jsoncpp_check DEPENDS jsoncpp_readerwriter_tests)
ENDIF(PYTHONINTERP_FOUND)
