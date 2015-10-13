#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
include(CheckIncludeFile)
# Check if we can build support for ELF parsing.
if(CMAKE_CXX_PLATFORM_ID MATCHES "OpenBSD")
  CHECK_INCLUDE_FILES("stdint.h;elf_abi.h" HAVE_ELF_H)
else()
  CHECK_INCLUDE_FILE("elf.h" HAVE_ELF_H)
endif()
if(HAVE_ELF_H)
  set(CMAKE_USE_ELF_PARSER 1)
elseif(HAIKU)
  # On Haiku, we need to include elf32.h from the private headers
  set(CMake_HAIKU_INCLUDE_DIRS
    /boot/system/develop/headers/private/system
    /boot/system/develop/headers/private/system/arch/x86
    )

  set(CMAKE_REQUIRED_INCLUDES ${CMake_HAIKU_INCLUDE_DIRS})
  CHECK_INCLUDE_FILE("elf32.h" HAVE_ELF32_H)
  unset(CMAKE_REQUIRED_INCLUDES)

  if(HAVE_ELF32_H)
    set(CMAKE_USE_ELF_PARSER 1)
  else()
    unset(CMake_HAIKU_INCLUDE_DIRS)
    set(CMAKE_USE_ELF_PARSER)
  endif()
else()
  set(CMAKE_USE_ELF_PARSER)
endif()

if(APPLE)
  set(CMAKE_USE_MACH_PARSER 1)
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMake_BIN_DIR})

# ensure Unicode friendly APIs are used on Windows
if(WIN32)
  add_definitions(-DUNICODE -D_UNICODE)
endif()

# configure the .h file
configure_file(
  "${CMake_SOURCE_DIR}/Source/cmConfigure.cmake.h.in"
  "${CMake_BINARY_DIR}/Source/cmConfigure.h"
  )
configure_file(
  "${CMake_SOURCE_DIR}/Source/cmVersionConfig.h.in"
  "${CMake_BINARY_DIR}/Source/cmVersionConfig.h"
  )
configure_file(
  "${CMake_SOURCE_DIR}/Source/CPack/cmCPackConfigure.h.in"
  "${CMake_BINARY_DIR}/Source/CPack/cmCPackConfigure.h"
  )

# Tell CMake executable in the build tree where to find the source tree.
configure_file(
  "${CMake_SOURCE_DIR}/Source/CMakeSourceDir.txt.in"
  "${CMake_BINARY_DIR}/CMakeFiles/CMakeSourceDir.txt" @ONLY
  )

# add the include path to find the .h
include_directories(
  "${CMake_BINARY_DIR}/Source"
  "${CMake_SOURCE_DIR}/Source"
  ${CMAKE_ZLIB_INCLUDES}
  ${CMAKE_EXPAT_INCLUDES}
  ${CMAKE_TAR_INCLUDES}
  ${CMAKE_COMPRESS_INCLUDES}
  ${CMake_HAIKU_INCLUDE_DIRS}
  )

# let cmake know it is supposed to use it
add_definitions(-DCMAKE_BUILD_WITH_CMAKE)

option(CMAKE_REGENERATE_YACCLEX
  "Regenerate YACC and LEXX files" OFF)
mark_as_advanced(CMAKE_REGENERATE_YACCLEX)
if(CMAKE_REGENERATE_YACCLEX)
  set(parsersLexers cmDependsFortran cmCommandArgument cmExpr)
  find_program(YACC_EXECUTABLE
    NAMES yacc bison
    PATHS /usr/bin
    DOC "Yacc or Bison executable")
  find_program(FLEX_EXECUTABLE
    NAMES flex
    PATHS /usr/bin
    DOC "Flex executable")
  mark_as_advanced(YACC_EXECUTABLE FLEX_EXECUTABLE)
  if(YACC_EXECUTABLE)
    set(BISON_FLAGS)
    if(YACC_EXECUTABLE MATCHES "bison")
      set(BISON_FLAGS "--yacc")
    endif()
    set(yacc_files)
    foreach(name ${parsersLexers})
      set(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}Parser.y")
      set(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}Parser.cxx")
      set(hdr "${CMAKE_CURRENT_BINARY_DIR}/${name}ParserTokens.h")
      add_custom_command(
        OUTPUT "${dst}"
        DEPENDS "${src}"
        COMMAND
        ${YACC_EXECUTABLE}
        --name-prefix=${name}_yy --defines=${hdr} -o${dst} ${src}
        )
      set(yacc_files ${yacc_files} "${dst}")
    endforeach()
    add_custom_target(RerunYacc DEPENDS ${yacc_files})
  endif()
  if(FLEX_EXECUTABLE)
    set(lex_files)
    foreach(name ${parsersLexers})
      set(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}Lexer.in.l")
      set(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}Lexer.cxx")
      set(hdr "${CMAKE_CURRENT_BINARY_DIR}/${name}Lexer.h")
      add_custom_command(
        OUTPUT "${dst}"
        DEPENDS "${src}"
        COMMAND
        ${FLEX_EXECUTABLE}
        --prefix=${name}_yy --header-file=${hdr} -o${dst} ${src}
        )
      set(lex_files ${lex_files} "${dst}")
    endforeach()
    add_custom_target(RerunLex DEPENDS ${lex_files})
  endif()

endif()

# Check if we can build the ELF parser.
if(CMAKE_USE_ELF_PARSER)
  set(ELF_SRCS cmELF.h cmELF.cxx)
endif()

# Check if we can build the Mach-O parser.
if(CMAKE_USE_MACH_PARSER)
  set(MACH_SRCS cmMachO.h cmMachO.cxx)
endif()

#
# Sources for CMakeLib
#
set(SRCS
  cmArchiveWrite.cxx
  cmBootstrapCommands1.cxx
  cmBootstrapCommands2.cxx
  cmCacheManager.cxx
  cmCacheManager.h
  "${CMAKE_CURRENT_BINARY_DIR}/cmCommands.cxx"
  cmCLocaleEnvironmentScope.h
  cmCLocaleEnvironmentScope.cxx
  cmCommands.h
  cmCommandArgumentLexer.cxx
  cmCommandArgumentParser.cxx
  cmCommandArgumentParserHelper.cxx
  cmComputeComponentGraph.cxx
  cmComputeComponentGraph.h
  cmComputeLinkDepends.cxx
  cmComputeLinkDepends.h
  cmComputeLinkInformation.cxx
  cmComputeLinkInformation.h
  cmComputeTargetDepends.h
  cmComputeTargetDepends.cxx
  cmCPackPropertiesGenerator.h
  cmCPackPropertiesGenerator.cxx
  cmCryptoHash.cxx
  cmCryptoHash.h
  cmCurl.cxx
  cmCurl.h
  cmCustomCommand.cxx
  cmCustomCommand.h
  cmCustomCommandGenerator.cxx
  cmCustomCommandGenerator.h
  cmDefinitions.cxx
  cmDefinitions.h
  cmDepends.cxx
  cmDepends.h
  cmDependsC.cxx
  cmDependsC.h
  cmDependsFortran.cxx
  cmDependsFortran.h
  cmDependsFortranLexer.cxx
  cmDependsFortranParser.cxx
  cmDependsFortranParser.h
  cmDependsJava.cxx
  cmDependsJava.h
  cmDependsJavaLexer.cxx
  cmDependsJavaParser.cxx
  cmDependsJavaParserHelper.cxx
  cmDependsJavaParserHelper.h
  cmDocumentation.cxx
  cmDocumentationFormatter.cxx
  cmDocumentationSection.cxx
  cmDynamicLoader.cxx
  cmDynamicLoader.h
  ${ELF_SRCS}
  cmExprLexer.cxx
  cmExprParser.cxx
  cmExprParserHelper.cxx
  cmExportBuildFileGenerator.h
  cmExportBuildFileGenerator.cxx
  cmExportFileGenerator.h
  cmExportFileGenerator.cxx
  cmExportInstallFileGenerator.h
  cmExportInstallFileGenerator.cxx
  cmExportTryCompileFileGenerator.h
  cmExportTryCompileFileGenerator.cxx
  cmExportSet.h
  cmExportSet.cxx
  cmExportSetMap.h
  cmExportSetMap.cxx
  cmExternalMakefileProjectGenerator.cxx
  cmExternalMakefileProjectGenerator.h
  cmExtraCodeBlocksGenerator.cxx
  cmExtraCodeBlocksGenerator.h
  cmExtraCodeLiteGenerator.cxx
  cmExtraCodeLiteGenerator.h
  cmExtraEclipseCDT4Generator.cxx
  cmExtraEclipseCDT4Generator.h
  cmExtraKateGenerator.cxx
  cmExtraKateGenerator.h
  cmExtraSublimeTextGenerator.cxx
  cmExtraSublimeTextGenerator.h
  cmFileLock.cxx
  cmFileLock.h
  cmFileLockPool.cxx
  cmFileLockPool.h
  cmFileLockResult.cxx
  cmFileLockResult.h
  cmFileTimeComparison.cxx
  cmFileTimeComparison.h
  cmGeneratedFileStream.cxx
  cmGeneratorExpressionContext.cxx
  cmGeneratorExpressionContext.h
  cmGeneratorExpressionDAGChecker.cxx
  cmGeneratorExpressionDAGChecker.h
  cmGeneratorExpressionEvaluationFile.cxx
  cmGeneratorExpressionEvaluationFile.h
  cmGeneratorExpressionEvaluator.cxx
  cmGeneratorExpressionEvaluator.h
  cmGeneratorExpressionLexer.cxx
  cmGeneratorExpressionLexer.h
  cmGeneratorExpressionNode.cxx
  cmGeneratorExpressionNode.h
  cmGeneratorExpressionParser.cxx
  cmGeneratorExpressionParser.h
  cmGeneratorExpression.cxx
  cmGeneratorExpression.h
  cmGeneratorTarget.cxx
  cmGeneratorTarget.h
  cmGlobalGenerator.cxx
  cmGlobalGenerator.h
  cmGlobalGeneratorFactory.h
  cmGlobalUnixMakefileGenerator3.cxx
  cmGlobalUnixMakefileGenerator3.h
  cmGraphAdjacencyList.h
  cmGraphVizWriter.cxx
  cmGraphVizWriter.h
  cmInstallGenerator.h
  cmInstallGenerator.cxx
  cmInstallExportGenerator.cxx
  cmInstalledFile.h
  cmInstalledFile.cxx
  cmInstallFilesGenerator.h
  cmInstallFilesGenerator.cxx
  cmInstallScriptGenerator.h
  cmInstallScriptGenerator.cxx
  cmInstallTargetGenerator.h
  cmInstallTargetGenerator.cxx
  cmInstallDirectoryGenerator.h
  cmInstallDirectoryGenerator.cxx
  cmListFileCache.cxx
  cmListFileCache.h
  cmListFileLexer.c
  cmLocalGenerator.cxx
  cmLocalGenerator.h
  cmLocalUnixMakefileGenerator3.cxx
  cmLocale.h
  ${MACH_SRCS}
  cmMakeDepend.cxx
  cmMakeDepend.h
  cmMakefile.cxx
  cmMakefile.h
  cmMakefileTargetGenerator.cxx
  cmMakefileExecutableTargetGenerator.cxx
  cmMakefileLibraryTargetGenerator.cxx
  cmMakefileUtilityTargetGenerator.cxx
  cmOSXBundleGenerator.cxx
  cmOSXBundleGenerator.h
  cmNewLineStyle.h
  cmNewLineStyle.cxx
  cmOrderDirectories.cxx
  cmOrderDirectories.h
  cmPolicies.h
  cmPolicies.cxx
  cmProcessTools.cxx
  cmProcessTools.h
  cmProperty.cxx
  cmProperty.h
  cmPropertyDefinition.cxx
  cmPropertyDefinition.h
  cmPropertyDefinitionMap.cxx
  cmPropertyDefinitionMap.h
  cmPropertyMap.cxx
  cmPropertyMap.h
  cmQtAutoGenerators.cxx
  cmQtAutoGenerators.h
  cmRST.cxx
  cmRST.h
  cmScriptGenerator.h
  cmScriptGenerator.cxx
  cmSourceFile.cxx
  cmSourceFile.h
  cmSourceFileLocation.cxx
  cmSourceFileLocation.h
  cmSourceGroup.cxx
  cmSourceGroup.h
  cmState.cxx
  cmState.h
  cmSystemTools.cxx
  cmSystemTools.h
  cmTarget.cxx
  cmTarget.h
  cmTargetExport.h
  cmTest.cxx
  cmTest.h
  cmTestGenerator.cxx
  cmTestGenerator.h
  cmUuid.cxx
  cmVariableWatch.cxx
  cmVariableWatch.h
  cmVersion.cxx
  cmVersion.h
  cmXMLParser.cxx
  cmXMLParser.h
  cmXMLSafe.cxx
  cmXMLSafe.h
  cmXMLWriter.cxx
  cmXMLWriter.h
  cmake.cxx
  cmake.h

  cm_get_date.h
  cm_get_date.c
  cm_sha2.h
  cm_sha2.c
  cm_utf8.h
  cm_utf8.c
  )

set(COMMAND_INCLUDES "#include \"cmTargetPropCommandBase.cxx\"\n")
list(APPEND SRCS cmTargetPropCommandBase.cxx)
set_property(SOURCE cmTargetPropCommandBase.cxx PROPERTY HEADER_FILE_ONLY ON)
set(NEW_COMMANDS "")
foreach(command_file
    cmAddCompileOptionsCommand
    cmAuxSourceDirectoryCommand
    cmBuildNameCommand
    cmCMakeHostSystemInformationCommand
    cmElseIfCommand
    cmExportCommand
    cmExportLibraryDependenciesCommand
    cmFLTKWrapUICommand
    cmIncludeExternalMSProjectCommand
    cmInstallProgramsCommand
    cmLinkLibrariesCommand
    cmLoadCacheCommand
    cmOutputRequiredFilesCommand
    cmQTWrapCPPCommand
    cmQTWrapUICommand
    cmRemoveCommand
    cmRemoveDefinitionsCommand
    cmSourceGroupCommand
    cmSubdirDependsCommand
    cmTargetCompileDefinitionsCommand
    cmTargetCompileFeaturesCommand
    cmTargetCompileOptionsCommand
    cmTargetIncludeDirectoriesCommand
    cmTargetSourcesCommand
    cmUseMangledMesaCommand
    cmUtilitySourceCommand
    cmVariableRequiresCommand
    cmVariableWatchCommand
    cmWriteFileCommand
    # This one must be last because it includes windows.h and
    # windows.h #defines GetCurrentDirectory which is a member
    # of cmMakefile
    cmLoadCommandCommand
    )
  set(COMMAND_INCLUDES "${COMMAND_INCLUDES}#include \"${command_file}.cxx\"\n")
  set(NEW_COMMANDS "${NEW_COMMANDS}commands.push_back(new ${command_file});\n")
  list(APPEND SRCS ${command_file}.cxx)
  set_property(SOURCE ${command_file}.cxx PROPERTY HEADER_FILE_ONLY ON)
endforeach()
configure_file(cmCommands.cxx.in ${CMAKE_CURRENT_BINARY_DIR}/cmCommands.cxx @ONLY)

# Kdevelop only works on UNIX and not windows
if(UNIX)
  set(SRCS ${SRCS} cmGlobalKdevelopGenerator.cxx)
endif()

# Xcode only works on Apple
if(APPLE)
  set(SRCS ${SRCS}
    cmXCodeObject.cxx
    cmXCode21Object.cxx
    cmGlobalXCodeGenerator.cxx
    cmGlobalXCodeGenerator.h
    cmLocalXCodeGenerator.cxx
    cmLocalXCodeGenerator.h)
endif()


if (WIN32)
  set(SRCS ${SRCS}
    cmCallVisualStudioMacro.cxx
    cmCallVisualStudioMacro.h
    )

  if(NOT UNIX)
    set(SRCS ${SRCS}
      cmGlobalBorlandMakefileGenerator.cxx
      cmGlobalBorlandMakefileGenerator.h
      cmGlobalMSYSMakefileGenerator.cxx
      cmGlobalMinGWMakefileGenerator.cxx
      cmGlobalNMakeMakefileGenerator.cxx
      cmGlobalNMakeMakefileGenerator.h
      cmGlobalJOMMakefileGenerator.cxx
      cmGlobalJOMMakefileGenerator.h
      cmGlobalVisualStudio6Generator.cxx
      cmGlobalVisualStudio6Generator.h
      cmGlobalVisualStudio71Generator.cxx
      cmGlobalVisualStudio71Generator.h
      cmGlobalVisualStudio7Generator.cxx
      cmGlobalVisualStudio7Generator.h
      cmGlobalVisualStudio8Generator.cxx
      cmGlobalVisualStudio8Generator.h
      cmGlobalVisualStudio9Generator.cxx
      cmGlobalVisualStudio9Generator.h
      cmVisualStudioGeneratorOptions.h
      cmVisualStudioGeneratorOptions.cxx
      cmVisualStudio10TargetGenerator.h
      cmVisualStudio10TargetGenerator.cxx
      cmLocalVisualStudio10Generator.cxx
      cmLocalVisualStudio10Generator.h
      cmGlobalVisualStudio10Generator.h
      cmGlobalVisualStudio10Generator.cxx
      cmGlobalVisualStudio11Generator.h
      cmGlobalVisualStudio11Generator.cxx
      cmGlobalVisualStudio12Generator.h
      cmGlobalVisualStudio12Generator.cxx
      cmGlobalVisualStudio14Generator.h
      cmGlobalVisualStudio14Generator.cxx
      cmGlobalVisualStudioGenerator.cxx
      cmGlobalVisualStudioGenerator.h
      cmIDEFlagTable.h
      cmIDEOptions.cxx
      cmIDEOptions.h
      cmLocalVisualStudio6Generator.cxx
      cmLocalVisualStudio6Generator.h
      cmLocalVisualStudio7Generator.cxx
      cmLocalVisualStudio7Generator.h
      cmLocalVisualStudioGenerator.cxx
      cmLocalVisualStudioGenerator.h
      cmVisualStudioSlnData.h
      cmVisualStudioSlnData.cxx
      cmVisualStudioSlnParser.h
      cmVisualStudioSlnParser.cxx
      cmVisualStudioWCEPlatformParser.h
      cmVisualStudioWCEPlatformParser.cxx
      cmGlobalGhsMultiGenerator.cxx
      cmGlobalGhsMultiGenerator.h
      cmLocalGhsMultiGenerator.cxx
      cmLocalGhsMultiGenerator.h
      cmGhsMultiTargetGenerator.cxx
      cmGhsMultiTargetGenerator.h
      cmGhsMultiGpj.cxx
      cmGhsMultiGpj.h
      )
  endif()
endif ()

# Watcom support
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set_property(SOURCE cmake.cxx APPEND PROPERTY COMPILE_DEFINITIONS CMAKE_USE_WMAKE)
  list(APPEND SRCS
    cmGlobalWatcomWMakeGenerator.cxx
    cmGlobalWatcomWMakeGenerator.h
    )
endif()

# Ninja support
set(SRCS ${SRCS}
  cmGlobalNinjaGenerator.cxx
  cmGlobalNinjaGenerator.h
  cmNinjaTypes.h
  cmLocalNinjaGenerator.cxx
  cmLocalNinjaGenerator.h
  cmNinjaTargetGenerator.cxx
  cmNinjaTargetGenerator.h
  cmNinjaNormalTargetGenerator.cxx
  cmNinjaNormalTargetGenerator.h
  cmNinjaUtilityTargetGenerator.cxx
  cmNinjaUtilityTargetGenerator.h
  )

if(WIN32 AND NOT CYGWIN)
  set_source_files_properties(cmcldeps.cxx PROPERTIES COMPILE_DEFINITIONS _WIN32_WINNT=0x0501)
  add_executable(cmcldeps cmcldeps.cxx)
  target_link_libraries(cmcldeps CMakeLib)
  install(TARGETS cmcldeps DESTINATION bin)
endif()

foreach(v CURL_CA_BUNDLE CURL_CA_PATH)
  if(${v})
    set_property(SOURCE cmCurl.cxx APPEND PROPERTY COMPILE_DEFINITIONS ${v}="${${v}}")
  endif()
endforeach()

# create a library used by the command line and the GUI
add_library(CMakeLib ${SRCS})
target_link_libraries(CMakeLib cmsys
  ${CMAKE_EXPAT_LIBRARIES} ${CMAKE_ZLIB_LIBRARIES}
  ${CMAKE_TAR_LIBRARIES} ${CMAKE_COMPRESS_LIBRARIES}
  ${CMAKE_CURL_LIBRARIES}
  ${CMAKE_JSONCPP_LIBRARIES}
  )

# On Apple we need CoreFoundation
if(APPLE)
  target_link_libraries(CMakeLib "-framework CoreFoundation")
endif()

if(CMAKE_BUILD_ON_VISUAL_STUDIO OR MINGW)
  # We need the rpcrt4 library for at least the VS7-VC10 generators.
  target_link_libraries(CMakeLib rpcrt4)
endif()

#
# CTestLib
#
include_directories(
  "${CMake_SOURCE_DIR}/Source/CTest"
  ${CMAKE_XMLRPC_INCLUDES}
  ${CMAKE_CURL_INCLUDES}
  )
#
# Sources for CTestLib
#
set(CTEST_SRCS cmCTest.cxx
  CTest/cmProcess.cxx
  CTest/cmCTestBatchTestHandler.cxx
  CTest/cmCTestBuildAndTestHandler.cxx
  CTest/cmCTestBuildCommand.cxx
  CTest/cmCTestBuildHandler.cxx
  CTest/cmCTestConfigureCommand.cxx
  CTest/cmCTestConfigureHandler.cxx
  CTest/cmCTestCoverageCommand.cxx
  CTest/cmCTestCoverageHandler.cxx
  CTest/cmCTestCurl.cxx
  CTest/cmParseMumpsCoverage.cxx
  CTest/cmParseCacheCoverage.cxx
  CTest/cmParseGTMCoverage.cxx
  CTest/cmParseJacocoCoverage.cxx
  CTest/cmParseBlanketJSCoverage.cxx
  CTest/cmParsePHPCoverage.cxx
  CTest/cmParseCoberturaCoverage.cxx
  CTest/cmParseDelphiCoverage.cxx
  CTest/cmCTestEmptyBinaryDirectoryCommand.cxx
  CTest/cmCTestGenericHandler.cxx
  CTest/cmCTestHandlerCommand.cxx
  CTest/cmCTestLaunch.cxx
  CTest/cmCTestMemCheckCommand.cxx
  CTest/cmCTestMemCheckHandler.cxx
  CTest/cmCTestMultiProcessHandler.cxx
  CTest/cmCTestReadCustomFilesCommand.cxx
  CTest/cmCTestRunScriptCommand.cxx
  CTest/cmCTestRunTest.cxx
  CTest/cmCTestScriptHandler.cxx
  CTest/cmCTestSleepCommand.cxx
  CTest/cmCTestStartCommand.cxx
  CTest/cmCTestSubmitCommand.cxx
  CTest/cmCTestSubmitHandler.cxx
  CTest/cmCTestTestCommand.cxx
  CTest/cmCTestTestHandler.cxx
  CTest/cmCTestUpdateCommand.cxx
  CTest/cmCTestUpdateHandler.cxx
  CTest/cmCTestUploadCommand.cxx
  CTest/cmCTestUploadHandler.cxx

  CTest/cmCTestVC.cxx
  CTest/cmCTestVC.h
  CTest/cmCTestGlobalVC.cxx
  CTest/cmCTestGlobalVC.h
  CTest/cmCTestCVS.cxx
  CTest/cmCTestCVS.h
  CTest/cmCTestSVN.cxx
  CTest/cmCTestSVN.h
  CTest/cmCTestBZR.cxx
  CTest/cmCTestBZR.h
  CTest/cmCTestGIT.cxx
  CTest/cmCTestGIT.h
  CTest/cmCTestHG.cxx
  CTest/cmCTestHG.h
  CTest/cmCTestP4.cxx
  CTest/cmCTestP4.h
  )

# Build CTestLib
add_library(CTestLib ${CTEST_SRCS})
target_link_libraries(CTestLib CMakeLib ${CMAKE_CURL_LIBRARIES} ${CMAKE_XMLRPC_LIBRARIES})

#
# Sources for CPack
#
set(CPACK_SRCS
  CPack/cmCPackArchiveGenerator.cxx
  CPack/cmCPackComponentGroup.cxx
  CPack/cmCPackGeneratorFactory.cxx
  CPack/cmCPackGenerator.cxx
  CPack/cmCPackLog.cxx
  CPack/cmCPackNSISGenerator.cxx
  CPack/IFW/cmCPackIFWPackage.cxx
  CPack/IFW/cmCPackIFWInstaller.cxx
  CPack/IFW/cmCPackIFWGenerator.cxx
  CPack/cmCPackSTGZGenerator.cxx
  CPack/cmCPackTGZGenerator.cxx
  CPack/cmCPackTXZGenerator.cxx
  CPack/cmCPackTarBZip2Generator.cxx
  CPack/cmCPackTarCompressGenerator.cxx
  CPack/cmCPackZIPGenerator.cxx
  CPack/cmCPack7zGenerator.cxx
  )

if(CYGWIN)
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackCygwinBinaryGenerator.cxx
    CPack/cmCPackCygwinSourceGenerator.cxx
    )
endif()

if(UNIX)
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackDebGenerator.cxx
    CPack/cmCPackRPMGenerator.cxx
    )
endif()

if(WIN32)
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/WiX/cmCPackWIXGenerator.cxx
    CPack/WiX/cmCPackWIXGenerator.h
    CPack/WiX/cmWIXAccessControlList.cxx
    CPack/WiX/cmWIXAccessControlList.h
    CPack/WiX/cmWIXDirectoriesSourceWriter.cxx
    CPack/WiX/cmWIXDirectoriesSourceWriter.h
    CPack/WiX/cmWIXFeaturesSourceWriter.cxx
    CPack/WiX/cmWIXFeaturesSourceWriter.h
    CPack/WiX/cmWIXFilesSourceWriter.cxx
    CPack/WiX/cmWIXFilesSourceWriter.h
    CPack/WiX/cmWIXPatch.cxx
    CPack/WiX/cmWIXPatch.h
    CPack/WiX/cmWIXPatchParser.cxx
    CPack/WiX/cmWIXPatchParser.h
    CPack/WiX/cmWIXRichTextFormatWriter.cxx
    CPack/WiX/cmWIXRichTextFormatWriter.h
    CPack/WiX/cmWIXShortcut.cxx
    CPack/WiX/cmWIXShortcut.h
    CPack/WiX/cmWIXSourceWriter.cxx
    CPack/WiX/cmWIXSourceWriter.h
  )
endif()

if(APPLE)
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackBundleGenerator.cxx
    CPack/cmCPackDragNDropGenerator.cxx
    CPack/cmCPackOSXX11Generator.cxx
    CPack/cmCPackPackageMakerGenerator.cxx
    )
endif()

# Build CPackLib
add_library(CPackLib ${CPACK_SRCS})
target_link_libraries(CPackLib CMakeLib)

if(APPLE)
  add_executable(cmakexbuild cmakexbuild.cxx)
  target_link_libraries(cmakexbuild CMakeLib)
  add_executable(OSXScriptLauncher
    CPack/OSXScriptLauncher.cxx)
  target_link_libraries(OSXScriptLauncher cmsys)
  target_link_libraries(OSXScriptLauncher "-framework CoreFoundation")
endif()

# Build CMake executable
add_executable(cmake cmakemain.cxx cmcmd.cxx cmcmd.h)
target_link_libraries(cmake CMakeLib)

# Build CTest executable
add_executable(ctest ctest.cxx)
target_link_libraries(ctest CTestLib)

# Build CPack executable
add_executable(cpack CPack/cpack.cxx)
target_link_libraries(cpack CPackLib)

# Curses GUI
if(BUILD_CursesDialog)
  include(${CMake_SOURCE_DIR}/Source/CursesDialog/CMakeLists.txt)
endif()

# Qt GUI
option(BUILD_QtDialog "Build Qt dialog for CMake" FALSE)
if(BUILD_QtDialog)
  add_subdirectory(QtDialog)
endif()

include (${CMake_BINARY_DIR}/Source/LocalUserOptions.cmake OPTIONAL)
include (${CMake_SOURCE_DIR}/Source/LocalUserOptions.cmake OPTIONAL)

install(TARGETS cmake ctest cpack DESTINATION bin)
if(APPLE)
  install(TARGETS cmakexbuild DESTINATION bin)
endif()

install(FILES cmCPluginAPI.h DESTINATION ${CMAKE_DATA_DIR}/include)
