/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessorUnitTest_DEFINED
#define GrProcessorUnitTest_DEFINED

#include "GrColor.h"
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

static inline GrColor GrRandomColor(SkRandom* random) {
    // There are only a few cases of random colors which interest us
    enum ColorMode {
        kAllOnes_ColorMode,
        kAllZeros_ColorMode,
        kAlphaOne_ColorMode,
        kRandom_ColorMode,
        kLast_ColorMode = kRandom_ColorMode
    };

    ColorMode colorMode = ColorMode(random->nextULessThan(kLast_ColorMode + 1));
    GrColor color;
    switch (colorMode) {
        case kAllOnes_ColorMode:
            color = GrColorPackRGBA(0xFF, 0xFF, 0xFF, 0xFF);
            break;
        case kAllZeros_ColorMode:
            color = GrColorPackRGBA(0, 0, 0, 0);
            break;
        case kAlphaOne_ColorMode:
            color = GrColorPackRGBA(random->nextULessThan(256),
                                    random->nextULessThan(256),
                                    random->nextULessThan(256),
                                    0xFF);
            break;
        case kRandom_ColorMode:
            uint8_t alpha = random->nextULessThan(256);
            color = GrColorPackRGBA(random->nextRangeU(0, alpha),
                                    random->nextRangeU(0, alpha),
                                    random->nextRangeU(0, alpha),
                                    alpha);
            break;
    }
    GrColorIsPMAssert(color);
    return color;
}

static inline uint8_t GrRandomCoverage(SkRandom* random) {
    enum CoverageMode {
        kZero_CoverageMode,
        kAllOnes_CoverageMode,
        kRandom_CoverageMode,
        kLast_CoverageMode = kRandom_CoverageMode
    };

    CoverageMode colorMode = CoverageMode(random->nextULessThan(kLast_CoverageMode + 1));
    uint8_t coverage;
    switch (colorMode) {
        case kZero_CoverageMode:
            coverage = 0;
        case kAllOnes_CoverageMode:
            coverage = 0xff;
            break;
        case kRandom_CoverageMode:
            coverage = random->nextULessThan(256);
            break;
    }
    return coverage;
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
        VerifyFactoryCount();
        SkASSERT(GetFactories()->count());
        uint32_t idx = random->nextRangeU(0, GetFactories()->count() - 1);
        GrProcessorTestFactory<Processor>* factory = (*GetFactories())[idx];
        return factory->fCreateProc(random, context, caps, dummyTextures);
    }

    /*
     * A test function which verifies the count of factories.
     */
    static void VerifyFactoryCount();

private:
    CreateProc fCreateProc;

    static SkTArray<GrProcessorTestFactory<Processor>*, true>* GetFactories();
};

/** GrProcessor subclasses should insert this macro in their declaration to be included in the
 *  program generation unit test.
 */
#define GR_DECLARE_GEOMETRY_PROCESSOR_TEST                                                         \
    static GrProcessorTestFactory<GrGeometryProcessor> gTestFactory SK_UNUSED;                     \
    static GrGeometryProcessor* TestCreate(SkRandom*,                                              \
                                           GrContext*,                                             \
                                           const GrDrawTargetCaps&,                                \
                                           GrTexture* dummyTextures[2])

#define GR_DECLARE_FRAGMENT_PROCESSOR_TEST                                                         \
    static GrProcessorTestFactory<GrFragmentProcessor> gTestFactory SK_UNUSED;                     \
    static GrFragmentProcessor* TestCreate(SkRandom*,                                              \
                                           GrContext*,                                             \
                                           const GrDrawTargetCaps&,                                \
                                           GrTexture* dummyTextures[2])

#define GR_DECLARE_XP_FACTORY_TEST                                                                 \
    static GrProcessorTestFactory<GrXPFactory> gTestFactory SK_UNUSED;                             \
    static GrXPFactory* TestCreate(SkRandom*,                                                      \
                                   GrContext*,                                                     \
                                   const GrDrawTargetCaps&,                                        \
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

#define GR_DEFINE_XP_FACTORY_TEST(Factory)                                                         \
    GrProcessorTestFactory<GrXPFactory> Factory :: gTestFactory(Factory :: TestCreate)

#define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(Effect)                                                  \
    GrProcessorTestFactory<GrGeometryProcessor> Effect :: gTestFactory(Effect :: TestCreate)

#else // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_FRAGMENT_PROCESSOR_TEST                                                         \
    static GrFragmentProcessor* TestCreate(SkRandom*,                                              \
                                           GrContext*,                                             \
                                           const GrDrawTargetCaps&,                                \
                                           GrTexture* dummyTextures[2])
#define GR_DEFINE_FRAGMENT_PROCESSOR_TEST(X)

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_XP_FACTORY_TEST                                                                 \
    static GrXPFactory* TestCreate(SkRandom*,                                                      \
                                   GrContext*,                                                     \
                                   const GrDrawTargetCaps&,                                        \
                                   GrTexture* dummyTextures[2])
#define GR_DEFINE_XP_FACTORY_TEST(X)

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_GEOMETRY_PROCESSOR_TEST                                                         \
    static GrGeometryProcessor* TestCreate(SkRandom*,                                              \
                                           GrContext*,                                             \
                                           const GrDrawTargetCaps&,                                \
                                           GrTexture* dummyTextures[2])
#define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(X)

#endif // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
#endif
