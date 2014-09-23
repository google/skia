/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessorUnitTest_DEFINED
#define GrProcessorUnitTest_DEFINED

#include "SkRandom.h"
#include "SkTArray.h"
#include "SkTypes.h"

class SkMatrix;
class GrDrawTargetCaps;

namespace GrProcessorUnitTest {
// Used to access the dummy textures in TestCreate procs.
enum {
    kSkiaPMTextureIdx = 0,
    kAlphaTextureIdx = 1,
};

/**
 * A helper for use in GrProcessor::TestCreate functions.
 */
const SkMatrix& TestMatrix(SkRandom*);

}

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

class GrContext;
class GrProcessor;
class GrTexture;

template <class Processor>
class GrProcessorTestFactory : SkNoncopyable {
public:

    typedef Processor* (*CreateProc)(SkRandom*,
                                    GrContext*,
                                    const GrDrawTargetCaps& caps,
                                    GrTexture* dummyTextures[]);

    GrProcessorTestFactory(CreateProc createProc) {
        fCreateProc = createProc;
        GetFactories()->push_back(this);
    }

    static Processor* CreateStage(SkRandom* random,
                                 GrContext* context,
                                 const GrDrawTargetCaps& caps,
                                 GrTexture* dummyTextures[]) {
        uint32_t idx = random->nextRangeU(0, GetFactories()->count() - 1);
        GrProcessorTestFactory<Processor>* factory = (*GetFactories())[idx];
        return factory->fCreateProc(random, context, caps, dummyTextures);
    }

private:
    CreateProc fCreateProc;

    #if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
    static SkTArray<GrProcessorTestFactory<Processor>*, true>* GetFactories() {
        static SkTArray<GrProcessorTestFactory<Processor>*, true> gFactories;
        return &gFactories;
    }
    #endif
};

/** GrProcessor subclasses should insert this macro in their declaration to be included in the
 *  program generation unit test.
 */

#define GR_DECLARE_GEOMETRY_PROCESSOR_TEST                                                         \
    static GrProcessorTestFactory<GrGeometryProcessor> gTestFactory;                               \
    static GrGeometryProcessor* TestCreate(SkRandom*,                                              \
                                GrContext*,                                                        \
                                const GrDrawTargetCaps&,                                           \
                                GrTexture* dummyTextures[2])

#define GR_DECLARE_FRAGMENT_PROCESSOR_TEST                                                         \
    static GrProcessorTestFactory<GrFragmentProcessor> gTestFactory;                               \
    static GrFragmentProcessor* TestCreate(SkRandom*,                                              \
                                GrContext*,                                                        \
                                const GrDrawTargetCaps&,                                           \
                                GrTexture* dummyTextures[2])

/** GrProcessor subclasses should insert this macro in their implementation file. They must then
 *  also implement this static function:
 *      GrProcessor* TestCreate(SkRandom*,
 *                           GrContext*,
 *                           const GrDrawTargetCaps&,
 *                           GrTexture* dummyTextures[2]);
 * dummyTextures[] are valid textures that can optionally be used to construct GrTextureAccesses.
 * The first texture has config kSkia8888_GrPixelConfig and the second has
 * kAlpha_8_GrPixelConfig. TestCreate functions are also free to create additional textures using
 * the GrContext.
 */
#define GR_DEFINE_FRAGMENT_PROCESSOR_TEST(Effect)                                                  \
    GrProcessorTestFactory<GrFragmentProcessor> Effect :: gTestFactory(Effect :: TestCreate)

#define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(Effect)                                                  \
    GrProcessorTestFactory<GrGeometryProcessor> Effect :: gTestFactory(Effect :: TestCreate)

#else // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_FRAGMENT_PROCESSOR_TEST                                                         \
    static GrFragmentProcessor* TestCreate(SkRandom*,                                              \
                                GrContext*,                                                        \
                                const GrDrawTargetCaps&,                                           \
                                GrTexture* dummyTextures[2])
#define GR_DEFINE_FRAGMENT_PROCESSOR_TEST(X)

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_GEOMETRY_PROCESSOR_TEST                                                         \
    static GrGeometryProcessor* TestCreate(SkRandom*,                                              \
                                GrContext*,                                                        \
                                const GrDrawTargetCaps&,                                           \
                                GrTexture* dummyTextures[2])
#define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(X)

#endif // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
#endif
