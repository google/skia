/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/gpu/GpuTypes.h"
#include "include/gpu/graphite/BackendTexture.h"
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/graphite/Recording.h"
#include "include/gpu/graphite/Surface.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ContextPriv.h"
#include "src/gpu/graphite/Surface_Graphite.h"
#include "src/gpu/graphite/Texture.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

using namespace skgpu::graphite;
using Mipmapped = skgpu::Mipmapped;

namespace {

// We draw the larger image into the smaller surface to force mipmapping
const SkISize kImageSize = { 32, 32 };
SkDEBUGCODE(constexpr int kNumMipLevels = 6;)
const SkISize kSurfaceSize = { 16, 16 };

constexpr int kNumMutations = 2;
constexpr SkColor4f kInitialColor = SkColors::kRed;
constexpr SkColor4f kMutationColors[kNumMutations] = {
    SkColors::kGreen,
    SkColors::kBlue
};

/*
 * We have 3 use cases. In each case there is a mutating task which changes the contents of an
 * image (somehow) and a shared redraw task which just creates a single Recording which draws the
 * image that is being mutated. The mutator's image must start off being 'kInitialColor' and
 * then cycle through 'kMutationColors'. The mutation tasks are:

 *  1) (AHBs) The client has wrapped a backend texture in an image and is changing the backend
 *     texture's contents.
 *  2) (Volatile Promise Images) The client has a pool of backend textures and updates both the
 *     contents of the backend textures and which one backs the image every frame
 *  3) (Surface/Image pair) The client has a surface and has snapped an image w/o a copy but
 *     keeps drawing to the surface
 *
 * There are also two scenarios for the mutation and redrawing tasks:
 *  a) Both use the same recorder
 *  b) They use separate recorders
 * The latter, obviously, requires more synchronization.
 */

// Base class for the 3 mutation methods.
//    init   - should create the SkImage that is going to be changing
//    mutate - should change the contents of the SkImage
class Mutator {
public:
    Mutator(skiatest::Reporter* reporter, Recorder* recorder, bool withMips)
            : fReporter(reporter)
            , fRecorder(recorder)
            , fWithMips(withMips) {
    }
    virtual ~Mutator() = default;

    virtual std::unique_ptr<Recording> init(const Caps*) = 0;
    virtual std::unique_ptr<Recording> mutate(int mutationIndex) = 0;
    virtual int getCase() const = 0;

    SkImage* getMutatingImage() {
        return fMutatingImg.get();
    }

protected:
    skiatest::Reporter* fReporter;
    Recorder* fRecorder;
    bool fWithMips;

    sk_sp<SkImage> fMutatingImg; // needs to be created in the 'init' method
};

// This class puts the 3 mutation use cases through their paces.
//    init - creates the single Recording that draws the mutator's image
//    checkResult - verifies that replaying the Recording results in the expected/mutated color
class Redrawer {
public:
    Redrawer(skiatest::Reporter* reporter, Recorder* recorder)
            : fReporter(reporter)
            , fRecorder(recorder) {
        SkImageInfo ii = SkImageInfo::Make(kSurfaceSize,
                                           kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);
        fReadbackPM.alloc(ii);
    }

    void init(SkImage* imageToDraw) {
        SkImageInfo ii = SkImageInfo::Make(kSurfaceSize,
                                           kRGBA_8888_SkColorType,
                                           kPremul_SkAlphaType);
        fImgDrawSurface = SkSurfaces::RenderTarget(fRecorder, ii, Mipmapped::kNo);
        REPORTER_ASSERT(fReporter, fImgDrawSurface);

        fImgDrawRecording = MakeRedrawRecording(fRecorder, fImgDrawSurface.get(), imageToDraw);
    }

    Recording* imgDrawRecording() {
        return fImgDrawRecording.get();
    }

    // This is here bc it uses a lot from the Redrawer (i.e., its recorder, its surface, etc.).
    void checkResult(Context* context,
                     int testcaseID,
                     bool useTwoRecorders,
                     bool withMips,
                     const SkColor4f& expectedColor) {

        fReadbackPM.erase(SkColors::kTransparent);

        if (!fImgDrawSurface->readPixels(fReadbackPM, 0, 0)) {
            ERRORF(fReporter, "readPixels failed");
        }

        auto error = std::function<ComparePixmapsErrorReporter>(
                [&](int x, int y, const float diffs[4]) {
                    ERRORF(fReporter,
                           "case %d%c - %s: "
                           "expected (%.1f %.1f %.1f %.1f) "
                           "- diffs (%.1f, %.1f, %.1f, %.1f)",
                           testcaseID, useTwoRecorders ? 'b' : 'a',
                           withMips ? "mipmapped" : "not-mipmapped",
                           expectedColor.fR, expectedColor.fG, expectedColor.fB, expectedColor.fA,
                           diffs[0], diffs[1], diffs[2], diffs[3]);
                });

        static constexpr float kTol[] = {0, 0, 0, 0};
        CheckSolidPixels(expectedColor, fReadbackPM, kTol, error);
    }

private:
    static std::unique_ptr<Recording> MakeRedrawRecording(Recorder* recorder,
                                                          SkSurface* surfaceToDrawTo,
                                                          SkImage* imageToDraw) {
        SkSamplingOptions sampling = SkSamplingOptions(SkFilterMode::kLinear,
                                                       SkMipmapMode::kNearest);

        SkCanvas* canvas = surfaceToDrawTo->getCanvas();

        canvas->clear(SkColors::kTransparent);
        canvas->drawImageRect(imageToDraw,
                              SkRect::MakeWH(kSurfaceSize.width(), kSurfaceSize.height()),
                              sampling);

        return recorder->snap();
    }

    skiatest::Reporter* fReporter;
    Recorder* fRecorder;

    sk_sp<SkSurface> fImgDrawSurface;
    std::unique_ptr<Recording> fImgDrawRecording;

    SkAutoPixmapStorage fReadbackPM;
};

void update_backend_texture(skiatest::Reporter* reporter,
                            Recorder* recorder,
                            const BackendTexture& backendTex,
                            SkColorType ct,
                            bool withMips,
                            SkColor4f color) {
    SkPixmap pixmaps[6];
    std::unique_ptr<char[]> memForPixmaps;

    const SkColor4f colors[6] = { color, color, color, color, color, color };

    int numMipLevels = ToolUtils::make_pixmaps(ct, kPremul_SkAlphaType, withMips, colors, pixmaps,
                                               &memForPixmaps);
    SkASSERT(numMipLevels == 1 || numMipLevels == kNumMipLevels);
    SkASSERT(kImageSize == pixmaps[0].dimensions());

    REPORTER_ASSERT(reporter, recorder->updateBackendTexture(backendTex, pixmaps, numMipLevels));
}

// case 1 (AHBs)
// To simulate the AHB use case this Mutator creates a BackendTexture and an SkImage that wraps
// it. To mutate the SkImage it simply updates the BackendTexture.
class UpdateBackendTextureMutator : public Mutator {
public:
    static std::unique_ptr<Mutator> Make(skiatest::Reporter* reporter,
                                         Recorder* recorder,
                                         bool withMips) {
        return std::make_unique<UpdateBackendTextureMutator>(reporter, recorder, withMips);
    }

    UpdateBackendTextureMutator(skiatest::Reporter* reporter, Recorder* recorder, bool withMips)
            : Mutator(reporter, recorder, withMips) {
    }
    ~UpdateBackendTextureMutator() override {
        fRecorder->deleteBackendTexture(fBETexture);
    }

    std::unique_ptr<Recording> init(const Caps* caps) override {
        // Note: not renderable
        TextureInfo info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                              fWithMips ? Mipmapped::kYes
                                                                        : Mipmapped::kNo,
                                                              skgpu::Protected::kNo,
                                                              skgpu::Renderable::kNo);
        REPORTER_ASSERT(fReporter, info.isValid());

        fBETexture = fRecorder->createBackendTexture(kImageSize, info);
        REPORTER_ASSERT(fReporter, fBETexture.isValid());

        update_backend_texture(fReporter, fRecorder, fBETexture, kRGBA_8888_SkColorType,
                               fWithMips, kInitialColor);

        fMutatingImg = SkImages::WrapTexture(fRecorder,
                                             fBETexture,
                                             kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType,
                                             /* colorSpace= */ nullptr);
        REPORTER_ASSERT(fReporter, fMutatingImg);

        return fRecorder->snap();
    }

    std::unique_ptr<Recording> mutate(int mutationIndex) override {
        update_backend_texture(fReporter, fRecorder, fBETexture, kRGBA_8888_SkColorType,
                               fWithMips, kMutationColors[mutationIndex]);
        return fRecorder->snap();
    }

    int getCase() const override { return 1; }

private:
    BackendTexture fBETexture;
};

// case 2 (Volatile Promise Images)
// To simulate the hardware video decoder use case this Mutator creates a set of BackendTextures
// and fills them w/ different colors. A single volatile Promise Image is created and is
// fulfilled by the different BackendTextures.
class VolatilePromiseImageMutator : public Mutator {
public:
    static std::unique_ptr<Mutator> Make(skiatest::Reporter* reporter,
                                         Recorder* recorder,
                                         bool withMips) {
        return std::make_unique<VolatilePromiseImageMutator>(reporter, recorder, withMips);
    }

    VolatilePromiseImageMutator(skiatest::Reporter* reporter, Recorder* recorder, bool withMips)
            : Mutator(reporter, recorder, withMips) {
    }

    ~VolatilePromiseImageMutator() override {
        // We need to delete the mutating image first since it holds onto the backend texture
        // that was last used to fulfill the volatile promise image.
        fMutatingImg.reset();

        fCallbackTracker.finishedTest();

        for (int i = 0; i < kNumMutations+1; ++i) {
            fRecorder->deleteBackendTexture(fBETextures[i]);
        }
    }

    static std::tuple<BackendTexture, void*> fulfill(void* ctx) {
        VolatilePromiseImageMutator* mutator = reinterpret_cast<VolatilePromiseImageMutator*>(ctx);

        int index = mutator->fCallbackTracker.onFulfillCB();

        return { mutator->fBETextures[index], &mutator->fCallbackTracker };
    }

    static void imageRelease(void* ctx) {
        VolatilePromiseImageMutator* mutator = reinterpret_cast<VolatilePromiseImageMutator*>(ctx);

        mutator->fCallbackTracker.onImageReleaseCB();
    }

    static void textureRelease(void* ctx) {
        CallbackTracker* callbackTracker = reinterpret_cast<CallbackTracker*>(ctx);

        callbackTracker->onTextureReleaseCB();
    }

    std::unique_ptr<Recording> init(const Caps* caps) override {
        // Note: not renderable
        TextureInfo info = caps->getDefaultSampledTextureInfo(kRGBA_8888_SkColorType,
                                                              fWithMips ? Mipmapped::kYes
                                                                        : Mipmapped::kNo,
                                                              skgpu::Protected::kNo,
                                                              skgpu::Renderable::kNo);
        REPORTER_ASSERT(fReporter, info.isValid());

        fBETextures[0] = fRecorder->createBackendTexture(kImageSize, info);
        REPORTER_ASSERT(fReporter, fBETextures[0].isValid());

        update_backend_texture(fReporter, fRecorder, fBETextures[0], kRGBA_8888_SkColorType,
                               fWithMips, kInitialColor);

        for (int i = 0; i < kNumMutations; ++i) {
            fBETextures[i+1] = fRecorder->createBackendTexture(kImageSize, info);
            REPORTER_ASSERT(fReporter, fBETextures[i+1].isValid());

            update_backend_texture(fReporter, fRecorder, fBETextures[i+1], kRGBA_8888_SkColorType,
                                   fWithMips, kMutationColors[i]);
        }

        fMutatingImg = SkImages::PromiseTextureFrom(fRecorder,
                                                    kImageSize,
                                                    info,
                                                    SkColorInfo(kRGBA_8888_SkColorType,
                                                                kPremul_SkAlphaType,
                                                                /* cs= */ nullptr),
                                                    Volatile::kYes,
                                                    fulfill,
                                                    imageRelease,
                                                    textureRelease,
                                                    this);
        REPORTER_ASSERT(fReporter, fMutatingImg);

        return fRecorder->snap();
    }

    std::unique_ptr<Recording> mutate(int mutationIndex) override {
        fCallbackTracker.onMutation();
        return nullptr;
    }

    int getCase() const override { return 2; }

private:
    class CallbackTracker {
    public:
        CallbackTracker() {
            for (int i = 0; i < kNumMutations+1; ++i) {
                fFulfilled[i] = false;
                fReleased[i] = false;
            }
        }

        void onMutation() {
            // In this use case, the active mutation occurs in the volatile promise image callbacks.
            ++fMutationCount;
        }

        int onFulfillCB() {
            SkASSERT(fMutationCount < kNumMutations+1);
            SkASSERT(fFulfilledCount == fMutationCount);
            // For this unit test we should only be fulfilling with each backend texture only once
            SkASSERT(!fFulfilled[fFulfilledCount]);
            SkASSERT(!fReleased[fFulfilledCount]);

            fFulfilled[fFulfilledCount] = true;
            return fFulfilledCount++;
        }

        void onImageReleaseCB() {
            SkASSERT(!fImageReleased);
            fImageReleased = true;
        }

        void onTextureReleaseCB() {
            SkASSERT(fReleasedCount >= 0 && fReleasedCount < kNumMutations+1);

            SkASSERT(fFulfilled[fReleasedCount]);
            SkASSERT(!fReleased[fReleasedCount]);
            fReleased[fReleasedCount] = true;
            fReleasedCount++;
        }

        void finishedTest() const {
            SkASSERT(fMutationCount == kNumMutations);
            SkASSERT(fImageReleased);

            for (int i = 0; i < kNumMutations+1; ++i) {
                SkASSERT(fFulfilled[i]);
                SkASSERT(fReleased[i]);
            }
        }

    private:
        int fMutationCount = 0;
        int fFulfilledCount = 0;
        bool fImageReleased = false;
        int fReleasedCount = 0;
        bool fFulfilled[kNumMutations+1];
        bool fReleased[kNumMutations+1];
    };

    CallbackTracker fCallbackTracker;

    BackendTexture fBETextures[kNumMutations+1];
};

// case 3 (Surface/Image pair)
// This mutator creates an SkSurface/SkImage pair that share the same backend object.
// Mutation is accomplished by simply drawing to the SkSurface.
class SurfaceMutator : public Mutator {
public:
    static std::unique_ptr<Mutator> Make(skiatest::Reporter* reporter,
                                         Recorder* recorder,
                                         bool withMips) {
        return std::make_unique<SurfaceMutator>(reporter, recorder, withMips);
    }

    SurfaceMutator(skiatest::Reporter* reporter, Recorder* recorder, bool withMips)
            : Mutator(reporter, recorder, withMips) {
    }

    std::unique_ptr<Recording> init(const Caps* /* caps */) override {
        SkImageInfo ii = SkImageInfo::Make(kImageSize, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

        fMutatingSurface = SkSurfaces::RenderTarget(
                fRecorder, ii, fWithMips ? Mipmapped::kYes : Mipmapped::kNo);
        REPORTER_ASSERT(fReporter, fMutatingSurface);

        fMutatingSurface->getCanvas()->clear(kInitialColor);

        fMutatingImg = SkSurfaces::AsImage(fMutatingSurface);
        REPORTER_ASSERT(fReporter, fMutatingImg);

        return fRecorder->snap();
    }

    std::unique_ptr<Recording> mutate(int mutationIndex) override {
        fMutatingSurface->getCanvas()->clear(kMutationColors[mutationIndex]);
        return fRecorder->snap();
    }

    int getCase() const override { return 3; }

private:
    sk_sp<SkSurface> fMutatingSurface;
};

using MutatorFactoryT = std::unique_ptr<Mutator> (*)(skiatest::Reporter*, Recorder*, bool withMips);

void run_test(skiatest::Reporter* reporter,
              Context* context,
              bool useTwoRecorders,
              bool withMips,
              MutatorFactoryT createMutator) {
    const Caps* caps = context->priv().caps();

    std::unique_ptr<Recorder> recorders[2];
    recorders[0] = context->makeRecorder();

    Recorder* mutatorRecorder = recorders[0].get();
    Recorder* redrawerRecorder = recorders[0].get();

    if (useTwoRecorders) {
        recorders[1] = context->makeRecorder();
        redrawerRecorder = recorders[1].get();
    }

    std::unique_ptr<Mutator> mutator = createMutator(reporter, mutatorRecorder, withMips);

    {
        std::unique_ptr<Recording> imgCreationRecording = mutator->init(caps);
        REPORTER_ASSERT(reporter, context->insertRecording({ imgCreationRecording.get() }));
    }

    {
        Redrawer redrawer(reporter, redrawerRecorder);

        redrawer.init(mutator->getMutatingImage());

        REPORTER_ASSERT(reporter, context->insertRecording({ redrawer.imgDrawRecording() }));
        redrawer.checkResult(context, mutator->getCase(),
                             useTwoRecorders, withMips, kInitialColor);

        for (int i = 0; i < kNumMutations; ++i) {
            {
                std::unique_ptr<Recording> imgMutationRecording = mutator->mutate(i);
                if (imgMutationRecording) {
                    REPORTER_ASSERT(reporter,
                                    context->insertRecording({imgMutationRecording.get()}));
                }
            }

            REPORTER_ASSERT(reporter, context->insertRecording({ redrawer.imgDrawRecording() }));
            redrawer.checkResult(context, mutator->getCase(),
                                 useTwoRecorders, withMips, kMutationColors[i]);
        }
    }
}

} // anonymous namespace

DEF_GRAPHITE_TEST_FOR_RENDERING_CONTEXTS(MutableImagesTest, reporter, context,
                                         CtsEnforcement::kNextRelease) {

    for (bool useTwoRecorders : { false, true }) {
        for (bool withMips : { false, true }) {
            // case 1 (AHBs)
            run_test(reporter, context, useTwoRecorders, withMips,
                     UpdateBackendTextureMutator::Make);

            // case 2 (Volatile Promise Images)
            run_test(reporter, context, useTwoRecorders, withMips,
                     VolatilePromiseImageMutator::Make);

            // case 3 (Surface/Image pair)
            if (!withMips) {
                // TODO: allow the mipmapped version when we can automatically regenerate mipmaps
                run_test(reporter, context, useTwoRecorders, withMips,
                         SurfaceMutator::Make);
            }
        }
    }
}
