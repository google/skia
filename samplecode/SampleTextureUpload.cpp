/*
 * Copyright 2019 Google Inc. and Adobe Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "samplecode/Sample.h"
#include "tools/timer/TimeUtils.h"

/**
 * This sample exercises heavy texture updates and uploads.
 */
class TextureUploadSample : public Sample {
    static constexpr int kMinTileSize = 128;
    static constexpr int kMaxTileSize = 2048;
    static constexpr float kGridScale = 0.25f;

    bool fDrawTexturesToScreen = true;
    int fTileSize = 256;
    int fTileRows = 8;
    int fTileCols = 8;

    sk_sp<SkSurface> fBlueSurface;
    sk_sp<SkSurface> fGraySurface;

    class RenderTargetTexture : public SkRefCnt {
    public:
        RenderTargetTexture(GrDirectContext* direct, int size) {
            SkSurfaceProps surfaceProps(0, kRGB_H_SkPixelGeometry);
            SkImageInfo imageInfo = SkImageInfo::Make(size, size, kRGBA_8888_SkColorType,
                                                      kPremul_SkAlphaType);
            fSurface = SkSurface::MakeRenderTarget(direct, SkBudgeted::kNo, imageInfo, 0,
                                                   &surfaceProps);
        }

        sk_sp<SkImage> getImage() {
            return fSurface->makeImageSnapshot();
        }

        void uploadRasterSurface(sk_sp<SkSurface> rasterSurface) {
            SkPixmap pixmap;
            rasterSurface->peekPixels(&pixmap);
            fSurface->writePixels(pixmap, 0, 0);
        }

    private:
        sk_sp<SkSurface> fSurface;
        sk_sp<SkImage> fCachedImage;
    };

    SkTArray<sk_sp<RenderTargetTexture>> fTextures;
    GrDirectContext* fCachedContext = nullptr;
    SkScalar fActiveTileIndex = 0;

    SkString name() override {
        return SkString("TextureUpload");
    }

    bool onChar(SkUnichar uni) override {
        if ('m' == uni) {
            fDrawTexturesToScreen = !fDrawTexturesToScreen;
            return true;
        } else if ('>' == uni) {
            fTileSize = std::min(kMaxTileSize, 2*fTileSize);
            fTileRows = kMaxTileSize/fTileSize;
            fTileCols = kMaxTileSize/fTileSize;
            fCachedContext = nullptr;
        } else if ('<' == uni) {
            fTileSize = std::max(kMinTileSize, fTileSize/2);
            fTileRows = kMaxTileSize/fTileSize;
            fTileCols = kMaxTileSize/fTileSize;
            fCachedContext = nullptr;
        }
        return false;
    }

    sk_sp<SkSurface> getFilledRasterSurface(SkColor color, int size) {
        sk_sp<SkSurface> surface(SkSurface::MakeRasterN32Premul(size, size));
        SkCanvas* canvas = surface->getCanvas();
        canvas->clear(color);
        return surface;
    }

    void onOnceBeforeDraw() override {
        this->setBGColor(0xFFFFFFFF);
        this->setSize(1024, 1024);
    }

    void initializeTextures(GrDirectContext* direct) {
        fTextures.reset();
        int textureCount = fTileRows * fTileCols;
        for (int i = 0; i < textureCount; i++) {
            fTextures.emplace_back(new RenderTargetTexture(direct, fTileSize));
        }

        // Construct two simple rasters of differing colors to serve
        // as cpu rasterized data to refresh textures with.
        fBlueSurface = this->getFilledRasterSurface(SK_ColorBLUE, fTileSize);
        fGraySurface = this->getFilledRasterSurface(SK_ColorGRAY, fTileSize);
    }

    void onDrawContent(SkCanvas* canvas) override {
#if SK_SUPPORT_GPU
        auto direct = GrAsDirectContext(canvas->recordingContext());
        if (direct) {
            // One-time context-specific setup.
            if (direct != fCachedContext) {
                fCachedContext = direct;
                this->initializeTextures(direct);
            }

            // Upload new texture data for all textures, simulating a full page of tiles
            // needing refresh.
            int textureCount = fTileRows * fTileCols;
            for (int i = 0; i < textureCount; i++) {
                fTextures[i]->uploadRasterSurface(i == fActiveTileIndex ? fBlueSurface
                                                                        : fGraySurface);
            }

            // Scale grid.
            canvas->scale(kGridScale, kGridScale);

            if (fDrawTexturesToScreen) {
                for (int y = 0; y < fTileRows; y++) {
                    for (int x = 0; x < fTileCols; x++) {
                        int currentIndex = y * fTileCols + x;
                        canvas->drawImage(fTextures[currentIndex]->getImage(),
                                          x * fTileSize, y * fTileSize);
                    }
                }
            }
        }
#endif
    }

    bool onAnimate(double nanos) override {
        constexpr SkScalar kDesiredDurationSecs = 16.0f;
        float numTiles = fTileRows*fTileCols;
        fActiveTileIndex = floorf(TimeUtils::Scaled(1e-9 * nanos,
                                                    numTiles/kDesiredDurationSecs, numTiles));
        return true;
    }
};


DEF_SAMPLE( return new TextureUploadSample(); )

