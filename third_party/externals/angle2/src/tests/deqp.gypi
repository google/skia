# Copyright 2015 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables':
    {
        # Define these variables within an inner variables dict.
        # This is necessary to get these variables defined for the conditions
        # within the outer variables dict which operate on these variables.
        'variables':
        {
            'angle_build_winrt%': 0,
            'angle_standalone%': 0,
        },

        # Copy conditionally-set variables to the outer variables dict.
        'angle_build_winrt%': '<(angle_build_winrt)',
        'angle_standalone%': '<(angle_standalone)',

        'deqp_path': '<(DEPTH)/third_party/deqp/src',
        'delibs_path': '<(deqp_path)/framework/delibs',
        'libpng_path': '<(DEPTH)/third_party/libpng',
        'zlib_path': '<(DEPTH)/third_party/zlib',

        'angle_build_deqp_libraries%' : 0,
        'angle_build_deqp_gtest_support%' : 0,
        'angle_build_deqp_executables%' : 0,
        'angle_build_deqp_gtest_executables%' :0,

        'conditions':
        [
            ['(OS=="win" or OS=="linux" or OS=="mac")',
            {
                # Build the dEQP libraries for all Windows/Linux builds
                'angle_build_deqp_libraries%': 1,
            }],
            ['((OS=="win" or OS=="linux" or OS=="mac") and angle_build_winrt==0)',
            {
                # Build the dEQP GoogleTest support helpers for all Windows/Linux builds except WinRT
                # GoogleTest doesn't support WinRT
                'angle_build_deqp_gtest_support%': 1,
            }],
            ['((OS=="win" or OS=="linux" or OS=="mac") and angle_standalone==1 and angle_build_winrt==0)',
            {
                # Build the dEQP executables for all standalone Windows/Linux builds except WinRT
                # GYP doesn't support generating standalone WinRT executables
                'angle_build_deqp_executables%': 1,

                # Build the GoogleTest versions of dEQP for all standalone Windows/Linux builds except WinRT
                # GoogleTest doesn't support WinRT
                'angle_build_deqp_gtest_executables%': 1,
            }],

            ['OS=="win"',
            {
                'deqp_include_dirs':
                [
                    '<(deqp_path)/framework/platform/win32',
                ],
                'deqp_libtester_sources':
                [
                    '<(deqp_path)/framework/delibs/dethread/win32/deMutexWin32.c',
                    '<(deqp_path)/framework/delibs/dethread/win32/deSemaphoreWin32.c',
                    '<(deqp_path)/framework/delibs/dethread/win32/deThreadLocalWin32.c',
                    '<(deqp_path)/framework/delibs/dethread/win32/deThreadWin32.c',
                ],
            }],
            ['OS=="linux" and use_x11==1',
            {
                'deqp_include_dirs':
                [
                    '<(deqp_path)/framework/platform/x11',
                ],
                'deqp_defines':
                [
                    # Ask the system headers to expose all the regular function otherwise
                    # dEQP doesn't compile and produces warnings about implicitly defined
                    # functions.
                    # This has to be GNU_SOURCE as on Linux dEQP uses syscall()
                    '_GNU_SOURCE',
                ],
            }],
            ['OS=="mac"',
            {
                'deqp_include_dirs':
                [
                    '<(deqp_path)/framework/platform/osx',
                ],
                'deqp_defines':
                [
                    # Ask the system headers to expose all the regular function otherwise
                    # dEQP doesn't compile and produces warnings about implicitly defined
                    # functions.
                    '_XOPEN_SOURCE=600',
                ],
            }],
            ['(OS=="linux" and use_x11==1) or OS=="mac"',
            {
                'deqp_libtester_sources':
                [
                    '<(deqp_path)/framework/delibs/dethread/unix/deMutexUnix.c',
                    '<(deqp_path)/framework/delibs/dethread/unix/deNamedSemaphoreUnix.c',
                    '<(deqp_path)/framework/delibs/dethread/unix/deSemaphoreUnix.c',
                    '<(deqp_path)/framework/delibs/dethread/unix/deThreadLocalUnix.c',
                    '<(deqp_path)/framework/delibs/dethread/unix/deThreadUnix.c',
                ],
            }],
        ],

        'deqp_msvs_disabled_warnings':
        [
            '4091', # typedef ignored when no variable is declared
            '4100',
            '4127', # conditional expression constant
            '4244', # possible loss of data
            '4245', # argument signed/unsigned mismatch
            '4297', # function assumed not to throw an exception but does
            '4389', # signed/unsigned mismatch
            '4510', # default constructor could not be generated
            '4512',
            '4610', # cannot be instantiated
            '4611', # setjmp interaction non-portable
            '4701', # potentially uninit used
            '4702', # unreachable code
            '4706',
            '4838', # conversion requires a narrowing conversion
            '4996', # deprecated
        ],
        'deqp_defines':
        [
            'DEQP_SUPPORT_GLES3=1',
            'DEQP_SUPPORT_GLES2=1',
            'DEQP_SUPPORT_EGL=1',
            'DEQP_TARGET_NAME="angle"',
            'DEQP_GLES3_RUNTIME_LOAD=1',
            'DEQP_GLES2_RUNTIME_LOAD=1',
            'QP_SUPPORT_PNG=1',
            '_HAS_EXCEPTIONS=1',
        ],
        'deqp_undefines':
        [
            'WIN32_LEAN_AND_MEAN',
            'NOMINMAX',
            '_HAS_EXCEPTIONS=0',
        ],
        'deqp_include_dirs':
        [
            '<(libpng_path)',
            '<(zlib_path)',
            '<(delibs_path)/debase',
            '<(delibs_path)/decpp',
            '<(delibs_path)/depool',
            '<(delibs_path)/dethread',
            '<(delibs_path)/deutil',
            '<(delibs_path)/destream',
            '<(deqp_path)/framework/common',
            '<(deqp_path)/framework/qphelper',
            '<(deqp_path)/framework/platform/null',
            '<(deqp_path)/framework/egl',
            '<(deqp_path)/framework/egl/wrapper',
            '<(deqp_path)/framework/opengl',
            '<(deqp_path)/framework/opengl/wrapper',
            '<(deqp_path)/framework/referencerenderer',
            '<(deqp_path)/framework/opengl/simplereference',
            '<(deqp_path)/framework/randomshaders',
            '<(deqp_path)/modules/gles2',
            '<(deqp_path)/modules/gles2/functional',
            '<(deqp_path)/modules/gles2/accuracy',
            '<(deqp_path)/modules/gles2/performance',
            '<(deqp_path)/modules/gles2/stress',
            '<(deqp_path)/modules/gles2/usecases',
            '<(deqp_path)/modules/gles3',
            '<(deqp_path)/modules/gles3/functional',
            '<(deqp_path)/modules/gles3/accuracy',
            '<(deqp_path)/modules/gles3/performance',
            '<(deqp_path)/modules/gles3/stress',
            '<(deqp_path)/modules/gles3/usecases',
            '<(deqp_path)/modules/gles31',
            '<(deqp_path)/modules/gles31/functional',
            '<(deqp_path)/modules/gles31/stress',
            '<(deqp_path)/modules/glshared',
            '<(deqp_path)/modules/glusecases',
            '<(deqp_path)/executor',
            '<(deqp_path)/execserver',
        ],
        'deqp_gles2_sources':
        [
            '<(deqp_path)/modules/gles2/accuracy/es2aAccuracyTests.cpp',
            '<(deqp_path)/modules/gles2/accuracy/es2aAccuracyTests.hpp',
            '<(deqp_path)/modules/gles2/accuracy/es2aTextureFilteringTests.cpp',
            '<(deqp_path)/modules/gles2/accuracy/es2aTextureFilteringTests.hpp',
            '<(deqp_path)/modules/gles2/accuracy/es2aTextureMipmapTests.cpp',
            '<(deqp_path)/modules/gles2/accuracy/es2aTextureMipmapTests.hpp',
            '<(deqp_path)/modules/gles2/accuracy/es2aVaryingInterpolationTests.cpp',
            '<(deqp_path)/modules/gles2/accuracy/es2aVaryingInterpolationTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fApiCase.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fApiCase.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fAttribLocationTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fAttribLocationTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fBlendTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fBlendTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fBooleanStateQueryTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fBooleanStateQueryTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fBufferObjectQueryTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fBufferObjectQueryTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fBufferTestUtil.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fBufferTestUtil.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fBufferWriteTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fBufferWriteTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fClippingTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fClippingTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fColorClearTest.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fColorClearTest.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fDebugMarkerTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fDebugMarkerTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fDefaultVertexAttributeTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fDefaultVertexAttributeTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fDepthRangeTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fDepthRangeTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fDepthStencilClearTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fDepthStencilClearTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fDepthStencilTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fDepthStencilTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fDepthTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fDepthTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fDitheringTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fDitheringTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fDrawTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fDrawTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fFboApiTest.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fFboApiTest.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fFboCompletenessTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fFboCompletenessTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fFboRenderTest.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fFboRenderTest.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fFboStateQueryTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fFboStateQueryTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fFloatStateQueryTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fFloatStateQueryTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fFlushFinishTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fFlushFinishTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fFragOpInteractionTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fFragOpInteractionTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fFunctionalTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fFunctionalTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fImplementationLimitTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fImplementationLimitTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fIntegerStateQueryTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fIntegerStateQueryTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fLifetimeTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fLifetimeTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fLightAmountTest.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fLightAmountTest.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fMultisampleTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fMultisampleTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeBufferApiTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeBufferApiTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeFragmentApiTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeFragmentApiTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeShaderApiTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeShaderApiTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeStateApiTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeStateApiTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeTextureApiTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeTextureApiTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeVertexArrayApiTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fNegativeVertexArrayApiTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fPolygonOffsetTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fPolygonOffsetTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fPrerequisiteTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fPrerequisiteTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fRandomFragmentOpTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fRandomFragmentOpTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fRandomShaderTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fRandomShaderTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fRasterizationTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fRasterizationTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fRboStateQueryTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fRboStateQueryTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fReadPixelsTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fReadPixelsTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fScissorTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fScissorTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderAlgorithmTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderAlgorithmTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderApiTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderApiTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderBuiltinVarTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderBuiltinVarTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderConstExprTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderConstExprTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderDiscardTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderDiscardTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderExecuteTest.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderExecuteTest.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderFragDataTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderFragDataTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderIndexingTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderIndexingTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderInvarianceTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderInvarianceTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderLoopTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderLoopTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderMatrixTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderMatrixTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderOperatorTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderOperatorTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderReturnTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderReturnTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderStateQueryTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderStateQueryTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderStructTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderStructTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderTextureFunctionTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fShaderTextureFunctionTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fStencilTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fStencilTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fStringQueryTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fStringQueryTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureCompletenessTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureCompletenessTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureFilteringTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureFilteringTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureFormatTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureFormatTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureMipmapTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureMipmapTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureSizeTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureSizeTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureSpecificationTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureSpecificationTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureStateQueryTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureStateQueryTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureUnitTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureUnitTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureWrapTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fTextureWrapTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fUniformApiTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fUniformApiTests.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fVertexArrayTest.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fVertexArrayTest.hpp',
            '<(deqp_path)/modules/gles2/functional/es2fVertexTextureTests.cpp',
            '<(deqp_path)/modules/gles2/functional/es2fVertexTextureTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pBlendTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pBlendTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pDrawCallBatchingTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pDrawCallBatchingTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pPerformanceTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pPerformanceTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pRedundantStateChangeTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pRedundantStateChangeTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pShaderCompilationCases.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pShaderCompilationCases.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pShaderCompilerTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pShaderCompilerTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pShaderControlStatementTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pShaderControlStatementTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pShaderOperatorTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pShaderOperatorTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pShaderOptimizationTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pShaderOptimizationTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pStateChangeCallTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pStateChangeCallTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pStateChangeTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pStateChangeTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pTextureCases.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pTextureCases.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pTextureCountTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pTextureCountTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pTextureFilteringTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pTextureFilteringTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pTextureFormatTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pTextureFormatTests.hpp',
            '<(deqp_path)/modules/gles2/performance/es2pTextureUploadTests.cpp',
            '<(deqp_path)/modules/gles2/performance/es2pTextureUploadTests.hpp',
            '<(deqp_path)/modules/gles2/stress/es2sDrawTests.cpp',
            '<(deqp_path)/modules/gles2/stress/es2sDrawTests.hpp',
            '<(deqp_path)/modules/gles2/stress/es2sLongRunningTests.cpp',
            '<(deqp_path)/modules/gles2/stress/es2sLongRunningTests.hpp',
            '<(deqp_path)/modules/gles2/stress/es2sMemoryTests.cpp',
            '<(deqp_path)/modules/gles2/stress/es2sMemoryTests.hpp',
            '<(deqp_path)/modules/gles2/stress/es2sSpecialFloatTests.cpp',
            '<(deqp_path)/modules/gles2/stress/es2sSpecialFloatTests.hpp',
            '<(deqp_path)/modules/gles2/stress/es2sStressTests.cpp',
            '<(deqp_path)/modules/gles2/stress/es2sStressTests.hpp',
            '<(deqp_path)/modules/gles2/stress/es2sVertexArrayTests.cpp',
            '<(deqp_path)/modules/gles2/stress/es2sVertexArrayTests.hpp',
            '<(deqp_path)/modules/gles2/tes2CapabilityTests.cpp',
            '<(deqp_path)/modules/gles2/tes2CapabilityTests.hpp',
            '<(deqp_path)/modules/gles2/tes2Context.cpp',
            '<(deqp_path)/modules/gles2/tes2Context.hpp',
            '<(deqp_path)/modules/gles2/tes2InfoTests.cpp',
            '<(deqp_path)/modules/gles2/tes2InfoTests.hpp',
            '<(deqp_path)/modules/gles2/tes2TestCase.cpp',
            '<(deqp_path)/modules/gles2/tes2TestCase.hpp',
            '<(deqp_path)/modules/gles2/tes2TestPackage.cpp',
            '<(deqp_path)/modules/gles2/tes2TestPackage.hpp',
            '<(deqp_path)/modules/gles2/tes2TestPackageEntry.cpp',
        ],
        'deqp_gles3_sources':
        [
            '<(deqp_path)/modules/gles3/accuracy/es3aAccuracyTests.cpp',
            '<(deqp_path)/modules/gles3/accuracy/es3aAccuracyTests.hpp',
            '<(deqp_path)/modules/gles3/accuracy/es3aTextureFilteringTests.cpp',
            '<(deqp_path)/modules/gles3/accuracy/es3aTextureFilteringTests.hpp',
            '<(deqp_path)/modules/gles3/accuracy/es3aTextureMipmapTests.cpp',
            '<(deqp_path)/modules/gles3/accuracy/es3aTextureMipmapTests.hpp',
            '<(deqp_path)/modules/gles3/accuracy/es3aVaryingInterpolationTests.cpp',
            '<(deqp_path)/modules/gles3/accuracy/es3aVaryingInterpolationTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fApiCase.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fApiCase.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fASTCDecompressionCases.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fASTCDecompressionCases.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fAttribLocationTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fAttribLocationTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fBlendTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fBlendTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fBooleanStateQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fBooleanStateQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fBufferCopyTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fBufferCopyTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fBufferMapTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fBufferMapTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fBufferObjectQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fBufferObjectQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fBufferWriteTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fBufferWriteTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fBuiltinPrecisionTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fBuiltinPrecisionTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fClippingTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fClippingTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fColorClearTest.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fColorClearTest.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fCompressedTextureTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fCompressedTextureTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fDefaultVertexArrayObjectTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fDefaultVertexArrayObjectTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fDefaultVertexAttributeTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fDefaultVertexAttributeTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fDepthStencilClearTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fDepthStencilClearTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fDepthStencilTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fDepthStencilTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fDepthTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fDepthTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fDitheringTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fDitheringTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fDrawTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fDrawTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboApiTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboApiTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboColorbufferTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboColorbufferTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboCompletenessTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboCompletenessTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboDepthbufferTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboDepthbufferTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboInvalidateTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboInvalidateTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboMultisampleTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboMultisampleTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboRenderTest.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboRenderTest.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboStateQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboStateQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboStencilbufferTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboStencilbufferTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboTestCase.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboTestCase.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboTestUtil.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFboTestUtil.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFloatStateQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFloatStateQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFlushFinishTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFlushFinishTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFragDepthTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFragDepthTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFragmentOutputTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFragmentOutputTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFragOpInteractionTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFragOpInteractionTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFramebufferBlitTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFramebufferBlitTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fFunctionalTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fFunctionalTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fImplementationLimitTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fImplementationLimitTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fIndexedStateQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fIndexedStateQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fInstancedRenderingTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fInstancedRenderingTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fInteger64StateQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fInteger64StateQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fIntegerStateQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fIntegerStateQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fInternalFormatQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fInternalFormatQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fLifetimeTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fLifetimeTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fMultisampleTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fMultisampleTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeBufferApiTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeBufferApiTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeFragmentApiTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeFragmentApiTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeShaderApiTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeShaderApiTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeStateApiTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeStateApiTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeTextureApiTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeTextureApiTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeVertexArrayApiTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fNegativeVertexArrayApiTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fOcclusionQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fOcclusionQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fPixelBufferObjectTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fPixelBufferObjectTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fPolygonOffsetTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fPolygonOffsetTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fPrerequisiteTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fPrerequisiteTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fPrimitiveRestartTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fPrimitiveRestartTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fRandomFragmentOpTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fRandomFragmentOpTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fRandomShaderTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fRandomShaderTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fRasterizationTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fRasterizationTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fRasterizerDiscardTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fRasterizerDiscardTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fRboStateQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fRboStateQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fReadPixelsTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fReadPixelsTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fSamplerObjectTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fSamplerObjectTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fSamplerStateQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fSamplerStateQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fScissorTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fScissorTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderApiTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderApiTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderBuiltinVarTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderBuiltinVarTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderCommonFunctionTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderCommonFunctionTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderConstExprTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderConstExprTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderDerivateTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderDerivateTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderDiscardTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderDiscardTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderFragDataTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderFragDataTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderIndexingTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderIndexingTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderInvarianceTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderInvarianceTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderLoopTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderLoopTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderMatrixTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderMatrixTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderOperatorTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderOperatorTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderPackingFunctionTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderPackingFunctionTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderPrecisionTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderPrecisionTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderReturnTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderReturnTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderStateQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderStateQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderStructTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderStructTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderSwitchTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderSwitchTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderTextureFunctionTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fShaderTextureFunctionTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fStencilTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fStencilTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fStringQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fStringQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fSyncTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fSyncTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureFilteringTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureFilteringTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureFormatTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureFormatTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureMipmapTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureMipmapTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureShadowTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureShadowTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureSizeTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureSizeTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureSpecificationTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureSpecificationTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureStateQueryTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureStateQueryTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureSwizzleTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureSwizzleTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureUnitTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureUnitTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureWrapTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fTextureWrapTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fTransformFeedbackTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fTransformFeedbackTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fUniformApiTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fUniformApiTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fUniformBlockTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fUniformBlockTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fVertexArrayObjectTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fVertexArrayObjectTests.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fVertexArrayTest.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fVertexArrayTest.hpp',
            '<(deqp_path)/modules/gles3/functional/es3fVertexTextureTests.cpp',
            '<(deqp_path)/modules/gles3/functional/es3fVertexTextureTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pBlendTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pBlendTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pBufferDataUploadTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pBufferDataUploadTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pDepthTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pDepthTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pPerformanceTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pPerformanceTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pRedundantStateChangeTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pRedundantStateChangeTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pShaderCompilationCases.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pShaderCompilationCases.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pShaderCompilerTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pShaderCompilerTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pShaderControlStatementTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pShaderControlStatementTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pShaderOperatorTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pShaderOperatorTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pShaderOptimizationTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pShaderOptimizationTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pStateChangeCallTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pStateChangeCallTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pStateChangeTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pStateChangeTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pTextureCases.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pTextureCases.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pTextureCountTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pTextureCountTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pTextureFilteringTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pTextureFilteringTests.hpp',
            '<(deqp_path)/modules/gles3/performance/es3pTextureFormatTests.cpp',
            '<(deqp_path)/modules/gles3/performance/es3pTextureFormatTests.hpp',
            '<(deqp_path)/modules/gles3/stress/es3sDrawTests.cpp',
            '<(deqp_path)/modules/gles3/stress/es3sDrawTests.hpp',
            '<(deqp_path)/modules/gles3/stress/es3sLongRunningShaderTests.cpp',
            '<(deqp_path)/modules/gles3/stress/es3sLongRunningShaderTests.hpp',
            '<(deqp_path)/modules/gles3/stress/es3sLongRunningTests.cpp',
            '<(deqp_path)/modules/gles3/stress/es3sLongRunningTests.hpp',
            '<(deqp_path)/modules/gles3/stress/es3sLongShaderTests.cpp',
            '<(deqp_path)/modules/gles3/stress/es3sLongShaderTests.hpp',
            '<(deqp_path)/modules/gles3/stress/es3sMemoryTests.cpp',
            '<(deqp_path)/modules/gles3/stress/es3sMemoryTests.hpp',
            '<(deqp_path)/modules/gles3/stress/es3sOcclusionQueryTests.cpp',
            '<(deqp_path)/modules/gles3/stress/es3sOcclusionQueryTests.hpp',
            '<(deqp_path)/modules/gles3/stress/es3sSpecialFloatTests.cpp',
            '<(deqp_path)/modules/gles3/stress/es3sSpecialFloatTests.hpp',
            '<(deqp_path)/modules/gles3/stress/es3sStressTests.cpp',
            '<(deqp_path)/modules/gles3/stress/es3sStressTests.hpp',
            '<(deqp_path)/modules/gles3/stress/es3sSyncTests.cpp',
            '<(deqp_path)/modules/gles3/stress/es3sSyncTests.hpp',
            '<(deqp_path)/modules/gles3/stress/es3sVertexArrayTests.cpp',
            '<(deqp_path)/modules/gles3/stress/es3sVertexArrayTests.hpp',
            '<(deqp_path)/modules/gles3/tes3Context.cpp',
            '<(deqp_path)/modules/gles3/tes3Context.hpp',
            '<(deqp_path)/modules/gles3/tes3InfoTests.cpp',
            '<(deqp_path)/modules/gles3/tes3InfoTests.hpp',
            '<(deqp_path)/modules/gles3/tes3TestCase.cpp',
            '<(deqp_path)/modules/gles3/tes3TestCase.hpp',
            '<(deqp_path)/modules/gles3/tes3TestPackage.cpp',
            '<(deqp_path)/modules/gles3/tes3TestPackage.hpp',
            '<(deqp_path)/modules/gles3/tes3TestPackageEntry.cpp',
        ],
        'deqp_egl_sources':
        [
            '<(deqp_path)/modules/egl/teglAndroidUtil.cpp',
            '<(deqp_path)/modules/egl/teglAndroidUtil.hpp',
            '<(deqp_path)/modules/egl/teglApiCase.cpp',
            '<(deqp_path)/modules/egl/teglApiCase.hpp',
            '<(deqp_path)/modules/egl/teglChooseConfigReference.cpp',
            '<(deqp_path)/modules/egl/teglChooseConfigReference.hpp',
            '<(deqp_path)/modules/egl/teglChooseConfigTests.cpp',
            '<(deqp_path)/modules/egl/teglChooseConfigTests.hpp',
            '<(deqp_path)/modules/egl/teglQueryConfigTests.cpp',
            '<(deqp_path)/modules/egl/teglQueryConfigTests.hpp',
            '<(deqp_path)/modules/egl/teglColorClearCase.cpp',
            '<(deqp_path)/modules/egl/teglColorClearCase.hpp',
            '<(deqp_path)/modules/egl/teglColorClearTests.cpp',
            '<(deqp_path)/modules/egl/teglColorClearTests.hpp',
            '<(deqp_path)/modules/egl/teglConfigList.cpp',
            '<(deqp_path)/modules/egl/teglConfigList.hpp',
            '<(deqp_path)/modules/egl/teglCreateContextTests.cpp',
            '<(deqp_path)/modules/egl/teglCreateContextTests.hpp',
            '<(deqp_path)/modules/egl/teglQueryContextTests.cpp',
            '<(deqp_path)/modules/egl/teglQueryContextTests.hpp',
            '<(deqp_path)/modules/egl/teglCreateSurfaceTests.cpp',
            '<(deqp_path)/modules/egl/teglCreateSurfaceTests.hpp',
            '<(deqp_path)/modules/egl/teglQuerySurfaceTests.cpp',
            '<(deqp_path)/modules/egl/teglQuerySurfaceTests.hpp',
            '<(deqp_path)/modules/egl/teglGetProcAddressTests.cpp',
            '<(deqp_path)/modules/egl/teglGetProcAddressTests.hpp',
            '<(deqp_path)/modules/egl/teglGLES1RenderUtil.cpp',
            '<(deqp_path)/modules/egl/teglGLES1RenderUtil.hpp',
            '<(deqp_path)/modules/egl/teglGLES2RenderUtil.cpp',
            '<(deqp_path)/modules/egl/teglGLES2RenderUtil.hpp',
            '<(deqp_path)/modules/egl/teglImageUtil.cpp',
            '<(deqp_path)/modules/egl/teglImageUtil.hpp',
            '<(deqp_path)/modules/egl/teglImageTests.cpp',
            '<(deqp_path)/modules/egl/teglImageTests.hpp',
            '<(deqp_path)/modules/egl/teglInfoTests.cpp',
            '<(deqp_path)/modules/egl/teglInfoTests.hpp',
            '<(deqp_path)/modules/egl/teglNegativeApiTests.cpp',
            '<(deqp_path)/modules/egl/teglNegativeApiTests.hpp',
            '<(deqp_path)/modules/egl/teglRenderCase.cpp',
            '<(deqp_path)/modules/egl/teglRenderCase.hpp',
            '<(deqp_path)/modules/egl/teglRenderTests.cpp',
            '<(deqp_path)/modules/egl/teglRenderTests.hpp',
            '<(deqp_path)/modules/egl/teglSimpleConfigCase.cpp',
            '<(deqp_path)/modules/egl/teglSimpleConfigCase.hpp',
            '<(deqp_path)/modules/egl/teglTestCase.cpp',
            '<(deqp_path)/modules/egl/teglTestCase.hpp',
            '<(deqp_path)/modules/egl/teglTestPackage.cpp',
            '<(deqp_path)/modules/egl/teglTestPackage.hpp',
            '<(deqp_path)/modules/egl/teglTestPackageEntry.cpp',
            '<(deqp_path)/modules/egl/teglVGRenderUtil.cpp',
            '<(deqp_path)/modules/egl/teglVGRenderUtil.hpp',
            '<(deqp_path)/modules/egl/teglImageFormatTests.hpp',
            '<(deqp_path)/modules/egl/teglImageFormatTests.cpp',
            '<(deqp_path)/modules/egl/teglGLES2SharingTests.hpp',
            '<(deqp_path)/modules/egl/teglGLES2SharingTests.cpp',
            '<(deqp_path)/modules/egl/teglGLES2SharingThreadedTests.hpp',
            '<(deqp_path)/modules/egl/teglGLES2SharingThreadedTests.cpp',
            '<(deqp_path)/modules/egl/teglSyncTests.hpp',
            '<(deqp_path)/modules/egl/teglSyncTests.cpp',
            '<(deqp_path)/modules/egl/teglMultiThreadTests.hpp',
            '<(deqp_path)/modules/egl/teglMultiThreadTests.cpp',
            '<(deqp_path)/modules/egl/teglMemoryStressTests.hpp',
            '<(deqp_path)/modules/egl/teglMemoryStressTests.cpp',
            '<(deqp_path)/modules/egl/teglMakeCurrentPerfTests.hpp',
            '<(deqp_path)/modules/egl/teglMakeCurrentPerfTests.cpp',
            '<(deqp_path)/modules/egl/teglGLES2SharedRenderingPerfTests.hpp',
            '<(deqp_path)/modules/egl/teglGLES2SharedRenderingPerfTests.cpp',
            '<(deqp_path)/modules/egl/teglPreservingSwapTests.hpp',
            '<(deqp_path)/modules/egl/teglPreservingSwapTests.cpp',
            '<(deqp_path)/modules/egl/teglClientExtensionTests.hpp',
            '<(deqp_path)/modules/egl/teglClientExtensionTests.cpp',
            '<(deqp_path)/modules/egl/teglCreateContextExtTests.hpp',
            '<(deqp_path)/modules/egl/teglCreateContextExtTests.cpp',
            '<(deqp_path)/modules/egl/teglSurfacelessContextTests.hpp',
            '<(deqp_path)/modules/egl/teglSurfacelessContextTests.cpp',
            '<(deqp_path)/modules/egl/teglSwapBuffersTests.hpp',
            '<(deqp_path)/modules/egl/teglSwapBuffersTests.cpp',
            '<(deqp_path)/modules/egl/teglNativeColorMappingTests.hpp',
            '<(deqp_path)/modules/egl/teglNativeColorMappingTests.cpp',
            '<(deqp_path)/modules/egl/teglNativeCoordMappingTests.hpp',
            '<(deqp_path)/modules/egl/teglNativeCoordMappingTests.cpp',
            '<(deqp_path)/modules/egl/teglResizeTests.hpp',
            '<(deqp_path)/modules/egl/teglResizeTests.cpp',
        ],
        'deqp_libtester_decpp_sources':
        [
            '<(deqp_path)/framework/delibs/decpp/deArrayBuffer.cpp',
            '<(deqp_path)/framework/delibs/decpp/deBlockBuffer.cpp',
            '<(deqp_path)/framework/delibs/decpp/deCommandLine.cpp',
            '<(deqp_path)/framework/delibs/decpp/deDefs.cpp',
            '<(deqp_path)/framework/delibs/decpp/deDirectoryIterator.cpp',
            '<(deqp_path)/framework/delibs/decpp/deDynamicLibrary.cpp',
            '<(deqp_path)/framework/delibs/decpp/deFilePath.cpp',
            '<(deqp_path)/framework/delibs/decpp/deMemPool.cpp',
            '<(deqp_path)/framework/delibs/decpp/deMutex.cpp',
            '<(deqp_path)/framework/delibs/decpp/dePoolArray.cpp',
            '<(deqp_path)/framework/delibs/decpp/dePoolString.cpp',
            '<(deqp_path)/framework/delibs/decpp/deProcess.cpp',
            '<(deqp_path)/framework/delibs/decpp/deRandom.cpp',
            '<(deqp_path)/framework/delibs/decpp/deRingBuffer.cpp',
            '<(deqp_path)/framework/delibs/decpp/deSemaphore.cpp',
            '<(deqp_path)/framework/delibs/decpp/deSharedPtr.cpp',
            '<(deqp_path)/framework/delibs/decpp/deSocket.cpp',
            '<(deqp_path)/framework/delibs/decpp/deSTLUtil.cpp',
            '<(deqp_path)/framework/delibs/decpp/deStringUtil.cpp',
            '<(deqp_path)/framework/delibs/decpp/deThread.cpp',
            '<(deqp_path)/framework/delibs/decpp/deThreadLocal.cpp',
            '<(deqp_path)/framework/delibs/decpp/deThreadSafeRingBuffer.cpp',
            '<(deqp_path)/framework/delibs/decpp/deUniquePtr.cpp',
        ],
        'deqp_libtester_sources':
        [
            '<(deqp_path)/execserver/xsDefs.cpp',
            '<(deqp_path)/execserver/xsExecutionServer.cpp',
            '<(deqp_path)/execserver/xsPosixFileReader.cpp',
            '<(deqp_path)/execserver/xsPosixTestProcess.cpp',
            '<(deqp_path)/execserver/xsProtocol.cpp',
            '<(deqp_path)/execserver/xsTcpServer.cpp',
            '<(deqp_path)/execserver/xsTestDriver.cpp',
            '<(deqp_path)/execserver/xsTestProcess.cpp',
            '<(deqp_path)/executor/xeBatchExecutor.cpp',
            '<(deqp_path)/executor/xeBatchResult.cpp',
            '<(deqp_path)/executor/xeCallQueue.cpp',
            '<(deqp_path)/executor/xeCommLink.cpp',
            '<(deqp_path)/executor/xeContainerFormatParser.cpp',
            '<(deqp_path)/executor/xeDefs.cpp',
            '<(deqp_path)/executor/xeLocalTcpIpLink.cpp',
            '<(deqp_path)/executor/xeTcpIpLink.cpp',
            '<(deqp_path)/executor/xeTestCase.cpp',
            '<(deqp_path)/executor/xeTestCaseListParser.cpp',
            '<(deqp_path)/executor/xeTestCaseResult.cpp',
            '<(deqp_path)/executor/xeTestLogParser.cpp',
            '<(deqp_path)/executor/xeTestLogWriter.cpp',
            '<(deqp_path)/executor/xeTestResultParser.cpp',
            '<(deqp_path)/executor/xeXMLParser.cpp',
            '<(deqp_path)/executor/xeXMLWriter.cpp',
            '<(deqp_path)/framework/common/tcuApp.cpp',
            '<(deqp_path)/framework/common/tcuBilinearImageCompare.cpp',
            '<(deqp_path)/framework/common/tcuCommandLine.cpp',
            '<(deqp_path)/framework/common/tcuCompressedTexture.cpp',
            '<(deqp_path)/framework/common/tcuCPUWarmup.cpp',
            '<(deqp_path)/framework/common/tcuDefs.cpp',
            '<(deqp_path)/framework/common/tcuEither.cpp',
            '<(deqp_path)/framework/common/tcuFactoryRegistry.cpp',
            '<(deqp_path)/framework/common/tcuFloatFormat.cpp',
            '<(deqp_path)/framework/common/tcuFunctionLibrary.cpp',
            '<(deqp_path)/framework/common/tcuFuzzyImageCompare.cpp',
            '<(deqp_path)/framework/common/tcuImageCompare.cpp',
            '<(deqp_path)/framework/common/tcuImageIO.cpp',
            '<(deqp_path)/framework/common/tcuInterval.cpp',
            '<(deqp_path)/framework/common/tcuPlatform.cpp',
            '<(deqp_path)/framework/common/tcuRandomValueIterator.cpp',
            '<(deqp_path)/framework/common/tcuRenderTarget.cpp',
            '<(deqp_path)/framework/common/tcuResource.cpp',
            '<(deqp_path)/framework/common/tcuResultCollector.cpp',
            '<(deqp_path)/framework/common/tcuRGBA.cpp',
            '<(deqp_path)/framework/common/tcuStringTemplate.cpp',
            '<(deqp_path)/framework/common/tcuSurface.cpp',
            '<(deqp_path)/framework/common/tcuTestCase.cpp',
            '<(deqp_path)/framework/common/tcuTestContext.cpp',
            '<(deqp_path)/framework/common/tcuTestHierarchyIterator.cpp',
            '<(deqp_path)/framework/common/tcuTestHierarchyUtil.cpp',
            '<(deqp_path)/framework/common/tcuTestLog.cpp',
            '<(deqp_path)/framework/common/tcuTestPackage.cpp',
            '<(deqp_path)/framework/common/tcuTestSessionExecutor.cpp',
            '<(deqp_path)/framework/common/tcuTexCompareVerifier.cpp',
            '<(deqp_path)/framework/common/tcuTexLookupVerifier.cpp',
            '<(deqp_path)/framework/common/tcuTexture.cpp',
            '<(deqp_path)/framework/common/tcuTextureUtil.cpp',
            '<(deqp_path)/framework/common/tcuTexVerifierUtil.cpp',
            '<(deqp_path)/framework/common/tcuThreadUtil.cpp',
            '<(deqp_path)/framework/common/tcuSeedBuilder.cpp',
            '<(deqp_path)/framework/delibs/debase/deDefs.c',
            '<(deqp_path)/framework/delibs/debase/deFloat16.c',
            '<(deqp_path)/framework/delibs/debase/deInt32.c',
            '<(deqp_path)/framework/delibs/debase/deInt32Test.c',
            '<(deqp_path)/framework/delibs/debase/deMath.c',
            '<(deqp_path)/framework/delibs/debase/deMemory.c',
            '<(deqp_path)/framework/delibs/debase/deRandom.c',
            '<(deqp_path)/framework/delibs/debase/deString.c',
            '<(deqp_path)/framework/delibs/deimage/deImage.c',
            '<(deqp_path)/framework/delibs/deimage/deTarga.c',
            '<(deqp_path)/framework/delibs/depool/deMemPool.c',
            '<(deqp_path)/framework/delibs/depool/dePoolArray.c',
            '<(deqp_path)/framework/delibs/depool/dePoolHashArray.c',
            '<(deqp_path)/framework/delibs/depool/dePoolHash.c',
            '<(deqp_path)/framework/delibs/depool/dePoolHashSet.c',
            '<(deqp_path)/framework/delibs/depool/dePoolHeap.c',
            '<(deqp_path)/framework/delibs/depool/dePoolMultiSet.c',
            '<(deqp_path)/framework/delibs/depool/dePoolSet.c',
            '<(deqp_path)/framework/delibs/depool/dePoolStringBuilder.c',
            '<(deqp_path)/framework/delibs/depool/dePoolTest.c',
            '<(deqp_path)/framework/delibs/destream/deFileStream.c',
            '<(deqp_path)/framework/delibs/destream/deRingbuffer.c',
            '<(deqp_path)/framework/delibs/destream/deStreamCpyThread.c',
            '<(deqp_path)/framework/delibs/destream/deThreadStream.c',
            '<(deqp_path)/framework/delibs/dethread/deAtomic.c',
            '<(deqp_path)/framework/delibs/dethread/deSingleton.c',
            '<(deqp_path)/framework/delibs/dethread/deThreadTest.c',
            '<(deqp_path)/framework/delibs/deutil/deClock.c',
            '<(deqp_path)/framework/delibs/deutil/deCommandLine.c',
            '<(deqp_path)/framework/delibs/deutil/deDynamicLibrary.c',
            '<(deqp_path)/framework/delibs/deutil/deFile.c',
            '<(deqp_path)/framework/delibs/deutil/deProcess.c',
            '<(deqp_path)/framework/delibs/deutil/deSocket.c',
            '<(deqp_path)/framework/delibs/deutil/deTimer.c',
            '<(deqp_path)/framework/delibs/deutil/deTimerTest.c',
            '<(deqp_path)/framework/egl/egluCallLogWrapper.cpp',
            '<(deqp_path)/framework/egl/egluConfigFilter.cpp',
            '<(deqp_path)/framework/egl/egluConfigInfo.cpp',
            '<(deqp_path)/framework/egl/egluDefs.cpp',
            '<(deqp_path)/framework/egl/egluGLContextFactory.cpp',
            '<(deqp_path)/framework/egl/egluGLFunctionLoader.cpp',
            '<(deqp_path)/framework/egl/egluGLFunctionLoader.cpp',
            '<(deqp_path)/framework/egl/egluGLUtil.cpp',
            '<(deqp_path)/framework/egl/egluNativeDisplay.cpp',
            '<(deqp_path)/framework/egl/egluNativePixmap.cpp',
            '<(deqp_path)/framework/egl/egluNativeWindow.cpp',
            '<(deqp_path)/framework/egl/egluPlatform.cpp',
            '<(deqp_path)/framework/egl/egluStaticESLibrary.cpp',
            '<(deqp_path)/framework/egl/egluStrUtil.cpp',
            '<(deqp_path)/framework/egl/egluUnique.cpp',
            '<(deqp_path)/framework/egl/egluUtil.cpp',
            '<(deqp_path)/framework/egl/wrapper/eglwDefs.cpp',
            '<(deqp_path)/framework/egl/wrapper/eglwFunctions.cpp',
            '<(deqp_path)/framework/egl/wrapper/eglwLibrary.cpp',
            '<(deqp_path)/framework/opengl/gluCallLogWrapper.cpp',
            '<(deqp_path)/framework/opengl/gluContextFactory.cpp',
            '<(deqp_path)/framework/opengl/gluContextInfo.cpp',
            '<(deqp_path)/framework/opengl/gluDefs.cpp',
            '<(deqp_path)/framework/opengl/gluDrawUtil.cpp',
            '<(deqp_path)/framework/opengl/gluDummyRenderContext.cpp',
            '<(deqp_path)/framework/opengl/gluES3PlusWrapperContext.cpp',
            '<(deqp_path)/framework/opengl/gluFboRenderContext.cpp',
            '<(deqp_path)/framework/opengl/gluObjectWrapper.cpp',
            '<(deqp_path)/framework/opengl/gluPixelTransfer.cpp',
            '<(deqp_path)/framework/opengl/gluPlatform.cpp',
            '<(deqp_path)/framework/opengl/gluProgramInterfaceQuery.cpp',
            '<(deqp_path)/framework/opengl/gluRenderConfig.cpp',
            '<(deqp_path)/framework/opengl/gluRenderContext.cpp',
            '<(deqp_path)/framework/opengl/gluShaderProgram.cpp',
            '<(deqp_path)/framework/opengl/gluShaderUtil.cpp',
            '<(deqp_path)/framework/opengl/gluStateReset.cpp',
            '<(deqp_path)/framework/opengl/gluStrUtil.cpp',
            '<(deqp_path)/framework/opengl/gluTexture.cpp',
            '<(deqp_path)/framework/opengl/gluTextureUtil.cpp',
            '<(deqp_path)/framework/opengl/gluVarType.cpp',
            '<(deqp_path)/framework/opengl/gluVarTypeUtil.cpp',
            '<(deqp_path)/framework/opengl/simplereference/sglrContext.cpp',
            '<(deqp_path)/framework/opengl/simplereference/sglrContextUtil.cpp',
            '<(deqp_path)/framework/opengl/simplereference/sglrContextWrapper.cpp',
            '<(deqp_path)/framework/opengl/simplereference/sglrGLContext.cpp',
            '<(deqp_path)/framework/opengl/simplereference/sglrReferenceContext.cpp',
            '<(deqp_path)/framework/opengl/simplereference/sglrReferenceUtils.cpp',
            '<(deqp_path)/framework/opengl/simplereference/sglrShaderProgram.cpp',
            '<(deqp_path)/framework/opengl/wrapper/glwDefs.cpp',
            '<(deqp_path)/framework/opengl/wrapper/glwFunctions.cpp',
            '<(deqp_path)/framework/opengl/wrapper/glwInitES20Direct.cpp',
            '<(deqp_path)/framework/opengl/wrapper/glwInitES30Direct.cpp',
            '<(deqp_path)/framework/opengl/wrapper/glwInitFunctions.cpp',
            '<(deqp_path)/framework/opengl/wrapper/glwWrapper.cpp',
            '<(deqp_path)/framework/platform/null/tcuNullContextFactory.cpp',
            '<(deqp_path)/framework/platform/null/tcuNullContextFactory.hpp',
            '<(deqp_path)/framework/platform/null/tcuNullRenderContext.cpp',
            '<(deqp_path)/framework/qphelper/qpCrashHandler.c',
            '<(deqp_path)/framework/qphelper/qpDebugOut.c',
            '<(deqp_path)/framework/qphelper/qpInfo.c',
            '<(deqp_path)/framework/qphelper/qpTestLog.c',
            '<(deqp_path)/framework/qphelper/qpWatchDog.c',
            '<(deqp_path)/framework/qphelper/qpXmlWriter.c',
            '<(deqp_path)/framework/randomshaders/rsgBinaryOps.cpp',
            '<(deqp_path)/framework/randomshaders/rsgBuiltinFunctions.cpp',
            '<(deqp_path)/framework/randomshaders/rsgDefs.cpp',
            '<(deqp_path)/framework/randomshaders/rsgExecutionContext.cpp',
            '<(deqp_path)/framework/randomshaders/rsgExpression.cpp',
            '<(deqp_path)/framework/randomshaders/rsgExpressionGenerator.cpp',
            '<(deqp_path)/framework/randomshaders/rsgFunctionGenerator.cpp',
            '<(deqp_path)/framework/randomshaders/rsgGeneratorState.cpp',
            '<(deqp_path)/framework/randomshaders/rsgNameAllocator.cpp',
            '<(deqp_path)/framework/randomshaders/rsgParameters.cpp',
            '<(deqp_path)/framework/randomshaders/rsgPrettyPrinter.cpp',
            '<(deqp_path)/framework/randomshaders/rsgProgramExecutor.cpp',
            '<(deqp_path)/framework/randomshaders/rsgProgramGenerator.cpp',
            '<(deqp_path)/framework/randomshaders/rsgSamplers.cpp',
            '<(deqp_path)/framework/randomshaders/rsgShader.cpp',
            '<(deqp_path)/framework/randomshaders/rsgShaderGenerator.cpp',
            '<(deqp_path)/framework/randomshaders/rsgStatement.cpp',
            '<(deqp_path)/framework/randomshaders/rsgToken.cpp',
            '<(deqp_path)/framework/randomshaders/rsgUtils.cpp',
            '<(deqp_path)/framework/randomshaders/rsgVariable.cpp',
            '<(deqp_path)/framework/randomshaders/rsgVariableManager.cpp',
            '<(deqp_path)/framework/randomshaders/rsgVariableType.cpp',
            '<(deqp_path)/framework/randomshaders/rsgVariableValue.cpp',
            '<(deqp_path)/framework/referencerenderer/rrDefs.cpp',
            '<(deqp_path)/framework/referencerenderer/rrFragmentOperations.cpp',
            '<(deqp_path)/framework/referencerenderer/rrMultisamplePixelBufferAccess.cpp',
            '<(deqp_path)/framework/referencerenderer/rrPrimitivePacket.cpp',
            '<(deqp_path)/framework/referencerenderer/rrRasterizer.cpp',
            '<(deqp_path)/framework/referencerenderer/rrRenderer.cpp',
            '<(deqp_path)/framework/referencerenderer/rrShaders.cpp',
            '<(deqp_path)/framework/referencerenderer/rrShadingContext.cpp',
            '<(deqp_path)/framework/referencerenderer/rrVertexAttrib.cpp',
            '<(deqp_path)/framework/referencerenderer/rrVertexPacket.cpp',
            '<(deqp_path)/modules/glshared/glsAttributeLocationTests.cpp',
            '<(deqp_path)/modules/glshared/glsBufferTestUtil.cpp',
            '<(deqp_path)/modules/glshared/glsBuiltinPrecisionTests.cpp',
            '<(deqp_path)/modules/glshared/glsCalibration.cpp',
            '<(deqp_path)/modules/glshared/glsDrawTest.cpp',
            '<(deqp_path)/modules/glshared/glsFboCompletenessTests.cpp',
            '<(deqp_path)/modules/glshared/glsFboUtil.cpp',
            '<(deqp_path)/modules/glshared/glsFragmentOpUtil.cpp',
            '<(deqp_path)/modules/glshared/glsFragOpInteractionCase.cpp',
            '<(deqp_path)/modules/glshared/glsInteractionTestUtil.cpp',
            '<(deqp_path)/modules/glshared/glsLifetimeTests.cpp',
            '<(deqp_path)/modules/glshared/glsLongStressCase.cpp',
            '<(deqp_path)/modules/glshared/glsLongStressTestUtil.cpp',
            '<(deqp_path)/modules/glshared/glsMemoryStressCase.cpp',
            '<(deqp_path)/modules/glshared/glsRandomShaderCase.cpp',
            '<(deqp_path)/modules/glshared/glsRandomShaderProgram.cpp',
            '<(deqp_path)/modules/glshared/glsRandomUniformBlockCase.cpp',
            '<(deqp_path)/modules/glshared/glsRasterizationTestUtil.cpp',
            '<(deqp_path)/modules/glshared/glsSamplerObjectTest.cpp',
            '<(deqp_path)/modules/glshared/glsScissorTests.cpp',
            '<(deqp_path)/modules/glshared/glsShaderConstExprTests.cpp',
            '<(deqp_path)/modules/glshared/glsShaderExecUtil.cpp',
            '<(deqp_path)/modules/glshared/glsShaderLibraryCase.cpp',
            '<(deqp_path)/modules/glshared/glsShaderLibrary.cpp',
            '<(deqp_path)/modules/glshared/glsShaderPerformanceCase.cpp',
            '<(deqp_path)/modules/glshared/glsShaderPerformanceMeasurer.cpp',
            '<(deqp_path)/modules/glshared/glsShaderRenderCase.cpp',
            '<(deqp_path)/modules/glshared/glsStateQueryUtil.cpp',
            '<(deqp_path)/modules/glshared/glsStateChangePerfTestCases.cpp',
            '<(deqp_path)/modules/glshared/glsTextureBufferCase.cpp',
            '<(deqp_path)/modules/glshared/glsTextureStateQueryTests.cpp',
            '<(deqp_path)/modules/glshared/glsTextureTestUtil.cpp',
            '<(deqp_path)/modules/glshared/glsUniformBlockCase.cpp',
            '<(deqp_path)/modules/glshared/glsVertexArrayTests.cpp',
            '<(angle_path)/src/tests/deqp_support/tcuANGLENativeDisplayFactory.cpp',
            '<(angle_path)/src/tests/deqp_support/tcuANGLENativeDisplayFactory.h',
            # TODO(jmadill): integrate with dEQP
            '<(angle_path)/src/tests/deqp_support/tcuRandomOrderExecutor.cpp',
            '<(angle_path)/src/tests/deqp_support/tcuRandomOrderExecutor.h',
        ],
    },
    'conditions':
    [
        ['angle_build_deqp_libraries==1 and angle_standalone==1',
        {
            'targets':
            [
                {
                    'target_name': 'angle_zlib',
                    'type': 'static_library',
                    'includes': [ '../../build/common_defines.gypi', ],
                    'include_dirs':
                    [
                        '<(zlib_path)',
                    ],
                    'direct_dependent_settings':
                    {
                        'include_dirs':
                        [
                            '<(zlib_path)',
                        ],
                    },
                    'msvs_settings':
                    {
                        'VCCLCompilerTool':
                        {
                            'AdditionalOptions':
                            [
                                '/wd4131', # old-style declarator
                                '/wd4244', # Conversion from 'type1' to 'type2', possible loss of data
                                '/wd4324', # structure was padded
                                '/wd4701', # potentially uninit used
                                '/wd4996', # deprecated
                            ],
                        },
                    },
                    'conditions':
                    [
                        ['angle_build_winrt==1',
                        {
                            # In zlib, deflate.c/insert_string_sse assumes _MSC_VER is only used for x86 or x64
                            # To compile this function for ARM using MSC, we trick it by defining __clang__
                            # __clang__ isn't used elsewhere zlib, so this workaround shouldn't impact anything else
                            'defines':
                            [
                                '__clang__',
                            ],
                        }],
                    ],
                    'sources':
                    [
                        '<(zlib_path)/adler32.c',
                        '<(zlib_path)/compress.c',
                        '<(zlib_path)/crc32.c',
                        '<(zlib_path)/crc32.h',
                        '<(zlib_path)/deflate.c',
                        '<(zlib_path)/deflate.h',
                        '<(zlib_path)/gzclose.c',
                        '<(zlib_path)/gzguts.h',
                        '<(zlib_path)/gzlib.c',
                        '<(zlib_path)/gzread.c',
                        '<(zlib_path)/gzwrite.c',
                        '<(zlib_path)/infback.c',
                        '<(zlib_path)/inffast.c',
                        '<(zlib_path)/inffast.h',
                        '<(zlib_path)/inffixed.h',
                        '<(zlib_path)/inflate.c',
                        '<(zlib_path)/inflate.h',
                        '<(zlib_path)/inftrees.c',
                        '<(zlib_path)/inftrees.h',
                        '<(zlib_path)/mozzconf.h',
                        '<(zlib_path)/trees.c',
                        '<(zlib_path)/trees.h',
                        '<(zlib_path)/uncompr.c',
                        '<(zlib_path)/x86.h',
                        '<(zlib_path)/zconf.h',
                        '<(zlib_path)/zlib.h',
                        '<(zlib_path)/zutil.c',
                        '<(zlib_path)/zutil.h',
                        '<(zlib_path)/simd_stub.c',
                    ],
                },

                {
                    'target_name': 'angle_libpng',
                    'type': 'static_library',
                    'includes': [ '../../build/common_defines.gypi', ],
                    'dependencies':
                    [
                        'angle_zlib'
                    ],
                    'msvs_settings':
                    {
                        'VCCLCompilerTool':
                        {
                            'AdditionalOptions':
                            [
                                '/wd4018', # signed/unsigned mismatch
                                '/wd4028', # parameter differs from decl
                                '/wd4101', # unreferenced local
                                '/wd4189', # unreferenced but initted
                                '/wd4244', # Conversion from 'type1' to 'type2', possible loss of data
                                '/wd4267', # Conversion from 'size_t' to 'type', possible loss of data
                            ],
                        },
                    },
                    'sources':
                    [
                        '<(libpng_path)/png.c',
                        '<(libpng_path)/pngerror.c',
                        '<(libpng_path)/pngget.c',
                        '<(libpng_path)/pngmem.c',
                        '<(libpng_path)/pngpread.c',
                        '<(libpng_path)/pngread.c',
                        '<(libpng_path)/pngrio.c',
                        '<(libpng_path)/pngrtran.c',
                        '<(libpng_path)/pngrutil.c',
                        '<(libpng_path)/pngset.c',
                        '<(libpng_path)/pngtrans.c',
                        '<(libpng_path)/pngwio.c',
                        '<(libpng_path)/pngwrite.c',
                        '<(libpng_path)/pngwtran.c',
                        '<(libpng_path)/pngwutil.c',
                    ],
                },
            ], # targets
        }], # angle_build_deqp_libraries==1 and angle_standalone==1
        ['angle_build_deqp_libraries==1',
        {
            'targets':
            [
                {
                    'target_name': 'angle_deqp_support',
                    'type': 'none',
                    'direct_dependent_settings':
                    {
                        'configurations':
                        {
                            'Common_Base':
                            {
                                'msvs_configuration_attributes':
                                {
                                    # dEQP requires ASCII
                                    'CharacterSet': '0',
                                },
                                'msvs_settings':
                                {
                                    'VCCLCompilerTool':
                                    {
                                        # dEQP requires exceptions and RTTI
                                        'ExceptionHandling': 1,
                                        'RuntimeTypeInfo': 'true',
                                    },
                                    'VCLinkerTool':
                                    {
                                        'conditions':
                                        [
                                            ['angle_build_winrt==0',
                                            {
                                                'AdditionalDependencies':
                                                [
                                                    'dbghelp.lib',
                                                    'gdi32.lib',
                                                    'user32.lib',
                                                    'ws2_32.lib',
                                                ],
                                            }],
                                            ['angle_build_winrt==1',
                                            {
                                                # Disable COMDAT optimizations, disabled by default for non-WinRT
                                                'AdditionalOptions': ['/OPT:NOREF', '/OPT:NOICF'],
                                                # AdditionalDependencies automatically configures the required .libs
                                                'AdditionalDependencies':
                                                [
                                                    '%(AdditionalDependencies)'
                                                ],
                                            }],
                                        ],
                                    },
                                },
                            },
                        },
                        # Re-enable RTTI and exceptions, dEQP needs them.
                        'cflags_cc!':
                        [
                            '-fno-exceptions',
                            '-fno-rtti',
                        ],
                        'msvs_disabled_warnings':
                        [
                            '<@(deqp_msvs_disabled_warnings)',
                        ],
                        'include_dirs': ['<@(deqp_include_dirs)'],
                        'defines': ['<@(deqp_defines)'],
                        'defines!': [ '<@(deqp_undefines)' ],
                    },
                    'conditions':
                    [
                        ['angle_build_winrt==1',
                        {
                            'type' : 'shared_library',
                        }],
                    ],
                },

                # Compile decpp separately because MSVC ignores the extension of the files when
                # outputting the obj file, and later thinks that a decpp obj and another obj are the
                # same, ignoring one and eventually producing a link error. The problem occurs for
                # example between decpp/deRandom.cpp and debase/deRandom.c
                {
                    'target_name': 'angle_deqp_decpp',
                    'type': 'static_library',
                    'dependencies': [ 'angle_deqp_support' ],
                    'sources':
                    [
                        '<@(deqp_libtester_decpp_sources)',
                    ],
                    # In a chromium build dl is required for deDynamicLibrary
                    'conditions':
                    [
                        ['OS=="linux"',
                        {
                            'link_settings':
                            {
                                'libraries': ['-ldl']
                            },
                        }],
                    ],
                },

                {
                    'target_name': 'angle_deqp_libtester',
                    'type': 'static_library',
                    'dependencies':
                    [
                        'angle_deqp_decpp',
                        'angle_deqp_support',
                        '<(angle_path)/src/angle.gyp:libEGL',
                        '<(angle_path)/util/util.gyp:angle_util',
                    ],
                    'export_dependent_settings':
                    [
                        'angle_deqp_support',
                        '<(angle_path)/util/util.gyp:angle_util',
                    ],
                    'include_dirs':
                    [
                        '<(angle_path)/include',
                    ],
                    'direct_dependent_settings':
                    {
                        'include_dirs':
                        [
                            '<(angle_path)/include',
                        ],
                        'defines':
                        [
                            'ANGLE_DEQP_LIBTESTER_IMPLEMENTATION',
                        ],
                    },
                    'msvs_settings':
                    {
                        'VCCLCompilerTool':
                        {
                            'AdditionalOptions':
                            [
                                '/bigobj', # needed for glsBuiltinPrecisionTests.cpp
                            ],
                        },
                    },
                    'sources':
                    [
                        '<@(deqp_libtester_sources)',
                    ],
                    'conditions':
                    [
                        ['angle_standalone==1',
                        {
                            'dependencies': [ 'angle_libpng' ],
                        },
                        { # angle_standalone!=1
                            'dependencies': [ '<(DEPTH)/third_party/libpng/libpng.gyp:libpng' ],
                        }],
                        ['OS=="mac"',
                        {
                            'direct_dependent_settings':
                            {
                                'xcode_settings':
                                {
                                    'DYLIB_INSTALL_NAME_BASE': '@rpath',
                                },
                            },
                        }],
                    ],
                },

                {
                    'target_name': 'angle_deqp_libgles2',
                    'type': 'shared_library',
                    'dependencies':
                    [
                        'angle_deqp_libtester',
                    ],
                    'defines':
                    [
                        'ANGLE_DEQP_GLES2_TESTS',
                    ],
                    'direct_dependent_settings':
                    {
                        'defines':
                        [
                            'ANGLE_DEQP_GLES2_TESTS',
                        ],
                    },
                    'sources':
                    [
                        '<@(deqp_gles2_sources)',
                        'deqp_support/angle_deqp_libtester_main.cpp',
                        'deqp_support/tcuANGLEPlatform.cpp',
                        'deqp_support/tcuANGLEPlatform.h',
                    ],
                },

                {
                    'target_name': 'angle_deqp_libgles3',
                    'type': 'shared_library',
                    'dependencies':
                    [
                        'angle_deqp_libtester',
                    ],
                    'defines':
                    [
                        'ANGLE_DEQP_GLES3_TESTS',
                    ],
                    'direct_dependent_settings':
                    {
                        'defines':
                        [
                            'ANGLE_DEQP_GLES3_TESTS',
                        ],
                    },
                    'sources':
                    [
                        '<@(deqp_gles3_sources)',
                        'deqp_support/angle_deqp_libtester_main.cpp',
                        'deqp_support/tcuANGLEPlatform.cpp',
                        'deqp_support/tcuANGLEPlatform.h',
                    ],
                },

                {
                    'target_name': 'angle_deqp_libegl',
                    'type': 'shared_library',
                    'dependencies':
                    [
                        'angle_deqp_libtester',
                    ],
                    'defines':
                    [
                        'ANGLE_DEQP_EGL_TESTS',
                    ],
                    'direct_dependent_settings':
                    {
                        'defines':
                        [
                            'ANGLE_DEQP_EGL_TESTS',
                        ],
                    },
                    'sources':
                    [
                        '<@(deqp_egl_sources)',
                        'deqp_support/angle_deqp_libtester_main.cpp',
                        'deqp_support/tcuANGLEPlatform.cpp',
                        'deqp_support/tcuANGLEPlatform.h',
                    ],
                },
            ], # targets
        }], # angle_build_deqp_libraries
        ['angle_build_deqp_executables==1',
        {
            "targets":
            [
                {
                    'target_name': 'angle_deqp_gles2_tests',
                    'type': 'executable',
                    'dependencies':
                    [
                        'angle_deqp_libgles2',
                    ],
                    'sources':
                    [
                        'deqp_support/angle_deqp_tests_main.cpp',
                    ],
                },

                {
                    'target_name': 'angle_deqp_gles3_tests',
                    'type': 'executable',
                    'dependencies':
                    [
                        'angle_deqp_libgles3',
                    ],
                    'sources':
                    [
                        'deqp_support/angle_deqp_tests_main.cpp',
                    ],
                },

                {
                    'target_name': 'angle_deqp_egl_tests',
                    'type': 'executable',
                    'dependencies':
                    [
                        'angle_deqp_libegl',
                    ],
                    'sources':
                    [
                        'deqp_support/angle_deqp_tests_main.cpp',
                    ],
                },
            ], # targets
        }], # angle_build_deqp_executables
        ['angle_build_deqp_gtest_support==1',
        {
            'targets':
            [
                # Helper target for synching our implementation with chrome's
                {
                    'target_name': 'angle_deqp_gtest_support',
                    'type': 'none',
                    'dependencies':
                    [
                        'angle_test_support',
                        '<(angle_path)/util/util.gyp:angle_util',
                    ],
                    'export_dependent_settings':
                    [
                        'angle_test_support',
                        '<(angle_path)/util/util.gyp:angle_util',
                    ],

                    'copies':
                    [
                        {
                            'destination': '<(PRODUCT_DIR)/deqp_support',
                            'files':
                            [
                                'deqp_support/dEQP-EGL-cases.txt.gz',
                                'deqp_support/dEQP-GLES2-cases.txt.gz',
                                'deqp_support/dEQP-GLES3-cases.txt.gz',
                                'deqp_support/deqp_egl_test_expectations.txt',
                                'deqp_support/deqp_gles2_test_expectations.txt',
                                'deqp_support/deqp_gles3_test_expectations.txt',
                            ],
                        },
                    ],

                    'direct_dependent_settings':
                    {
                        'include_dirs':
                        [
                            'deqp_support',
                            'third_party/gpu_test_expectations',
                        ],
                        'sources':
                        [
                            'deqp_support/angle_deqp_gtest.cpp',
                            'third_party/gpu_test_expectations/gpu_info.cc',
                            'third_party/gpu_test_expectations/gpu_info.h',
                            'third_party/gpu_test_expectations/gpu_test_config.cc',
                            'third_party/gpu_test_expectations/gpu_test_config.h',
                            'third_party/gpu_test_expectations/gpu_test_expectations_parser.cc',
                            'third_party/gpu_test_expectations/gpu_test_expectations_parser.h',
                        ],

                        'defines':
                        [
                            # Re-define the missing Windows macros
                            '<@(deqp_undefines)',
                        ],

                        'msvs_settings':
                        {
                            'VCLinkerTool':
                            {
                                'AdditionalDependencies':
                                [
                                    'user32.lib',
                                ],
                            },
                        },

                        'conditions':
                        [
                            ['OS=="linux"',
                            {
                                'ldflags':
                                [
                                    '<!@(pkg-config --libs-only-L --libs-only-other libpci)',
                                ],
                                'libraries':
                                [
                                    '<!@(pkg-config --libs-only-l libpci)',
                                ],
                            }],
                            ['OS=="mac"',
                            {
                                'sources':
                                [
                                    'third_party/gpu_test_expectations/gpu_test_config_mac.mm',
                                ],
                                'link_settings':
                                {
                                    'libraries':
                                    [
                                        '$(SDKROOT)/System/Library/Frameworks/IOKit.framework',
                                    ],
                                },
                            }],
                        ],
                    },

                    'conditions':
                    [
                        ['angle_standalone==1',
                        {
                            'dependencies': [ 'angle_zlib' ],
                            'export_dependent_settings': [ 'angle_zlib' ],
                        },
                        { # angle_standalone!=1
                            'dependencies': [ '<(zlib_path)/zlib.gyp:zlib' ],
                            'export_dependent_settings': [ '<(zlib_path)/zlib.gyp:zlib' ],
                        }],
                    ],
                },
            ], # targets
        }], # angle_build_deqp_gtest_support
        ['angle_build_deqp_gtest_executables==1',
        {
            "targets":
            [
                {
                    'target_name': 'angle_deqp_gtest_gles2_tests',
                    'type': 'executable',
                    'includes': [ '../../build/common_defines.gypi', ],
                    'dependencies':
                    [
                        'angle_deqp_gtest_support',
                        'angle_deqp_libgles2',
                    ],
                    'sources':
                    [
                        'deqp_support/angle_deqp_gtest_main.cpp',
                    ],
                },

                {
                    'target_name': 'angle_deqp_gtest_gles3_tests',
                    'type': 'executable',
                    'includes': [ '../../build/common_defines.gypi', ],
                    'dependencies':
                    [
                        'angle_deqp_gtest_support',
                        'angle_deqp_libgles3',
                    ],
                    'sources':
                    [
                        'deqp_support/angle_deqp_gtest_main.cpp',
                    ],
                },

                {
                    'target_name': 'angle_deqp_gtest_egl_tests',
                    'type': 'executable',
                    'includes': [ '../../build/common_defines.gypi', ],
                    'dependencies':
                    [
                        'angle_deqp_gtest_support',
                        'angle_deqp_libegl',
                    ],
                    'sources':
                    [
                        'deqp_support/angle_deqp_gtest_main.cpp',
                    ],
                },
            ], # targets
        }], # angle_build_deqp_gtest_executables
    ], # conditions
}
