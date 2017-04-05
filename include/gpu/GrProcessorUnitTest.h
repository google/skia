/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessorUnitTest_DEFINED
#define GrProcessorUnitTest_DEFINED

#include "SkTypes.h"

#if GR_TEST_UTILS

#include "../private/GrTextureProxy.h"
#include "../private/SkTArray.h"
#include "GrTestUtils.h"

class SkMatrix;
class GrCaps;
class GrContext;
class GrRenderTargetContext;
struct GrProcessorTestData;
class GrTexture;
class GrXPFactory;

namespace GrProcessorUnitTest {

// Used to access the dummy textures in TestCreate procs.
enum {
    kSkiaPMTextureIdx = 0,
    kAlphaTextureIdx = 1,
};

/** This allows parent FPs to implement a test create with known leaf children in order to avoid
creating an unbounded FP tree which may overflow various shader limits. */
sk_sp<GrFragmentProcessor> MakeChildFP(GrProcessorTestData*);

}

/*
 * GrProcessorTestData is an argument struct to TestCreate functions
 * fTextures are valid textures that can optionally be used to construct
 * TextureSampler. The first texture has config kSkia8888_GrPixelConfig and the second has
 * kAlpha_8_GrPixelConfig. TestCreate functions are also free to create additional textures using
 * the GrContext.
 */
struct GrProcessorTestData {
    GrProcessorTestData(SkRandom* random,
                        GrContext* context,
                        const GrRenderTargetContext* renderTargetContext,
                        sk_sp<GrTextureProxy> proxies[2])
            : fRandom(random)
            , fRenderTargetContext(renderTargetContext)
            , fContext(context) {
        fProxies[0] = proxies[0];
        fProxies[1] = proxies[1];
    }
    SkRandom* fRandom;
    const GrRenderTargetContext* fRenderTargetContext;

    GrContext* context() { return fContext; }
    GrResourceProvider* resourceProvider();
    const GrCaps* caps();
    sk_sp<GrTextureProxy> textureProxy(int index) { return fProxies[index]; }

private:
    GrContext* fContext;
    sk_sp<GrTextureProxy> fProxies[2];
};

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

class GrProcessor;
class GrTexture;

template <class Processor> class GrProcessorTestFactory : private SkNoncopyable {
public:
    typedef sk_sp<Processor> (*MakeProc)(GrProcessorTestData*);

    GrProcessorTestFactory(MakeProc makeProc) {
        fMakeProc = makeProc;
        GetFactories()->push_back(this);
    }

    /** Pick a random factory function and create a processor.  */
    static sk_sp<Processor> Make(GrProcessorTestData* data) {
        VerifyFactoryCount();
        SkASSERT(GetFactories()->count());
        uint32_t idx = data->fRandom->nextRangeU(0, GetFactories()->count() - 1);
        return MakeIdx(idx, data);
    }

    /** Number of registered factory functions */
    static int Count() { return GetFactories()->count(); }

    /** Use factory function at Index idx to create a processor. */
    static sk_sp<Processor> MakeIdx(int idx, GrProcessorTestData* data) {
        GrProcessorTestFactory<Processor>* factory = (*GetFactories())[idx];
        sk_sp<Processor> processor = factory->fMakeProc(data);
        SkASSERT(processor);
        return processor;
    }

private:
    /**
     * A test function which verifies the count of factories.
     */
    static void VerifyFactoryCount();

    MakeProc fMakeProc;

    static SkTArray<GrProcessorTestFactory<Processor>*, true>* GetFactories();
};

class GrXPFactoryTestFactory : private SkNoncopyable {
public:
    using GetFn = const GrXPFactory*(GrProcessorTestData*);

    GrXPFactoryTestFactory(GetFn* getProc) : fGetProc(getProc) { GetFactories()->push_back(this); }

    static const GrXPFactory* Get(GrProcessorTestData* data) {
        VerifyFactoryCount();
        SkASSERT(GetFactories()->count());
        uint32_t idx = data->fRandom->nextRangeU(0, GetFactories()->count() - 1);
        const GrXPFactory* xpf = (*GetFactories())[idx]->fGetProc(data);
        SkASSERT(xpf);
        return xpf;
    }

private:
    static void VerifyFactoryCount();

    GetFn* fGetProc;
    static SkTArray<GrXPFactoryTestFactory*, true>* GetFactories();
};

/** GrProcessor subclasses should insert this macro in their declaration to be included in the
 *  program generation unit test.
 */
#define GR_DECLARE_GEOMETRY_PROCESSOR_TEST                                                         \
    static GrProcessorTestFactory<GrGeometryProcessor> gTestFactory SK_UNUSED;                     \
    static sk_sp<GrGeometryProcessor> TestCreate(GrProcessorTestData*)

#define GR_DECLARE_FRAGMENT_PROCESSOR_TEST                                                         \
    static GrProcessorTestFactory<GrFragmentProcessor> gTestFactory SK_UNUSED;                     \
    static sk_sp<GrFragmentProcessor> TestCreate(GrProcessorTestData*)

#define GR_DECLARE_XP_FACTORY_TEST                                                                 \
    static GrXPFactoryTestFactory gTestFactory SK_UNUSED;                                          \
    static const GrXPFactory* TestGet(GrProcessorTestData*)

/** GrProcessor subclasses should insert this macro in their implementation file. They must then
 *  also implement this static function:
 *      GrProcessor* TestCreate(GrProcessorTestData*);
 */
#define GR_DEFINE_FRAGMENT_PROCESSOR_TEST(Effect)                                                  \
    GrProcessorTestFactory<GrFragmentProcessor> Effect::gTestFactory(Effect::TestCreate)

#define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(Effect)                                                  \
    GrProcessorTestFactory<GrGeometryProcessor> Effect::gTestFactory(Effect::TestCreate)

#define GR_DEFINE_XP_FACTORY_TEST(Factory)                                                         \
    GrXPFactoryTestFactory Factory::gTestFactory(Factory::TestGet)

#else // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_FRAGMENT_PROCESSOR_TEST                                                         \
    static sk_sp<GrFragmentProcessor> TestCreate(GrProcessorTestData*)
#define GR_DEFINE_FRAGMENT_PROCESSOR_TEST(X)

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_GEOMETRY_PROCESSOR_TEST                                                         \
    static sk_sp<GrGeometryProcessor> TestCreate(GrProcessorTestData*)
#define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(X)

// The unit test relies on static initializers. Just declare the TestGet function so that
// its definitions will compile.
#define GR_DECLARE_XP_FACTORY_TEST                                                                 \
    const GrXPFactory* TestGet(GrProcessorTestData*)
#define GR_DEFINE_XP_FACTORY_TEST(X)

#endif  // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
#else   // GR_TEST_UTILS
    #define GR_DECLARE_GEOMETRY_PROCESSOR_TEST
    #define GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    #define GR_DECLARE_XP_FACTORY_TEST
    #define GR_DEFINE_FRAGMENT_PROCESSOR_TEST(...)
    #define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(...)
    #define GR_DEFINE_XP_FACTORY_TEST(...)
    #define GR_DECLARE_FRAGMENT_PROCESSOR_TEST
    #define GR_DEFINE_FRAGMENT_PROCESSOR_TEST(...)
    #define GR_DECLARE_GEOMETRY_PROCESSOR_TEST
    #define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(...)
    #define GR_DECLARE_XP_FACTORY_TEST
    #define GR_DEFINE_XP_FACTORY_TEST(...)
#endif  // GR_TEST_UTILS
#endif  // GrProcessorUnitTest_DEFINED
