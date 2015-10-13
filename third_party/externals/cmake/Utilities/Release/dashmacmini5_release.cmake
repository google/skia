set(PROCESSORS 4)
set(CMAKE_RELEASE_DIRECTORY /Users/kitware/CMakeReleaseDirectory)
# set(USER_OVERRIDE "set(CMAKE_CXX_LINK_EXECUTABLE \\\"gcc  <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS>  -o <TARGET> <LINK_LIBRARIES>  -shared-libgcc -lstdc++-static\\\")")
set(INSTALL_PREFIX /)
set(HOST dashmacmini5)
set(MAKE_PROGRAM "make")
set(MAKE "${MAKE_PROGRAM} -j5")
set(CPACK_BINARY_GENERATORS "DragNDrop TGZ TZ")
set(CPACK_SOURCE_GENERATORS "TGZ TZ")
set(CPACK_DMG_FORMAT "UDBZ") #build using bzip2 for smaller package size
set(INITIAL_CACHE "
CMAKE_USE_OPENSSL:BOOL=OFF
OPENSSL_CRYPTO_LIBRARY:FILEPATH=/Users/kitware/openssl-1.0.1g-install/lib/libcrypto.a
OPENSSL_INCLUDE_DIR:PATH=/Users/kitware/openssl-1.0.1g-install/include
OPENSSL_SSL_LIBRARY:FILEPATH=/Users/kitware/openssl-1.0.1g-install/lib/libssl.a
CMAKE_BUILD_TYPE:STRING=Release
CMAKE_OSX_ARCHITECTURES:STRING=x86_64
CMAKE_OSX_DEPLOYMENT_TARGET:STRING=10.6
CMAKE_SKIP_BOOTSTRAP_TEST:STRING=TRUE
CPACK_SYSTEM_NAME:STRING=Darwin-x86_64
BUILD_QtDialog:BOOL=TRUE
CMake_GUI_DISTRIBUTE_WITH_Qt_LGPL:BOOL=TRUE
QT_QMAKE_EXECUTABLE:FILEPATH=/Users/kitware/Support/qt-4.8.0/install/bin/qmake
")
get_filename_component(path "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${path}/release_cmake.cmake)
