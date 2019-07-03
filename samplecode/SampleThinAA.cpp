/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "samplecode/Sample.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkFont.h"
#include "include/core/SkImage.h"
#include "include/core/SkPath.h"
#include "include/core/SkSurface.h"
#include "tools/timer/AnimTimer.h"

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

    virtual sk_sp<ShapeRenderer> toHairline() = 0;

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

    sk_sp<ShapeRenderer> toHairline() override {
        // Not really available but can't return nullptr
        return Make();
    }

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

    static sk_sp<ShapeRenderer> MakeLines(SkScalar depth, bool hairline = false) {
        return MakeCurve(-depth, hairline);
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
        } else if (fDepth < 0.f) {
            name.appendf("line-%.2f", -fDepth);
        } else {
            name.append("line");
        }

        return name;
    }

    sk_sp<ShapeRenderer> toHairline() override {
        return sk_sp<ShapeRenderer>(new PathRenderer(fDepth, true));
    }

    void draw(SkCanvas* canvas, SkPaint* paint, SkScalar tx, SkScalar ty, SkScalar angle) override {
        SkPath path;
        path.moveTo(kTileWidth / 2.f, 2.f);

        if (fDepth > 0.f) {
            path.quadTo(kTileWidth / 2.f + fDepth, kTileHeight / 2.f,
                        kTileWidth / 2.f, kTileHeight - 2.f);
        } else {
            if (fDepth < 0.f) {
                path.lineTo(kTileWidth / 2.f + fDepth, kTileHeight / 2.f);
            }
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
        paint->setStrokeJoin(SkPaint::kMiter_Join);
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

    sk_sp<ShapeRenderer> toHairline() override {
        return Make(fRenderer->toHairline(), fSupersampleFactor, fForceRasterBackend);
    }

    void draw(SkCanvas* canvas, SkPaint* paint, SkScalar tx, SkScalar ty, SkScalar angle) override {
        // Subpixel translation+angle are applied in the offscreen buffer
        this->prepareBuffer(canvas, paint, tx, ty, angle);
        this->redraw(canvas);
    }

    // Exposed so that it's easy to fill the offscreen buffer, then draw zooms/filters of it before
    // drawing the original scale back into the canvas.
    void prepareBuffer(SkCanvas* canvas, SkPaint* paint, SkScalar tx, SkScalar ty, SkScalar angle) {
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
    }

    void redraw(SkCanvas* canvas, SkScalar scale = 1.f, bool debugMode = false) {
        SkASSERT(fLastRendered);
        // Use medium quality filter to get mipmaps when drawing smaller, or use nearest filtering
        // when upscaling
        SkPaint blit;
        blit.setFilterQuality(scale > 1.f ? kNone_SkFilterQuality : kMedium_SkFilterQuality);
        if (debugMode) {
            // Makes anything that's > 1/255 alpha fully opaque and sets color to medium green.
            static constexpr float kFilter[] = {
                0.f, 0.f, 0.f, 0.f, 16.f/255,
                0.f, 0.f, 0.f, 0.f, 200.f/255,
                0.f, 0.f, 0.f, 0.f, 16.f/255,
                0.f, 0.f, 0.f, 255.f, 0.f
            };

            blit.setColorFilter(SkColorFilters::Matrix(kFilter));
        }

        canvas->scale(scale, scale);
        canvas->drawImageRect(fLastRendered, SkRect::MakeWH(kTileWidth, kTileHeight), &blit);
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

    typedef ShapeRenderer INHERITED;
};

class ThinAASample : public Sample {
public:
    ThinAASample() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    void onOnceBeforeDraw() override {
        // Setup all base renderers
        fShapes.push_back(RectRenderer::Make());
        fShapes.push_back(PathRenderer::MakeLine());
        fShapes.push_back(PathRenderer::MakeLines(4.f)); // 2 segments
        fShapes.push_back(PathRenderer::MakeCurve(2.f)); // Shallow curve
        fShapes.push_back(PathRenderer::MakeCurve(8.f)); // Deep curve

        for (int i = 0; i < fShapes.count(); ++i) {
            fNative.push_back(OffscreenShapeRenderer::Make(fShapes[i], 1));
            fRaster.push_back(OffscreenShapeRenderer::Make(fShapes[i], 1, /* raster */ true));
            fSS4.push_back(OffscreenShapeRenderer::Make(fShapes[i], 4)); // 4x4 -> 16 samples
            fSS16.push_back(OffscreenShapeRenderer::Make(fShapes[i], 8)); // 8x8 -> 64 samples

            fHairline.push_back(OffscreenShapeRenderer::Make(fRaster[i]->toHairline(), 1));
        }

        // Start it at something subpixel
        fStrokeWidth = 0.5f;

        fSubpixelX = 0.f;
        fSubpixelY = 0.f;
        fAngle = 0.f;

        fCurrentStage = AnimStage::kMoveLeft;
        fLastFrameTime = -1.f;

        // Don't animate in the beginning
        fAnimTranslate = false;
        fAnimRotate = false;
    }

    void onDrawContent(SkCanvas* canvas) override {
        // Move away from screen edge and add instructions
        SkPaint text;
        SkFont font(nullptr, 12);
        canvas->translate(60.f, 20.f);
        canvas->drawString("Each row features a rendering command under different AA strategies. "
                           "Native refers to the current backend of the viewer, e.g. OpenGL.",
                           0, 0, font, text);

        canvas->drawString(SkStringPrintf("Stroke width: %.2f ('-' to decrease, '=' to increase)",
                fStrokeWidth), 0, 24, font, text);
        canvas->drawString(SkStringPrintf("Rotation: %.3f ('r' to animate, 'y' sets to 90, 'u' sets"
                " to 0, 'space' adds 15)", fAngle), 0, 36, font, text);
        canvas->drawString(SkStringPrintf("Translation: %.3f, %.3f ('t' to animate)",
                fSubpixelX, fSubpixelY), 0, 48, font, text);

        canvas->translate(0.f, 100.f);

        // Draw with surface matching current viewer surface type
        this->drawShapes(canvas, "Native", 0, fNative);

        // Draw with forced raster backend so it's easy to compare side-by-side
        this->drawShapes(canvas, "Raster", 1, fRaster);

        // Draw paths as hairlines + alpha hack
        this->drawShapes(canvas, "Hairline", 2, fHairline);

        // Draw at 4x supersampling in bottom left
        this->drawShapes(canvas, "SSx16", 3, fSS4);

        // And lastly 16x supersampling in bottom right
        this->drawShapes(canvas, "SSx64", 4, fSS16);
    }

    bool onAnimate(const AnimTimer& timer) override {
        SkScalar t = timer.secs();
        SkScalar dt = fLastFrameTime < 0.f ? 0.f : t - fLastFrameTime;
        fLastFrameTime = t;

        if (!fAnimRotate && !fAnimTranslate) {
            // Keep returning true so that the last frame time is tracked
            fLastFrameTime = -1.f;
            return false;
        }

        switch(fCurrentStage) {
            case AnimStage::kMoveLeft:
                fSubpixelX += 2.f * dt;
                if (fSubpixelX >= 1.f) {
                    fSubpixelX = 1.f;
                    fCurrentStage = AnimStage::kMoveDown;
                }
                break;
            case AnimStage::kMoveDown:
                fSubpixelY += 2.f * dt;
                if (fSubpixelY >= 1.f) {
                    fSubpixelY = 1.f;
                    fCurrentStage = AnimStage::kMoveRight;
                }
                break;
            case AnimStage::kMoveRight:
                fSubpixelX -= 2.f * dt;
                if (fSubpixelX <= -1.f) {
                    fSubpixelX = -1.f;
                    fCurrentStage = AnimStage::kMoveUp;
                }
                break;
            case AnimStage::kMoveUp:
                fSubpixelY -= 2.f * dt;
                if (fSubpixelY <= -1.f) {
                    fSubpixelY = -1.f;
                    fCurrentStage = fAnimRotate ? AnimStage::kRotate : AnimStage::kMoveLeft;
                }
                break;
            case AnimStage::kRotate: {
                SkScalar newAngle = fAngle + dt * 15.f;
                bool completed = SkScalarMod(newAngle, 15.f) < SkScalarMod(fAngle, 15.f);
                fAngle = SkScalarMod(newAngle, 360.f);
                if (completed) {
                    // Make sure we're on a 15 degree boundary
                    fAngle = 15.f * SkScalarRoundToScalar(fAngle / 15.f);
                    if (fAnimTranslate) {
                        fCurrentStage = this->getTranslationStage();
                    }
                }
            } break;
        }

        return true;
    }

    SkString name() override { return SkString("Thin-AA"); }

    bool onQuery(Sample::Event* evt) override {
        SkUnichar key;
        if (Sample::CharQ(*evt, &key)) {
            switch(key) {
                case 't':
                    // Toggle translation animation.
                    fAnimTranslate = !fAnimTranslate;
                    if (!fAnimTranslate && fAnimRotate && fCurrentStage != AnimStage::kRotate) {
                        // Turned off an active translation so go to rotating
                        fCurrentStage = AnimStage::kRotate;
                    } else if (fAnimTranslate && !fAnimRotate &&
                               fCurrentStage == AnimStage::kRotate) {
                        // Turned on translation, rotation had been paused too, so reset the stage
                        fCurrentStage = this->getTranslationStage();
                    }
                    return true;
                case 'r':
                    // Toggle rotation animation.
                    fAnimRotate = !fAnimRotate;
                    if (!fAnimRotate && fAnimTranslate && fCurrentStage == AnimStage::kRotate) {
                        // Turned off an active rotation so go back to translation
                        fCurrentStage = this->getTranslationStage();
                    } else if (fAnimRotate && !fAnimTranslate &&
                               fCurrentStage != AnimStage::kRotate) {
                        // Turned on rotation, translation had been paused too, so reset to rotate
                        fCurrentStage = AnimStage::kRotate;
                    }
                    return true;
                case 'u': fAngle = 0.f; return true;
                case 'y': fAngle = 90.f; return true;
                case ' ': fAngle = SkScalarMod(fAngle + 15.f, 360.f); return true;
                case '-': fStrokeWidth = SkMaxScalar(0.1f, fStrokeWidth - 0.05f); return true;
                case '=': fStrokeWidth = SkMinScalar(1.f, fStrokeWidth + 0.05f); return true;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

private:
    // Base renderers that get wrapped on the offscreen renderers so that they can be transformed
    // for visualization, or supersampled.
    SkTArray<sk_sp<ShapeRenderer>> fShapes;

    SkTArray<sk_sp<OffscreenShapeRenderer>> fNative;
    SkTArray<sk_sp<OffscreenShapeRenderer>> fRaster;
    SkTArray<sk_sp<OffscreenShapeRenderer>> fHairline;
    SkTArray<sk_sp<OffscreenShapeRenderer>> fSS4;
    SkTArray<sk_sp<OffscreenShapeRenderer>> fSS16;

    SkScalar fStrokeWidth;

    // Animated properties to stress the AA algorithms
    enum class AnimStage {
        kMoveRight, kMoveDown, kMoveLeft, kMoveUp, kRotate
    } fCurrentStage;
    SkScalar fLastFrameTime;
    bool     fAnimRotate;
    bool     fAnimTranslate;

    // Current frame's animation state
    SkScalar fSubpixelX;
    SkScalar fSubpixelY;
    SkScalar fAngle;

    AnimStage getTranslationStage() {
        // For paused translations (i.e. fAnimTranslate toggled while translating), the current
        // stage moves to kRotate, but when restarting the translation animation, we want to
        // go back to where we were without losing any progress.
        if (fSubpixelX > -1.f) {
            if (fSubpixelX >= 1.f) {
                // Can only be moving down on right edge, given our transition states
                return AnimStage::kMoveDown;
            } else if (fSubpixelY > 0.f) {
                // Can only be moving right along top edge
                return AnimStage::kMoveRight;
            } else {
                // Must be moving left along bottom edge
                return AnimStage::kMoveLeft;
            }
        } else {
            // Moving up along the left edge, or is at the very top so start moving left
            return fSubpixelY > -1.f ? AnimStage::kMoveUp : AnimStage::kMoveLeft;
        }
    }

    void drawShapes(SkCanvas* canvas, const char* name, int gridX,
                    SkTArray<sk_sp<OffscreenShapeRenderer>> shapes) {
        SkAutoCanvasRestore autoRestore(canvas, /* save */ true);

        for (int i = 0; i < shapes.count(); ++i) {
            this->drawShape(canvas, name, gridX, shapes[i].get(), i == 0);
            // drawShape positions the canvas properly for the next iteration
        }
    }

    void drawShape(SkCanvas* canvas, const char* name, int gridX,
                   OffscreenShapeRenderer* shape, bool drawNameLabels) {
        static constexpr SkScalar kZoomGridWidth = 8 * ShapeRenderer::kTileWidth + 8.f;
        static constexpr SkRect kTile = SkRect::MakeWH(ShapeRenderer::kTileWidth,
                                                       ShapeRenderer::kTileHeight);
        static constexpr SkRect kZoomTile = SkRect::MakeWH(8 * ShapeRenderer::kTileWidth,
                                                           8 * ShapeRenderer::kTileHeight);

        // Labeling per shape and detailed labeling that isn't per-stroke
        canvas->save();
        SkPaint text;
        SkFont font(nullptr, 12);

        if (gridX == 0) {
            SkString name = shape->name();
            SkScalar centering = name.size() * 4.f; // ad-hoc

            canvas->save();
            canvas->translate(-10.f, 4 * ShapeRenderer::kTileHeight + centering);
            canvas->rotate(-90.f);
            canvas->drawString(shape->name(), 0.f, 0.f, font, text);
            canvas->restore();
        }
        if (drawNameLabels) {
            canvas->drawString(name, gridX * kZoomGridWidth, -10.f, font, text);
        }
        canvas->restore();

        // Paints for outlines and actual shapes
        SkPaint outline;
        outline.setStyle(SkPaint::kStroke_Style);
        SkPaint clear;
        clear.setColor(SK_ColorWHITE);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(fStrokeWidth);

        // Generate a saved image of the correct stroke width, but don't put it into the canvas
        // yet since we want to draw the "original" size on top of the zoomed in version
        shape->prepareBuffer(canvas, &paint, fSubpixelX, fSubpixelY, fAngle);

        // Draw it at 8X zoom
        SkScalar x = gridX * kZoomGridWidth;

        canvas->save();
        canvas->translate(x, 0.f);
        canvas->drawRect(kZoomTile, outline);
        shape->redraw(canvas, 8.0f);
        canvas->restore();

        // Draw the original
        canvas->save();
        canvas->translate(x + 4.f, 4.f);
        canvas->drawRect(kTile, clear);
        canvas->drawRect(kTile, outline);
        shape->redraw(canvas, 1.f);
        canvas->restore();

        // Now redraw it into the coverage location (just to the right of the original scale)
        canvas->save();
        canvas->translate(x + ShapeRenderer::kTileWidth + 8.f, 4.f);
        canvas->drawRect(kTile, clear);
        canvas->drawRect(kTile, outline);
        shape->redraw(canvas, 1.f, /* debug */ true);
        canvas->restore();

        // Lastly, shift the canvas translation down by 8 * kTH + padding for the next set of shapes
        canvas->translate(0.f, 8.f * ShapeRenderer::kTileHeight + 20.f);
    }

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new ThinAASample; )

}
