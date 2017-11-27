/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkMipMap.h"
#include "SkRandom.h"
#include "SkSurface.h"
#include "SkUtils.h"

#include "GrContextPriv.h"
#include "GrTest.h"
#include "GrRenderTarget.h"
#include "GrTexture.h"
#include "GrTypes.h"

#include "Resources.h"

struct DrawOptions {
    DrawOptions(int w, int h, bool r, bool g, bool p, bool k, bool srgb, bool f16,
                bool textOnly, const char* s,
                GrMipMapped mipMapping,
                int offScreenWidth,
                int offScreenHeight,
                int offScreenSampleCount,
                GrMipMapped offScreenMipMapping)
        : size(SkISize::Make(w, h))
        , raster(r)
        , gpu(g)
        , pdf(p)
        , skp(k)
        , srgb(srgb)
        , f16(f16)
        , textOnly(textOnly)
        , source(s)
        , fMipMapping(mipMapping)
        , fOffScreenWidth(offScreenWidth)
        , fOffScreenHeight(offScreenHeight)
        , fOffScreenSampleCount(offScreenSampleCount)
        , fOffScreenMipMapping(offScreenMipMapping) {
        // F16 mode is only valid for color correct backends.
        SkASSERT(srgb || !f16);
    }
    SkISize size;
    bool raster;
    bool gpu;
    bool pdf;
    bool skp;
    bool srgb;
    bool f16;
    bool textOnly;
    const char* source;

    // This flag is used when a GPU texture resource is created and exposed as a GrBackendTexture.
    // In this case the resource is created with extra room to accomodate mipmaps.
    // TODO: The SkImage::makeTextureImage API would need to be widened to allow this to be true
    // for the non-backend gpu SkImages.
    GrMipMapped fMipMapping;

    // Parameters for an GPU offscreen resource exposed as a GrBackendRenderTarget
    int         fOffScreenWidth;
    int         fOffScreenHeight;
    int         fOffScreenSampleCount;
    // TODO: should we also expose stencilBits here? How about the config?

    GrMipMapped fOffScreenMipMapping; // only applicable if the offscreen is also textureable
};

// Globals externed in fiddle_main.h
sk_sp<GrTexture>      backingTexture;  // not externed
GrBackendTexture      backEndTexture;

sk_sp<GrRenderTarget> backingRenderTarget; // not externed
GrBackendRenderTarget backEndRenderTarget;

sk_sp<GrTexture>      backingTextureRenderTarget;  // not externed
GrBackendTexture      backEndTextureRenderTarget;

static bool setup_backend_objects(GrContext* context,
                                  const SkBitmap& bm,
                                  const DrawOptions& options) {
    if (!context) {
        return false;
    }

    GrBackend backend = context->contextPriv().getBackend();
    const GrPixelConfig kConfig = kRGBA_8888_GrPixelConfig;

    GrSurfaceDesc backingDesc;
    backingDesc.fFlags = kNone_GrSurfaceFlags;
    backingDesc.fOrigin = kTopLeft_GrSurfaceOrigin;
    backingDesc.fWidth = bm.width();
    backingDesc.fHeight = bm.height();
    backingDesc.fConfig = kConfig;
    backingDesc.fSampleCnt = 0;

    if (!bm.empty()) {
        int mipLevelCount = GrMipMapped::kYes == options.fMipMapping
                                    ? SkMipMap::ComputeLevelCount(bm.width(), bm.height())
                                    : 1;
        std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipLevelCount]);

        texels[0].fPixels = bm.getPixels();
        texels[0].fRowBytes = bm.rowBytes();

        for (int i = 1; i < mipLevelCount; i++) {
            texels[i].fPixels = nullptr;
            texels[i].fRowBytes = 0;
        }

        // w/o loosening of the assert this fails the assert in GrResourceProvider::createTexture
        backingTexture = context->resourceProvider()->createTexture(
                                                        backingDesc, SkBudgeted::kNo,
                                                        texels.get(), mipLevelCount,
                                                        SkDestinationSurfaceColorMode::kLegacy);
        if (!backingTexture) {
            return false;
        }

        backEndTexture = GrTest::CreateBackendTexture(backend,
                                                      backingDesc.fWidth,
                                                      backingDesc.fHeight,
                                                      kConfig,
                                                      options.fMipMapping,
                                                      backingTexture->getTextureHandle());
        if (!backEndTexture.isValid()) {
            return false;
        }
    }

    backingDesc.fFlags = kRenderTarget_GrSurfaceFlag;
    backingDesc.fWidth = options.fOffScreenWidth;
    backingDesc.fHeight = options.fOffScreenHeight;
    backingDesc.fSampleCnt = options.fOffScreenSampleCount;

    SkAutoTMalloc<uint32_t> data(backingDesc.fWidth * backingDesc.fHeight);
    sk_memset32(data.get(), 0, backingDesc.fWidth * backingDesc.fHeight);


    {
        // This backend object should be renderable but not textureable. Given the limitations
        // of how we're creating it though it will wind up being secretly textureable.
        // We use this fact to initialize it with data but don't allow mipmaps
        GrMipLevel level0 = { data.get(), backingDesc.fWidth*sizeof(uint32_t) };

        sk_sp<GrTexture> tmp = context->resourceProvider()->createTexture(
                                                            backingDesc, SkBudgeted::kNo,
                                                            &level0, 1,
                                                            SkDestinationSurfaceColorMode::kLegacy);
        if (!tmp || !tmp->asRenderTarget()) {
            return false;
        }

        backingRenderTarget = sk_ref_sp(tmp->asRenderTarget());

        backEndRenderTarget = GrTest::CreateBackendRenderTarget(
                                                    backend,
                                                    backingDesc.fWidth,
                                                    backingDesc.fHeight,
                                                    backingDesc.fSampleCnt, 0,
                                                    kConfig,
                                                    backingRenderTarget->getRenderTargetHandle());
        if (!backEndRenderTarget.isValid()) {
            return false;
        }
    }

    {
        int mipLevelCount = GrMipMapped::kYes == options.fOffScreenMipMapping
                            ? SkMipMap::ComputeLevelCount(backingDesc.fWidth, backingDesc.fHeight)
                            : 1;
        std::unique_ptr<GrMipLevel[]> texels(new GrMipLevel[mipLevelCount]);

        texels[0].fPixels = data.get();
        texels[0].fRowBytes = backingDesc.fWidth*sizeof(uint32_t);

        for (int i = 1; i < mipLevelCount; i++) {
            texels[i].fPixels = nullptr;
            texels[i].fRowBytes = 0;
        }

        backingTextureRenderTarget = context->resourceProvider()->createTexture(
                                                            backingDesc, SkBudgeted::kNo,
                                                            texels.get(), mipLevelCount,
                                                            SkDestinationSurfaceColorMode::kLegacy);
        if (!backingTextureRenderTarget || !backingTextureRenderTarget->asRenderTarget()) {
            return false;
        }

        backEndTextureRenderTarget = GrTest::CreateBackendTexture(
                                                    backend,
                                                    backingDesc.fWidth,
                                                    backingDesc.fHeight,
                                                    kConfig,
                                                    options.fOffScreenMipMapping,
                                                    backingTextureRenderTarget->getTextureHandle());
        if (!backEndTextureRenderTarget.isValid()) {
            return false;
        }
    }


    return true;
}

namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(1000, 1000);
    }

    void onDraw(SkCanvas* canvas) override {

        GrContext* context = canvas->getGrContext();
        if (context) {
            DrawOptions options(512, 512, false, true, false, false, false, false,
                                false, "foo", GrMipMapped(true), 512, 512, 0, GrMipMapped(true));

            SkBitmap bm;
            GetResourceAsBitmap("mandrill_512.png", &bm);

            setup_backend_objects(context, bm, options);

            sk_sp<SkImage> tmp = SkImage::MakeFromTexture(context,
                                                          backEndTexture,
                                                          kTopLeft_GrSurfaceOrigin,
                                                          kOpaque_SkAlphaType,
                                                          nullptr);

            // TODO: this sampleCnt parameter here should match that set in the options!
            sk_sp<SkSurface> tmp2 = SkSurface::MakeFromBackendTexture(context,
                                                                      backEndTextureRenderTarget,
                                                                      kTopLeft_GrSurfaceOrigin,
                                                                      0, nullptr, nullptr);

            // Note: this surface should only be renderable (i.e., not textureable)
            sk_sp<SkSurface> tmp3 = SkSurface::MakeFromBackendRenderTarget(context,
                                                                           backEndRenderTarget,
                                                                           kTopLeft_GrSurfaceOrigin,
                                                                           nullptr, nullptr);
        }


    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}
