/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrProcessorUnitTest_DEFINED
#define GrProcessorUnitTest_DEFINED

#include "include/core/SkTypes.h"

#if defined(GPU_TEST_UTILS)

#include "include/core/SkString.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"  // IWYU pragma: keep

#include <memory>
#include <tuple>

class GrCaps;
class GrGeometryProcessor;
class GrProcessorTestData;
class GrProxyProvider;
class GrRecordingContext;
class GrXPFactory;
class SkArenaAlloc;
class SkRandom;
enum SkAlphaType : int;
enum class GrColorType;
namespace skgpu::ganesh { class SurfaceDrawContext; }

namespace GrProcessorUnitTest {

/** This allows parent FPs to implement a test create with known leaf children in order to avoid
 *  creating an unbounded FP tree which may overflow various shader limits.
 *  MakeOptionalChildFP is the same as MakeChildFP, but can return null.
 */
std::unique_ptr<GrFragmentProcessor> MakeChildFP(GrProcessorTestData*);
std::unique_ptr<GrFragmentProcessor> MakeOptionalChildFP(GrProcessorTestData*);

}  // namespace GrProcessorUnitTest

/** GrProcessorTestData is an argument struct to TestCreate functions
 *  fTextures are valid textures that can optionally be used to construct
 *  TextureSampler. The first texture has a RGBA8 format and the second has Alpha8 format for the
 *  specific backend API. TestCreate functions are also free to create additional textures using
 *  the GrContext.
 */
class GrProcessorTestData {
public:
    using ViewInfo = std::tuple<GrSurfaceProxyView, GrColorType, SkAlphaType>;

    GrProcessorTestData(SkRandom* random, skgpu::ganesh::SurfaceDrawContext* sdc,
                        int maxTreeDepth, int numViews, const ViewInfo views[]);
    GrProcessorTestData(SkRandom* random, skgpu::ganesh::SurfaceDrawContext* sdc,
                        int maxTreeDepth, int numViews, const ViewInfo views[],
                        std::unique_ptr<GrFragmentProcessor> inputFP);
    GrProcessorTestData(const GrProcessorTestData&) = delete;
    ~GrProcessorTestData();

    GrRecordingContext* context() { return fContext; }
    skgpu::ganesh::SurfaceDrawContext* surfaceDrawContext() { return fDrawContext; }
    GrProxyProvider* proxyProvider();
    const GrCaps* caps();
    SkArenaAlloc* allocator() { return fArena.get(); }
    std::unique_ptr<GrFragmentProcessor> inputFP();

    ViewInfo randomView();
    ViewInfo randomAlphaOnlyView();

    SkRandom* fRandom;
    int fCurrentTreeDepth = 0;
    int fMaxTreeDepth = 1;

private:
    GrRecordingContext* fContext;
    skgpu::ganesh::SurfaceDrawContext* fDrawContext;
    skia_private::TArray<ViewInfo> fViews;
    std::unique_ptr<SkArenaAlloc> fArena;
    std::unique_ptr<GrFragmentProcessor> fInputFP;
};

template <class ProcessorSmartPtr>
class GrProcessorTestFactory : private SkNoncopyable {
public:
    using MakeProc = ProcessorSmartPtr (*)(GrProcessorTestData*);

    GrProcessorTestFactory(MakeProc makeProc, const char* name);

    /** Pick a random factory function and create a processor.  */
    static ProcessorSmartPtr Make(GrProcessorTestData* data);

    /** Use factory function at Index idx to create a processor. */
    static ProcessorSmartPtr MakeIdx(int idx, GrProcessorTestData* data);

    /** Number of registered factory functions */
    static int Count();

private:
    /** A test function which verifies the count of factories. */
    static void VerifyFactoryCount();
    static skia_private::TArray<GrProcessorTestFactory<ProcessorSmartPtr>*, true>* GetFactories();

    MakeProc fMakeProc;
    SkString fName;
};

using GrFragmentProcessorTestFactory = GrProcessorTestFactory<std::unique_ptr<GrFragmentProcessor>>;
using GrGeometryProcessorTestFactory = GrProcessorTestFactory<GrGeometryProcessor*>;

class GrXPFactoryTestFactory : private SkNoncopyable {
public:
    using GetFn = const GrXPFactory*(GrProcessorTestData*);

    GrXPFactoryTestFactory(GetFn* getProc);

    static const GrXPFactory* Get(GrProcessorTestData* data);

private:
    /** A test function which verifies the count of factories. */
    static void VerifyFactoryCount();
    static skia_private::TArray<GrXPFactoryTestFactory*, true>* GetFactories();

    GetFn* fGetProc;
};

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

/** GrProcessor subclasses should insert this macro in their declaration to be included in the
 *  program generation unit test.
 */
#define GR_DECLARE_GEOMETRY_PROCESSOR_TEST                                \
    [[maybe_unused]] static GrGeometryProcessorTestFactory* gTestFactory; \
    static GrGeometryProcessor* TestCreate(GrProcessorTestData*);

#define GR_DECLARE_FRAGMENT_PROCESSOR_TEST                                \
    [[maybe_unused]] static GrFragmentProcessorTestFactory* gTestFactory; \
    static std::unique_ptr<GrFragmentProcessor> TestCreate(GrProcessorTestData*);

#define GR_DECLARE_XP_FACTORY_TEST                                \
    [[maybe_unused]] static GrXPFactoryTestFactory* gTestFactory; \
    static const GrXPFactory* TestGet(GrProcessorTestData*);

/** GrProcessor subclasses should insert this macro in their implementation file. They must then
 *  also implement this static function:
 *      GrProcessor* TestCreate(GrProcessorTestData*);
 */
#define GR_DEFINE_FRAGMENT_PROCESSOR_TEST(Effect)          \
    GrFragmentProcessorTestFactory* Effect::gTestFactory = \
            new GrFragmentProcessorTestFactory(Effect::TestCreate, #Effect);

#define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(Effect)          \
    GrGeometryProcessorTestFactory* Effect::gTestFactory = \
            new GrGeometryProcessorTestFactory(Effect::TestCreate, #Effect);

#define GR_DEFINE_XP_FACTORY_TEST(Factory) \
    GrXPFactoryTestFactory* Factory::gTestFactory = new GrXPFactoryTestFactory(Factory::TestGet);

#else // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_FRAGMENT_PROCESSOR_TEST                                                         \
    static std::unique_ptr<GrFragmentProcessor> TestCreate(GrProcessorTestData*);
#define GR_DEFINE_FRAGMENT_PROCESSOR_TEST(X)

// The unit test relies on static initializers. Just declare the TestCreate function so that
// its definitions will compile.
#define GR_DECLARE_GEOMETRY_PROCESSOR_TEST                                                         \
    static GrGeometryProcessor* TestCreate(GrProcessorTestData*);
#define GR_DEFINE_GEOMETRY_PROCESSOR_TEST(X)

// The unit test relies on static initializers. Just declare the TestGet function so that
// its definitions will compile.
#define GR_DECLARE_XP_FACTORY_TEST                                                                 \
    const GrXPFactory* TestGet(GrProcessorTestData*);
#define GR_DEFINE_XP_FACTORY_TEST(X)

#endif  // !SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
#else   // defined(GPU_TEST_UTILS)
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
#endif  // defined(GPU_TEST_UTILS)
#endif  // GrProcessorUnitTest_DEFINED
