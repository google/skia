/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrEffectUnitTest_DEFINED
#define GrEffectUnitTest_DEFINED

#include "GrNoncopyable.h"
#include "SkRandom.h"
#include "SkTArray.h"

class SkMatrix;
class GrDrawTargetCaps;

namespace GrEffectUnitTest {
// Used to access the dummy textures in TestCreate procs.
enum {
    kSkiaPMTextureIdx = 0,
    kAlphaTextureIdx = 1,
};

/**
 * A helper for use in GrEffect::TestCreate functions.
 */
const SkMatrix& TestMatrix(SkMWCRandom*);

}

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

class GrContext;
class GrEffectRef;
class GrTexture;

class GrEffectTestFactory : GrNoncopyable {
public:

    typedef GrEffectRef* (*CreateProc)(SkMWCRandom*,
                                       GrContext*,
                                       const GrDrawTargetCaps& caps,
                                       GrTexture* dummyTextures[]);

    GrEffectTestFactory(CreateProc createProc) {
        fCreateProc = createProc;
        GetFactories()->push_back(this);
    }

    static GrEffectRef* CreateStage(SkMWCRandom* random,
                                    GrContext* context,
                                    const GrDrawTargetCaps& caps,
                                    GrTexture* dummyTextures[]) {
        uint32_t idx = random->nextRangeU(0, GetFactories()->count() - 1);
        GrEffectTestFactory* factory = (*GetFactories())[idx];
        return factory->fCreateProc(random, context, caps, dummyTextures);
    }

private:
    CreateProc fCreateProc;
    static SkTArray<GrEffectTestFactory*, true>* GetFactories();
};

/** GrEffect subclasses should insert this macro in their declaration to be included in the
 *  program generation unit test.
 */
#define GR_DECLARE_EFFECT_TEST                                                      \
    static GrEffectTestFactory gTestFactory;                                        \
    static GrEffectRef* TestCreate(SkMWCRandom*,                                    \
                                   GrContext*,                                      \
                                   const GrDrawTargetCaps&,                         \
                                   GrTexture* dummyTextures[2])

/** GrEffect subclasses should insert this macro in their implementation file. They must then
 *  also implement this static function:
 *      GrEffect* TestCreate(SkMWCRandom*,
 *                           GrContext*,
 *                           const GrDrawTargetCaps&,
 *                           GrTexture* dummyTextures[2]);
 * dummyTextures[] are valid textures that can optionally be used to construct GrTextureAccesses.
 * The first texture has config kSkia8888_GrPixelConfig and the second has
 * kAlpha_8_GrPixelConfig. TestCreate functions are also free to create additional textures using
 * the GrContext.
 */
#define GR_DEFINE_EFFECT_TEST(Effect)                                               \
    GrEffectTestFactory Effect :: gTestFactory(Effect :: TestCreate)

#else // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_EFFECT_TEST                                                      \
    static GrEffectRef* TestCreate(SkMWCRandom*,                                    \
                                   GrContext*,                                      \
                                   const GrDrawTargetCaps&,                         \
                                   GrTexture* dummyTextures[2])
#define GR_DEFINE_EFFECT_TEST(X)

#endif // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
#endif
