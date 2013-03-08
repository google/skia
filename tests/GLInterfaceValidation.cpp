
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#include "Test.h"
// This is a GPU-backend specific test
#if SK_SUPPORT_GPU

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

    static const int kBogusSize = 16;

#if SK_ANGLE
    SkANGLEGLContext::AutoContextRestore angleACR;
    SkANGLEGLContext angleContext;
    bool angleContextInit = angleContext.init(kBogusSize, kBogusSize);
    REPORTER_ASSERT(reporter, angleContextInit);
    if (!angleContextInit) {
        return;
    }
#endif

    // On some platforms GrGLCreateNativeInterface will fail unless an OpenGL
    // context has been created. Also, preserve the current context that may
    // be in use by outer test harness.
    SkNativeGLContext::AutoContextRestore nglACR;
    SkNativeGLContext nglctx;
    bool nativeContextInit = nglctx.init(kBogusSize, kBogusSize);
    REPORTER_ASSERT(reporter, nativeContextInit);
    if (!nativeContextInit) {
        return;
    }

#if SK_MESA
    // We must have a current OSMesa context to initialize an OSMesa
    // GrGLInterface
    SkMesaGLContext::AutoContextRestore mglACR;
    SkMesaGLContext mglctx;
    bool mesaContextInit = mglctx.init(kBogusSize, kBogusSize);
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

#endif
