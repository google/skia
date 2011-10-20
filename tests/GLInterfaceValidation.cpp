
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkNativeGLContext.h"
#include "SkMesaGLContext.h"

static void GLInterfaceValidationTest(skiatest::Reporter* reporter) {
    typedef const GrGLInterface* (*interfaceFactory)();
    struct {
       interfaceFactory fFactory;
       const char* fName;
    } interfaceFactories[] = {
        {GrGLCreateNativeInterface, "Native"},
#if SK_MESA
        {GrGLCreateMesaInterface, "Mesa"},
#endif
    };

    // On some platforms GrGLCreateNativeInterface will fail unless an OpenGL
    // context has been created. Also, preserve the current context that may
    // be in use by outer test harness.
    SkNativeGLContext::AutoContextRestore nglacr;
    SkNativeGLContext nglctx;
    static const int gBOGUS_SIZE = 16;
    REPORTER_ASSERT(reporter, nglctx.init(gBOGUS_SIZE, gBOGUS_SIZE));
#if SK_MESA
    // We must have a current OSMesa context to initialize an OSMesa 
    // GrGLInterface
    SkMesaGLContext::AutoContextRestore mglacr;
    SkMesaGLContext mglctx;
    REPORTER_ASSERT(reporter, mglctx.init(gBOGUS_SIZE, gBOGUS_SIZE));
#endif

    SkAutoTUnref<const GrGLInterface> iface;
    for (size_t i = 0; i < SK_ARRAY_COUNT(interfaceFactories); ++i) {
        iface.reset(interfaceFactories[i].fFactory());
        REPORTER_ASSERT(reporter, NULL != iface.get());
        if (iface.get()) {
            REPORTER_ASSERT(reporter, iface.get()->validate(kOpenGL_Shaders_GrEngine));
        }
    }
}


#include "TestClassDef.h"
DEFINE_TESTCLASS("GLInterfaceValidation",
                 GLInterfaceValidationTestClass,
                 GLInterfaceValidationTest)

