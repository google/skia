/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkAnimTimer.h"
#include "SkColorFilter.h"
#include "SkFont.h"
#include "SkImage.h"
#include "SkPath.h"
#include "SkSurface.h"

namespace skiagm {

class ShapeRenderer : public SkRefCntBase {
public:
    static constexpr SkScalar kTileWidth = 20.f;
    static constexpr SkScalar kTileHeight = 20.f;

    virtual ~ShapeRenderer() {}

    // Draw the shape, limited to kTileWidth x kTileHeight. It must apply the local subpixel (tx,
    // ty) translation and rotation by angle. Prior to these transform adjustments, the SkCanvas
    // will only have pixel aligned translations (these are separated to make super-sampling
    // renderers easier).
    virtual void draw(SkCanvas* canvas, SkPaint* paint,
                      SkScalar tx, SkScalar ty, SkScalar angle) = 0;

    virtual SkString name() = 0;

    void applyLocalTransform(SkCanvas* canvas, SkScalar tx, SkScalar ty, SkScalar angle) {
        canvas->translate(tx, ty);
        canvas->rotate(angle, kTileWidth / 2.f, kTileHeight / 2.f);
    }
};

class RectRenderer : public ShapeRenderer {
public:
    static sk_sp<ShapeRenderer> Make() {
        return sk_sp<ShapeRenderer>(new RectRenderer());
    }

    SkString name() override { return SkString("rect"); }

    void draw(SkCanvas* canvas, SkPaint* paint, SkScalar tx, SkScalar ty, SkScalar angle) override {
        SkScalar width = paint->getStrokeWidth();
        paint->setStyle(SkPaint::kFill_Style);

        this->applyLocalTransform(canvas, tx, ty, angle);
        canvas->drawRect(SkRect::MakeLTRB(kTileWidth / 2.f - width / 2.f, 2.f,
                                          kTileWidth / 2.f + width / 2.f, kTileHeight - 2.f),
                         *paint);
    }

private:
    RectRenderer() {}

    typedef ShapeRenderer INHERITED;
};

class PathRenderer : public ShapeRenderer {
public:
    static sk_sp<ShapeRenderer> MakeLine(bool hairline = false) {
        return MakeCurve(0.f, hairline);
    }

    static sk_sp<ShapeRenderer> MakeCurve(SkScalar depth, bool hairline = false) {
        return sk_sp<ShapeRenderer>(new PathRenderer(depth, hairline));
    }

    SkString name() override {
        SkString name;
        if (fHairline) {
            name.append("hairline");
            if (fDepth > 0.f) {
                name.appendf("-curve-%.2f", fDepth);
            }
        } else if (fDepth > 0.f) {
            name.appendf("curve-%.2f", fDepth);
        } else {
            name.append("line");
        }

        return name;
    }

    void draw(SkCanvas* canvas, SkPaint* paint, SkScalar tx, SkScalar ty, SkScalar angle) override {
        SkPath path;
        path.moveTo(kTileWidth / 2.f, 2.f);

        if (fDepth > 0.f) {
            path.quadTo(kTileWidth / 2.f + fDepth, kTileHeight / 2.f,
                        kTileWidth / 2.f, kTileHeight - 2.f);
        } else {
            path.lineTo(kTileWidth / 2.f, kTileHeight - 2.f);
        }

        if (fHairline) {
            // Fake thinner hairlines by making it transparent, conflating coverage and alpha
            SkColor4f color = paint->getColor4f();
            SkScalar width = paint->getStrokeWidth();
            if (width > 1.f) {
                // Can't emulate width larger than a pixel
                return;
            }
            paint->setColor4f({color.fR, color.fG, color.fB, width}, nullptr);
            paint->setStrokeWidth(0.f);
        }

        // Adding round caps forces Ganesh to use the path renderer for lines instead of converting
        // them to rectangles (which are already explicitly tested). However, when not curved, the
        // GrShape will still find a way to turn it into a rrect draw so it doesn't hit the
        // path renderer in that condition.
        paint->setStrokeCap(SkPaint::kRound_Cap);
        paint->setStyle(SkPaint::kStroke_Style);

        this->applyLocalTransform(canvas, tx, ty, angle);
        canvas->drawPath(path, *paint);
    }

private:
    SkScalar fDepth; // 0.f to make a line, otherwise outset of curve from end points
    bool fHairline;

    PathRenderer(SkScalar depth, bool hairline)
            : fDepth(depth)
            , fHairline(hairline) {}

    typedef ShapeRenderer INHERITED;
};

class OffscreenShapeRenderer : public ShapeRenderer {
public:
    ~OffscreenShapeRenderer() override = default;

    static sk_sp<OffscreenShapeRenderer> Make(sk_sp<ShapeRenderer> renderer, int supersample,
                                              bool forceRaster = false) {
        SkASSERT(supersample > 0);
        return sk_sp<OffscreenShapeRenderer>(new OffscreenShapeRenderer(std::move(renderer),
                                                                        supersample, forceRaster));
    }

    SkString name() override {
        SkString name = fRenderer->name();
        if (fSupersampleFactor != 1) {
            name.prependf("%dx-", fSupersampleFactor * fSupersampleFactor);
        }
        return name;
    }

    void draw(SkCanvas* canvas, SkPaint* paint, SkScalar tx, SkScalar ty, SkScalar angle) override {
        // Subpixel translation+angle are applied in the offscreen buffer
        sk_sp<SkImage> image = this->drawToBuffer(canvas, paint, tx, ty, angle);

        // Use highest quality filter so that the supersampled images are downsampled appropriately
        SkPaint blit;
        blit.setFilterQuality(kHigh_SkFilterQuality);
        this->redrawFiltered(canvas, std::move(image), &blit);
    }

    // Redraw the results of draw() at 4X scale
    void redrawAt4X(SkCanvas* canvas) {
        SkASSERT(fLastRendered);

        // Use no filtering so that when scaled up, we can see the original pixelization
        SkPaint blit;
        blit.setFilterQuality(kNone_SkFilterQuality);

        canvas->scale(4.0f, 4.0f);
        this->redrawFiltered(canvas, fLastRendered, &blit);
    }

    // Redraws the results of draw() at original scale, but transformed to be opaque black anywhere
    // the original rendering touched a pixel.
    void redrawTouchedPixels(SkCanvas* canvas) {
        // Makes anything that's > 1/255 alpha fully opaque and sets color to medium green.
        static constexpr SkScalar kFilter[] = {
            0.f, 0.f, 0.f, 0.f, 16.f,
            0.f, 0.f, 0.f, 0.f, 200.f,
            0.f, 0.f, 0.f, 0.f, 16.f,
            0.f, 0.f, 0.f, 255.f, 0.f
        };

        SkPaint blit;
        blit.setFilterQuality(kHigh_SkFilterQuality); // still downsampling in this case
        blit.setColorFilter(SkColorFilter::MakeMatrixFilterRowMajor255(kFilter));
        this->redrawFiltered(canvas, fLastRendered, &blit);
    }

private:
    bool                 fForceRasterBackend;
    sk_sp<SkImage>       fLastRendered;
    sk_sp<ShapeRenderer> fRenderer;
    int                  fSupersampleFactor;

    OffscreenShapeRenderer(sk_sp<ShapeRenderer> renderer, int supersample, bool forceRaster)
            : fForceRasterBackend(forceRaster)
            , fLastRendered(nullptr)
            , fRenderer(std::move(renderer))
            , fSupersampleFactor(supersample) { }

    void redrawFiltered(SkCanvas* canvas, sk_sp<SkImage> image, SkPaint* paint) {
        // We just need to draw the resulting bitmap scaled by 1/ss into the original canvas
        canvas->drawImageRect(std::move(image), SkRect::MakeWH(kTileWidth, kTileHeight), paint);
    }

    sk_sp<SkImage> drawToBuffer(SkCanvas* canvas, SkPaint* paint, SkScalar tx, SkScalar ty,
                                SkScalar angle) {
        auto info = SkImageInfo::Make(fSupersampleFactor * kTileWidth,
                                      fSupersampleFactor * kTileHeight,
                                      kRGBA_8888_SkColorType, kPremul_SkAlphaType);
        auto surface = fForceRasterBackend ? SkSurface::MakeRaster(info)
                                           : canvas->makeSurface(info);

        surface->getCanvas()->save();
        // Make fully transparent so it is easy to determine pixels that are touched by partial cov.
        surface->getCanvas()->clear(SK_ColorTRANSPARENT);
        // Set up scaling to fit supersampling amount
        surface->getCanvas()->scale(fSupersampleFactor, fSupersampleFactor);
        fRenderer->draw(surface->getCanvas(), paint, tx, ty, angle);
        surface->getCanvas()->restore();

        // Save image so it can be drawn zoomed in or to visualize touched pixels; only valid until
        // the next call to draw()
        fLastRendered = surface->makeImageSnapshot();
        return fLastRendered;
    }

    typedef ShapeRenderer INHERITED;
};

class ThinAAGM : public GM {
public:
    ThinAAGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        return SkString("thinaa");
    }

    SkISize onISize() override {
        return SkISize::Make(240, 320); // FIXME wrong
    }

    void onOnceBeforeDraw() override {
        // Setup all base renderers
        fShapes.push_back(RectRenderer::Make());
        fShapes.push_back(PathRenderer::MakeLine());
        fShapes.push_back(PathRenderer::MakeCurve(2.f)); // Shallow curve
        fShapes.push_back(PathRenderer::MakeCurve(8.f)); // Deep curve

        // Include hairlines
        fShapes.push_back(PathRenderer::MakeLine(true)); // Hairline
        fShapes.push_back(PathRenderer::MakeCurve(8.f, true)); // Hairline curve

        for (int i = 0; i < fShapes.count(); ++i) {
            fNative.push_back(OffscreenShapeRenderer::Make(fShapes[i], 1));
            fRaster.push_back(OffscreenShapeRenderer::Make(fShapes[i], 1, /* raster */ true));
            fSS4.push_back(OffscreenShapeRenderer::Make(fShapes[i], 2)); // 2x2 -> 4 samples
            fSS16.push_back(OffscreenShapeRenderer::Make(fShapes[i], 4)); // 4x4 -> 16 samples
        }

        fSubpixelX = 0.f;
        fSubpixelY = 0.f;
        fAngle = 0.f;
        fLastAlpha = 0.f;

        // Don't animate in the beginning
        fAnimTranslate = false;
        fAnimRotate = false;
    }

    void onDraw(SkCanvas* canvas) override {
        // Move away from screen edge and add instructions
        SkPaint text;
        SkFont font(nullptr, 12);
        canvas->translate(60.f, 20.f);
        canvas->drawString("Each 2x2 grid is the same shape and stroke width, but rendered with "
                           "current backend (TL), raster backend (TR), 4x supersampling (BL), "
                           "and 16x supersampling (BR).", 0, 0, font, text);
        canvas->drawString("Supersamples are drawn with current backend. "
                           "The hairline variants abuse alpha to mimic the thinner stroke widths.",
                           0, 12, font, text);
        canvas->drawString("'r' animates rotation. 't' animates translation. 'y' resets to 90, "
                           "'u' to 0 degrees, and 'space' increments by 30 degrees.",
                           0, 24, font, text);
        canvas->translate(0.f, 100.f);

        // Draw with surface matching current viewer surface type in top left of 2x2 grid
        this->drawShapes(canvas, 0, 0, fNative);

        // Draw with forced raster backend so it's easy to compare side-by-side in top right
        this->drawShapes(canvas, 1, 0, fRaster);

        // Draw at 4x supersampling in bottom left
        this->drawShapes(canvas, 0, 1, fSS4);

        // And lastly 16x supersampling in bottom right
        this->drawShapes(canvas, 1, 1, fSS16);
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        SkScalar period = 5.f; // 1 second per side + 1 second for rotate

        if (!fAnimRotate && !fAnimTranslate) {
            // Do nothing
            return false;
        } else if (!fAnimRotate) {
            period = 4.f; // Drop rotation
        } else if (!fAnimTranslate) {
            period = 1.f;
        }

        SkScalar t = timer.scaled(1.f, period);
        SkScalar alpha = SkScalarFraction(t);
        // bool looped = alpha < fLastAlpha;

        if (t > 4.f || !fAnimTranslate) {
            // Adjust angle, 10 degrees per second
            SkScalar da;
            if (alpha < fLastAlpha) {
                // Looped back around
                da = (1 - fLastAlpha) + alpha;
            } else {
                da = alpha - fLastAlpha;
            }

            // SkDebugf("alpha: %.3f, angle: %.3f, looped: %d\n", alpha, fAngle, looped);
            // SkDebugf("new angle: %.3f, wrapped: %.3f\n", fAngle + alpha * 10.f + (looped ? 10.f : 0.f),
                // SkScalarMod(fAngle + alpha * 10.f + (looped ? 10.f : 0.f), 360.f));
            fAngle = SkScalarMod(fAngle + da * 10.f, 360.f);
        } else if (t > 3.f) {
            // Move Y towards top at rate of 4px per second
            fSubpixelY = (1 - alpha) * 4.f;
        } else if (t > 2.f) {
            // Move X towards left at rate of 4px per second
            fSubpixelX = (1 - alpha) * 4.f;
        } else if (t > 1.f) {
            // Move Y towards bottom at rate of 4px per second
            fSubpixelY = alpha * 4.f;
        } else {
            // Move X towards right at rate of 4px per second
            fSubpixelX = alpha * 4.f;
        }
        fLastAlpha = alpha;

        return true;
    }

    bool onHandleKey(SkUnichar uni) override {
        switch (uni) {
            case 't': fAnimTranslate = !fAnimTranslate; return true;
            case 'r': fAnimRotate = !fAnimRotate; return true;
            case 'u': fAngle = 0.f; return true;
            case 'y': fAngle = 90.f; return true;
            case ' ': fAngle = SkScalarMod(fAngle + 30.f, 360.f); return true;
            default: break;
        }
        return false;
    }

private:
    // Base renderers that get wrapped on the offscreen renderers so that they can be transformed
    // for visualization, or supersampled.
    SkTArray<sk_sp<ShapeRenderer>> fShapes;

    SkTArray<sk_sp<OffscreenShapeRenderer>> fNative;
    SkTArray<sk_sp<OffscreenShapeRenderer>> fRaster;
    SkTArray<sk_sp<OffscreenShapeRenderer>> fSS4;
    SkTArray<sk_sp<OffscreenShapeRenderer>> fSS16;

    // Animated properties to stress the AA algorithms
    SkScalar fSubpixelX;
    SkScalar fSubpixelY;
    SkScalar fAngle;
    SkScalar fLastAlpha;

    bool     fAnimRotate;
    bool     fAnimTranslate;

    void drawShapes(SkCanvas* canvas, int gridX, int gridY,
                    SkTArray<sk_sp<OffscreenShapeRenderer>> shapes) {
        SkAutoCanvasRestore autoRestore(canvas, /* save */ true);

        for (int i = 0; i < shapes.count(); ++i) {
            this->drawShape(canvas, gridX, gridY, shapes[i].get(), i == 0);
        }
    }

    void drawShape(SkCanvas* canvas, int gridX, int gridY,
                   OffscreenShapeRenderer* shape, bool drawDetailedLabels) {
        SkASSERT(gridX >= 0 && gridX < 2);
        SkASSERT(gridY >= 0 && gridY < 2);

        static constexpr SkScalar kStrokeWidths[] = { 1.0f, .5f, .25f, 0.125f };

        static constexpr SkScalar kGridWidth = 2 * ShapeRenderer::kTileWidth + 4.f;
        static constexpr SkScalar kZoomGridWidth = 8 * ShapeRenderer::kTileWidth + 4.f;

        static constexpr SkScalar kCoverageOffsetX = 0.f;
        static constexpr SkScalar kCoverageOffsetY = 2 * ShapeRenderer::kTileHeight + 2.0f;
        static constexpr SkScalar kZoomOffsetX = SK_ARRAY_COUNT(kStrokeWidths) * kGridWidth;
        static constexpr SkScalar kZoomOffsetY = -2 * ShapeRenderer::kTileHeight;

        bool drawShapeLabel = gridX == 0 && gridY == 0;
        drawDetailedLabels &= drawShapeLabel;

        // Labeling per shape and detailed labeling that isn't per-stroke
        canvas->save();
        SkPaint text;
        SkFont font(nullptr, 12);

        if (drawShapeLabel) {
            SkString name = shape->name();
            SkScalar centering = name.size() * 3.f; // ad-hoc

            canvas->save();
            canvas->translate(-35.f, 2 * ShapeRenderer::kTileHeight + 2.f + centering);
            canvas->rotate(-90.f);
            canvas->drawString(shape->name(), 0.f, 0.f, font, text);
            canvas->restore();
        }
        if (drawDetailedLabels) {
            canvas->drawString("Out", -24.f, ShapeRenderer::kTileHeight + 2.f, font, text);
            canvas->drawString("Hit", -24.f, 3 * ShapeRenderer::kTileHeight + 6.f, font, text);
            canvas->drawString("Stroke Widths", 0.f, -50.f, font, text);
            canvas->drawString("Zoomed (4X)", kZoomOffsetX, -50.f, font, text);
        }
        canvas->restore();

        SkPaint outline;
        outline.setColor(SK_ColorBLACK);
        outline.setAntiAlias(false);
        outline.setStyle(SkPaint::kStroke_Style);

        for (size_t i = 0; i < SK_ARRAY_COUNT(kStrokeWidths); ++i) {
            canvas->save();

            SkScalar tx = gridX * ShapeRenderer::kTileWidth;
            SkScalar ox = i * kGridWidth;
            // Zoomed offset that keeps 4px gap
            SkScalar zx = i * kZoomGridWidth;
            SkScalar ty = gridY * ShapeRenderer::kTileHeight;

            if (drawDetailedLabels) {
                canvas->save();
                canvas->translate(ox + ShapeRenderer::kTileWidth / 2.f, -12.f);
                canvas->rotate(-45.f);
                SkString w;
                w.appendf("%.3f", kStrokeWidths[i]);
                canvas->drawString(w, 0.f, 0.f, font, text);
                canvas->restore();
            }

            // Draw an outline around the original, coverage, and zoomed renderings
            canvas->drawRect(SkRect::MakeXYWH(tx + ox, ty - 2.f, ShapeRenderer::kTileWidth,
                    ShapeRenderer::kTileHeight), outline);
            canvas->drawRect(SkRect::MakeXYWH(kCoverageOffsetX + tx + ox, kCoverageOffsetY + ty,
                    ShapeRenderer::kTileWidth, ShapeRenderer::kTileHeight), outline);
            canvas->drawRect(SkRect::MakeXYWH(kZoomOffsetX + 4 * tx + zx, kZoomOffsetY + 4 * ty,
                    4 * ShapeRenderer::kTileWidth, 4 * ShapeRenderer::kTileHeight), outline);

            // Next draw the shape directly, which will generate a saved image of the correct width
            canvas->save();
            canvas->translate(tx + ox, ty - 2.f);

            SkPaint paint;
            paint.setColor4f({0.f, 0.f, 0.f, 1.f}, nullptr);
            paint.setAntiAlias(true);
            paint.setStrokeWidth(kStrokeWidths[i]);
            shape->draw(canvas, &paint, fSubpixelX, fSubpixelY, fAngle);
            canvas->restore();

            // Now redraw it into the coverage location
            canvas->save();
            canvas->translate(kCoverageOffsetX + tx + ox, kCoverageOffsetY + ty);
            shape->redrawTouchedPixels(canvas);
            canvas->restore();

            // And lastly, draw it at 4X zoom off to the right
            canvas->save();
            canvas->translate(kZoomOffsetX + 4 * tx + zx, kZoomOffsetY + 4 * ty);
            shape->redrawAt4X(canvas);
            canvas->restore();

            canvas->restore();
        }

        // Shift the canvas translation down by 8 * kTH + padding for the next set of shapes
        // (8 since that's the full height of the 4X 2x2 grid)
        canvas->translate(0.f, 8.f * ShapeRenderer::kTileHeight + 20.f);
    }

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ThinAAGM; )

}
