
IF(JSONCPP_LIB_BUILD_SHARED)
  ADD_DEFINITIONS( -DJSON_DLL )
ENDIF(JSONCPP_LIB_BUILD_SHARED)

ADD_EXECUTABLE( jsoncpp_test 
                jsontest.cpp
                jsontest.h
                main.cpp
                )

TARGET_LINK_LIBRARIES(jsoncpp_test jsoncpp_lib)

# Run unit tests in post-build
# (default cmake workflow hides away the test result into a file, resulting in poor dev workflow?!?)
IF(JSONCPP_WITH_POST_BUILD_UNITTEST)
    ADD_CUSTOM_COMMAND( TARGET jsoncpp_test
				        POST_BUILD
				        COMMAND jsoncpp_test)
ENDIF(JSONCPP_WITH_POST_BUILD_UNITTEST)

SET_TARGET_PROPERTIES(jsoncpp_test PROPERTIES OUTPUT_NAME jsoncpp_test) 
