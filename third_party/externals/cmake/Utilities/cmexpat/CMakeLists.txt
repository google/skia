PROJECT(CMEXPAT)

SET(expat_SRCS
  xmlparse.c
  xmltok.c
  xmlrole.c
)

INCLUDE(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
TEST_BIG_ENDIAN(CMEXPAT_BIGENDIAN)

INCLUDE_DIRECTORIES(
  "${CMEXPAT_BINARY_DIR}/.."
  )

IF(WIN32)
  IF(NOT BUILD_SHARED_LIBS)
    SET (CM_EXPAT_STATIC 1)
  ENDIF(NOT BUILD_SHARED_LIBS)
ENDIF(WIN32)

CONFIGURE_FILE(${CMEXPAT_SOURCE_DIR}/expatConfig.h.in
               ${CMEXPAT_BINARY_DIR}/expatConfig.h)
CONFIGURE_FILE(${CMEXPAT_SOURCE_DIR}/expatDllConfig.h.in
               ${CMEXPAT_BINARY_DIR}/expatDllConfig.h)
CONFIGURE_FILE(${CMEXPAT_SOURCE_DIR}/.NoDartCoverage
               ${CMEXPAT_BINARY_DIR}/.NoDartCoverage)
CONFIGURE_FILE(${CMEXPAT_SOURCE_DIR}/cm_expat_mangle.h
               ${CMEXPAT_BINARY_DIR}/cm_expat_mangle.h)
CONFIGURE_FILE(${CMEXPAT_SOURCE_DIR}/expat.h
               ${CMEXPAT_BINARY_DIR}/expat.h)

ADD_LIBRARY(cmexpat ${expat_SRCS})
INSTALL(FILES COPYING DESTINATION ${CMAKE_DOC_DIR}/cmexpat)
