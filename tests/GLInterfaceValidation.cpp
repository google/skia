
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#if SK_ANGLE
#include "gl/SkANGLEGLContext.h"
#endif
#include "gl/SkNativeGLContext.h"
#if SK_MESA
#include "gl/SkMesaGLContext.h"
#endif

static void GLInterfaceValidationTest(skiatest::Reporter* reporter) {
    typedef const GrGLInterface* (*interfaceFactory)();
    struct {
       interfaceFactory fFactory;
       const char* fName;
    } interfaceFactories[] = {
#if SK_ANGLE
        {GrGLCreateANGLEInterface, "ANGLE"},
#endif
        {GrGLCreateNativeInterface, "Native"},
#if SK_MESA
        {GrGLCreateMesaInterface, "Mesa"},
#endif
        {GrGLCreateDebugInterface, "Debug"},
        {GrGLCreateNullInterface, "Null"},
    };

    // On some platforms GrGLCreateNativeInterface will fail unless an OpenGL
    // context has been created. Also, preserve the current context that may
    // be in use by outer test harness.
    SkNativeGLContext::AutoContextRestore nglacr;
    SkNativeGLContext nglctx;
    static const int gBOGUS_SIZE = 16;
    bool nativeContextInit = nglctx.init(gBOGUS_SIZE, gBOGUS_SIZE);
    REPORTER_ASSERT(reporter, nativeContextInit);
    if (!nativeContextInit) {
        return;
    }
#if SK_MESA
    // We must have a current OSMesa context to initialize an OSMesa
    // GrGLInterface
    SkMesaGLContext::AutoContextRestore mglacr;
    SkMesaGLContext mglctx;
    bool mesaContextInit = mglctx.init(gBOGUS_SIZE, gBOGUS_SIZE);
    REPORTER_ASSERT(reporter, mesaContextInit);
    if(!mesaContextInit) {
        return;
    }
#endif

    SkAutoTUnref<const GrGLInterface> iface;
    for (size_t i = 0; i < SK_ARRAY_COUNT(interfaceFactories); ++i) {
        iface.reset(interfaceFactories[i].fFactory());
        REPORTER_ASSERT(reporter, NULL != iface.get());
        if (iface.get()) {
            for (GrGLBinding binding = kFirstGrGLBinding;
                 binding <= kLastGrGLBinding;
                 binding = static_cast<GrGLBinding>(binding << 1)) {
                if (iface.get()->fBindingsExported & binding) {
                    REPORTER_ASSERT(reporter, iface.get()->validate(binding));
                }
            }
        }
    }
}


#include "TestClassDef.h"
DEFINE_TESTCLASS("GLInterfaceValidation",
                 GLInterfaceValidationTestClass,
                 GLInterfaceValidationTest)

