/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"

#include "SkCanvas.h"
#include "SkImage.h"
#include "SkRandom.h"
#include "SkSurface.h"

enum class ClampingMode {
    // Submit image set entries with the fast constraint
    kAlwaysFast,
    // Submit image set entries with the strict constraint
    kAlwaysStrict,
    // Submit non-right/bottom tiles as fast, the bottom-right corner as strict, and bottom or right
    // edge tiles as strict with geometry modification to match content area. These will be
    // submitted from left-to-right, top-to-bottom so will necessarily be split into many batches.
    kChromeTiling_RowMajor,
    // As above, but group all fast tiles first, then bottom and right edge tiles in a second batch.
    kChromeTiling_Optimal
};

enum class TransformMode {
    // Tiles will be axis aligned on integer pixels
    kNone,
    // Subpixel, tiles will be axis aligned but adjusted to subpixel coordinates
    kSubpixel,
    // Rotated, tiles will be rotated globally; they won't overlap but their device space bounds may
    kRotated,
    // Perspective, tiles will have global perspective
    kPerspective
};

/**
 * Simulates drawing layers images in a grid a la a tile based compositor.
 */
class CompositingImages : public Benchmark {
public:
    CompositingImages(SkISize imageSize, SkISize tileSize, SkISize tileGridSize,
                      ClampingMode clampMode, TransformMode transformMode, int layerCnt)
            : fImageSize(imageSize)
            , fTileSize(tileSize)
            , fTileGridSize(tileGridSize)
            , fClampMode(clampMode)
            , fTransformMode(transformMode)
            , fLayerCnt(layerCnt) {
        fName.appendf("compositing_images_tile_size_%dx%d_grid_%dx%d_layers_%d",
                      fTileSize.fWidth, fTileSize.fHeight, fTileGridSize.fWidth,
                      fTileGridSize.fHeight, fLayerCnt);
        if (imageSize != tileSize) {
            fName.appendf("_image_%dx%d", imageSize.fWidth, imageSize.fHeight);
        }
        switch(clampMode) {
            case ClampingMode::kAlwaysFast:
                fName.append("_fast");
                break;
            case ClampingMode::kAlwaysStrict:
                fName.append("_strict");
                break;
            case ClampingMode::kChromeTiling_RowMajor:
                fName.append("_chrome");
                break;
            case ClampingMode::kChromeTiling_Optimal:
                fName.append("_chrome_optimal");
                break;
        }
        switch(transformMode) {
            case TransformMode::kNone:
                break;
            case TransformMode::kSubpixel:
                fName.append("_subpixel");
                break;
            case TransformMode::kRotated:
                fName.append("_rotated");
                break;
            case TransformMode::kPerspective:
                fName.append("_persp");
                break;
        }
    }

    bool isSuitableFor(Backend backend) override { return kGPU_Backend == backend; }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onPerCanvasPreDraw(SkCanvas* canvas) override {
        // Use image size, which may be larger than the tile size (emulating how Chrome specifies
        // their tiles).
        auto ii = SkImageInfo::Make(fImageSize.fWidth, fImageSize.fHeight, kRGBA_8888_SkColorType,
                                    kPremul_SkAlphaType, nullptr);
        SkRandom random;
        int numImages = fLayerCnt * fTileGridSize.fWidth * fTileGridSize.fHeight;
        fImages.reset(new sk_sp<SkImage>[numImages]);
        for (int i = 0; i < numImages; ++i) {
            auto surf = canvas->makeSurface(ii);
            SkColor color = random.nextU();
            surf->getCanvas()->clear(color);
            SkPaint paint;
            paint.setColor(~color);
            paint.setBlendMode(SkBlendMode::kSrc);
            // While the image may be bigger than fTileSize, prepare its content as if fTileSize
            // is what will be visible.
            surf->getCanvas()->drawRect(
                    SkRect::MakeLTRB(3, 3, fTileSize.fWidth - 3, fTileSize.fHeight - 3), paint);
            fImages[i] = surf->makeImageSnapshot();
        }
    }

    void onPerCanvasPostDraw(SkCanvas*) override { fImages.reset(); }

    void onDraw(int loops, SkCanvas* canvas) override {
        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);
        paint.setAntiAlias(true);

        canvas->save();
        canvas->concat(this->getTransform());

        for (int i = 0; i < loops; ++i) {
            for (int l = 0; l < fLayerCnt; ++l) {
                SkAutoTArray<SkCanvas::ImageSetEntry> set(
                        fTileGridSize.fWidth * fTileGridSize.fHeight);

                if (fClampMode == ClampingMode::kAlwaysFast ||
                    fClampMode == ClampingMode::kAlwaysStrict) {
                    // Simple 2D for loop, submit everything as a single batch
                    int i = 0;
                    for (int y = 0; y < fTileGridSize.fHeight; ++y) {
                        for (int x = 0; x < fTileGridSize.fWidth; ++x) {
                            set[i++] = this->getEntry(x, y, l);
                        }
                    }

                    SkCanvas::SrcRectConstraint constraint =
                            fClampMode == ClampingMode::kAlwaysFast
                                    ? SkCanvas::kFast_SrcRectConstraint
                                    : SkCanvas::kStrict_SrcRectConstraint;
                    canvas->experimental_DrawEdgeAAImageSet(set.get(), i, nullptr, nullptr, &paint,
                                                            constraint);
                } else if (fClampMode == ClampingMode::kChromeTiling_RowMajor) {
                    // Same tile order, but break batching between fast and strict sections, and
                    // adjust bottom and right tiles to encode content area distinct from src rect.
                    int i = 0;
                    for (int y = 0; y < fTileGridSize.fHeight - 1; ++y) {
                        int rowStart = i;
                        for (int x = 0; x < fTileGridSize.fWidth - 1; ++x) {
                            set[i++] = this->getEntry(x, y, l);
                        }
                        // Flush "fast" horizontal row
                        canvas->experimental_DrawEdgeAAImageSet(set.get() + rowStart,
                                fTileGridSize.fWidth - 1, nullptr, nullptr, &paint,
                                SkCanvas::kFast_SrcRectConstraint);
                        // Then flush a single adjusted entry for the right edge
                        SkPoint dstQuad[4];
                        set[i++] = this->getAdjustedEntry(fTileGridSize.fWidth - 1, y, l, dstQuad);
                        canvas->experimental_DrawEdgeAAImageSet(
                                set.get() + fTileGridSize.fWidth - 1, 1, dstQuad, nullptr, &paint,
                                SkCanvas::kStrict_SrcRectConstraint);
                    }
                    // For last row, accumulate it as a single strict batch
                    int rowStart = i;
                    SkAutoTArray<SkPoint> dstQuads(4 * (fTileGridSize.fWidth - 1));
                    for (int x = 0; x < fTileGridSize.fWidth - 1; ++x) {
                        set[i++] = this->getAdjustedEntry(x, fTileGridSize.fHeight - 1, l,
                                                          dstQuads.get() + x * 4);
                    }
                    // The corner can use conventional strict mode without geometric adjustment
                    set[i++] = this->getEntry(
                            fTileGridSize.fWidth - 1, fTileGridSize.fHeight - 1, l);
                    canvas->experimental_DrawEdgeAAImageSet(set.get() + rowStart,
                            fTileGridSize.fWidth, dstQuads.get(), nullptr, &paint,
                            SkCanvas::kStrict_SrcRectConstraint);
                } else {
                    SkASSERT(fClampMode == ClampingMode::kChromeTiling_Optimal);
                    int i = 0;
                    // Interior fast tiles
                    for (int y = 0; y < fTileGridSize.fHeight - 1; ++y) {
                        for (int x = 0; x < fTileGridSize.fWidth - 1; ++x) {
                            set[i++] = this->getEntry(x, y, l);
                        }
                    }
                    canvas->experimental_DrawEdgeAAImageSet(set.get(), i, nullptr, nullptr, &paint,
                                                            SkCanvas::kFast_SrcRectConstraint);

                    // Right edge
                    int strictStart = i;
                    SkAutoTArray<SkPoint> dstQuads(
                            4 * (fTileGridSize.fWidth + fTileGridSize.fHeight - 2));
                    for (int y = 0; y < fTileGridSize.fHeight - 1; ++y) {
                        set[i++] = this->getAdjustedEntry(fTileGridSize.fWidth - 1, y, l,
                                                          dstQuads.get() + y * 4);
                    }
                    canvas->experimental_DrawEdgeAAImageSet(set.get() + strictStart,
                            i - strictStart, dstQuads.get(), nullptr, &paint,
                            SkCanvas::kStrict_SrcRectConstraint);
                    int quadStart = 4 * (fTileGridSize.fHeight - 1);
                    strictStart = i;
                    for (int x = 0; x < fTileGridSize.fWidth - 1; ++x) {
                        set[i++] = this->getAdjustedEntry(x, fTileGridSize.fHeight - 1, l,
                                                          dstQuads.get() + quadStart + x * 4);
                    }
                    set[i++] = this->getEntry(
                            fTileGridSize.fWidth - 1, fTileGridSize.fHeight - 1, l);
                    canvas->experimental_DrawEdgeAAImageSet(set.get() + strictStart,
                            i - strictStart, dstQuads.get() + quadStart, nullptr, &paint,
                            SkCanvas::kStrict_SrcRectConstraint);
                }
            }
            // Prevent any batching between composited "frames".
            canvas->flush();
        }
        canvas->restore();
    }

private:
    SkMatrix getTransform() const {
        SkMatrix m;
        switch(fTransformMode) {
            case TransformMode::kNone:
                m.setIdentity();
                break;
            case TransformMode::kSubpixel:
                m.setTranslate(0.5f, 0.5f);
                break;
            case TransformMode::kRotated:
                m.setRotate(15.f);
                break;
            case TransformMode::kPerspective: {
                m.setIdentity();
                m.setPerspY(0.001f);
                m.setSkewX(SkIntToScalar(8) / 25);
                break;
            }
        }
        return m;
    }

    SkIPoint onGetSize() override {
        SkRect size = SkRect::MakeWH(1.25f * fTileSize.fWidth * fTileGridSize.fWidth,
                                     1.25f * fTileSize.fHeight * fTileGridSize.fHeight);
        this->getTransform().mapRect(&size);
        return SkIPoint::Make(SkScalarCeilToInt(size.width()), SkScalarCeilToInt(size.height()));
    }

    unsigned getEdgeFlags(int x, int y) const {
        unsigned flags = SkCanvas::kNone_QuadAAFlags;
        if (x == 0) {
            flags |= SkCanvas::kLeft_QuadAAFlag;
        } else if (x == fTileGridSize.fWidth - 1) {
            flags |= SkCanvas::kRight_QuadAAFlag;
        }

        if (y == 0) {
            flags |= SkCanvas::kTop_QuadAAFlag;
        } else if (y == fTileGridSize.fHeight - 1) {
            flags |= SkCanvas::kBottom_QuadAAFlag;
        }
        return flags;
    }

    SkCanvas::ImageSetEntry getEntry(int x, int y, int layer) const {
        int imageIdx =
                fTileGridSize.fWidth * fTileGridSize.fHeight * layer + fTileGridSize.fWidth * y + x;
        SkRect srcRect = SkRect::Make(fTileSize);
        // Make a non-identity transform between src and dst so bilerp isn't disabled.
        float dstWidth = srcRect.width() * 1.25f;
        float dstHeight = srcRect.height() * 1.25f;
        SkRect dstRect = SkRect::MakeXYWH(dstWidth * x, dstHeight * y, dstWidth, dstHeight);
        return SkCanvas::ImageSetEntry(fImages[imageIdx], srcRect, dstRect, 1.f,
                                       this->getEdgeFlags(x, y));
    }

    SkCanvas::ImageSetEntry getAdjustedEntry(int x, int y, int layer, SkPoint dstQuad[4]) const {
        SkASSERT(x == fTileGridSize.fWidth - 1 || y == fTileGridSize.fHeight - 1);

        SkCanvas::ImageSetEntry entry = this->getEntry(x, y, layer);
        SkRect contentRect = SkRect::Make(fImageSize);
        if (x == fTileGridSize.fWidth - 1) {
            // Right edge, so restrict horizontal content to tile width
            contentRect.fRight = fTileSize.fWidth;
        }
        if (y == fTileGridSize.fHeight - 1) {
            // Bottom edge, so restrict vertical content to tile height
            contentRect.fBottom = fTileSize.fHeight;
        }

        SkMatrix srcToDst = SkMatrix::MakeRectToRect(entry.fSrcRect, entry.fDstRect,
                                                     SkMatrix::kFill_ScaleToFit);

        // Story entry's dstRect into dstQuad, and use contentRect and contentDst as its src and dst
        entry.fDstRect.toQuad(dstQuad);
        entry.fSrcRect = contentRect;
        entry.fDstRect = srcToDst.mapRect(contentRect);
        entry.fHasClip = true;

        return entry;
    }

    std::unique_ptr<sk_sp<SkImage>[]> fImages;
    SkString fName;
    SkISize fImageSize;
    SkISize fTileSize;
    SkISize fTileGridSize;
    ClampingMode fClampMode;
    TransformMode fTransformMode;
    int fLayerCnt;

    typedef Benchmark INHERITED;
};

// Subpixel = false; all of the draw commands align with integer pixels so AA will be automatically
// turned off within the operation
DEF_BENCH(return new CompositingImages({256, 256}, {256, 256}, {8, 8}, ClampingMode::kAlwaysFast, TransformMode::kNone, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {512, 512}, {4, 4}, ClampingMode::kAlwaysFast, TransformMode::kNone, 1));
DEF_BENCH(return new CompositingImages({1024, 512}, {1024, 512}, {2, 4}, ClampingMode::kAlwaysFast, TransformMode::kNone, 1));

DEF_BENCH(return new CompositingImages({256, 256}, {256, 256}, {8, 8}, ClampingMode::kAlwaysFast, TransformMode::kNone, 4));
DEF_BENCH(return new CompositingImages({512, 512}, {512, 512}, {4, 4}, ClampingMode::kAlwaysFast, TransformMode::kNone, 4));
DEF_BENCH(return new CompositingImages({1024, 512}, {1024, 512}, {2, 4}, ClampingMode::kAlwaysFast, TransformMode::kNone, 4));

DEF_BENCH(return new CompositingImages({256, 256}, {256, 256}, {8, 8}, ClampingMode::kAlwaysFast, TransformMode::kNone, 16));
DEF_BENCH(return new CompositingImages({512, 512}, {512, 512}, {4, 4}, ClampingMode::kAlwaysFast, TransformMode::kNone, 16));
DEF_BENCH(return new CompositingImages({1024, 512}, {1024, 512}, {2, 4}, ClampingMode::kAlwaysFast, TransformMode::kNone, 16));

// Subpixel = true; force the draw commands to not align with pixels exactly so AA remains on
DEF_BENCH(return new CompositingImages({256, 256}, {256, 256}, {8, 8}, ClampingMode::kAlwaysFast, TransformMode::kSubpixel, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {512, 512}, {4, 4}, ClampingMode::kAlwaysFast, TransformMode::kSubpixel, 1));
DEF_BENCH(return new CompositingImages({1024, 512}, {1024, 512}, {2, 4}, ClampingMode::kAlwaysFast, TransformMode::kSubpixel, 1));

DEF_BENCH(return new CompositingImages({256, 256}, {256, 256}, {8, 8}, ClampingMode::kAlwaysFast, TransformMode::kSubpixel, 4));
DEF_BENCH(return new CompositingImages({512, 512}, {512, 512}, {4, 4}, ClampingMode::kAlwaysFast, TransformMode::kSubpixel, 4));
DEF_BENCH(return new CompositingImages({1024, 512}, {1024, 512}, {2, 4}, ClampingMode::kAlwaysFast, TransformMode::kSubpixel, 4));

DEF_BENCH(return new CompositingImages({256, 256}, {256, 256}, {8, 8}, ClampingMode::kAlwaysFast, TransformMode::kSubpixel, 16));
DEF_BENCH(return new CompositingImages({512, 512}, {512, 512}, {4, 4}, ClampingMode::kAlwaysFast, TransformMode::kSubpixel, 16));
DEF_BENCH(return new CompositingImages({1024, 512}, {1024, 512}, {2, 4}, ClampingMode::kAlwaysFast, TransformMode::kSubpixel, 16));

// Test different tiling scenarios inspired by Chrome's compositor
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kAlwaysFast, TransformMode::kNone, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kAlwaysStrict, TransformMode::kNone, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kChromeTiling_RowMajor, TransformMode::kNone, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kChromeTiling_Optimal, TransformMode::kNone, 1));

DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kAlwaysFast, TransformMode::kSubpixel, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kAlwaysStrict, TransformMode::kSubpixel, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kChromeTiling_RowMajor, TransformMode::kSubpixel, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kChromeTiling_Optimal, TransformMode::kSubpixel, 1));

DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kAlwaysFast, TransformMode::kRotated, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kAlwaysStrict, TransformMode::kRotated, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kChromeTiling_RowMajor, TransformMode::kRotated, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kChromeTiling_Optimal, TransformMode::kRotated, 1));

DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kAlwaysFast, TransformMode::kPerspective, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kAlwaysStrict, TransformMode::kPerspective, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kChromeTiling_RowMajor, TransformMode::kPerspective, 1));
DEF_BENCH(return new CompositingImages({512, 512}, {380, 380}, {5, 5}, ClampingMode::kChromeTiling_Optimal, TransformMode::kPerspective, 1));
