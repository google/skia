/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"

#include "SkAnimTimer.h"
#include "SkPath.h"

namespace skiagm {

class ShapeRenderer {
public:
    static constexpr SkScalar kTileWidth = 20.f;
    static constexpr SkScalar kTileHeight = 20.f;

    virtual ~ShapeRenderer() {}

    // Draw the shape, limited to kTileWidth x kTileHeight. When applicable (i.e. not hairline)
    // the shape should be 'width' wide, and it must apply the local subpixel (tx, ty) translation
    // and rotation by angle. Prior to these transform adjustments, the SkCanvas will only have
    // pixel aligned translations (these are separated to make super-sampling renderers easier).
    virtual void draw(SkCanvas* canvas, SkScalar width,
                      SkScalar tx, SkScalar ty, SkScalar angle) = 0;

    virtual SkString name() = 0;

    void applyLocalTransform(SkCanvas* canvas, SkScalar tx, SkScalar ty, SkScalar angle) {
        canvas->translate(tx, ty);
        canvas->rotate(angle, kTileWidth / 2.f, kTileHeight / 2.f);
    }

    SkPaint makePaint(SkPaint::Style style, SkScalar width) {
        SkPaint paint;
        paint.setColor4f({0.f, 0.f, 0.f, 1.f}, nullptr);
        paint.setAntiAlias(false);
        paint.setStyle(style);
        paint.setBlendMode(SkBlendMode::kClear);
        if (style != SkPaint::kFill_Style) {
            paint.setStrokeWidth(width);
            paint.setStrokeCap(SkPaint::kRound_Cap);
        }
        return paint;
    }
};

class RectRenderer : public ShapeRenderer {
public:
    static std::unique_ptr<ShapeRenderer> Make() {
        return std::unique_ptr<ShapeRenderer>(new RectRenderer());
    }

    SkString name() override { return SkString("rect"); }

    void draw(SkCanvas* canvas, SkScalar width, SkScalar tx, SkScalar ty, SkScalar angle) override {
        this->applyLocalTransform(canvas, tx, ty, angle);
        canvas->drawRect(SkRect::MakeLTRB(kTileWidth / 2.f - width / 2.f, 2.f,
                                          kTileWidth / 2.f + width / 2.f, kTileHeight - 2.f),
                         this->makePaint(SkPaint::kFill_Style, width));
    }

private:
    RectRenderer() {}

    typedef ShapeRenderer INHERITED;
};

class PathRenderer : public ShapeRenderer {
public:
    static std::unique_ptr<ShapeRenderer> MakeLine(SkScalar width = -1.f) {
        return MakeCurve(0.f, width);
    }

    static std::unique_ptr<ShapeRenderer> MakeCurve(SkScalar depth, SkScalar width = -1.f) {
        return std::unique_ptr<ShapeRenderer>(new PathRenderer(depth, width));
    }

    SkString name() override {
        SkString name("line");
        if (fWidthOverride > 0.f) {
            name.appendf("-%.2f", fWidthOverride);
        } else if (fWidthOverride == 0.f) {
            name.prepend("hair");
        }

        if (fDepth > 0.f) {
            name.appendf("-curve-%.2f", fDepth);
        }
        return name;
    }

    void draw(SkCanvas* canvas, SkScalar width, SkScalar tx, SkScalar ty, SkScalar angle) override {
        SkPath path;
        path.moveTo(kTileWidth / 2.f, 2.f);

        if (fDepth > 0.f) {
            path.quadTo(kTileWidth / 2.f + fDepth, kTileHeight / 2.f,
                        kTileWidth / 2.f, kTileHeight - 2.f);
        } else {
            path.lineTo(kTileWidth / 2.f, kTileHeight - 2.f);
        }

        SkScalar strokeWidth = fWidthOverride >= 0.f ? fWidthOverride : width;

        this->applyLocalTransform(canvas, tx, ty, angle);
        canvas->drawPath(path, this->makePaint(SkPaint::kStroke_Style, strokeWidth));
    }

private:
    PathRenderer(SkScalar depth, SkScalar width)
        : fDepth(depth)
        , fWidthOverride(width) {}

    SkScalar fDepth; // 0.f to make a line, otherwise outset of curve from end points
    SkScalar fWidthOverride; // negative value uses width parameter

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
        // Setup all renderers
        fShapes.push_back(RectRenderer::Make());
        fShapes.push_back(PathRenderer::MakeLine());
        fShapes.push_back(PathRenderer::MakeLine(0.f)); // Hairline
        fShapes.push_back(PathRenderer::MakeCurve(2.f)); // Shallow curve
        fShapes.push_back(PathRenderer::MakeCurve(8.f)); // Deep curve
        fShapes.push_back(PathRenderer::MakeCurve(8.f, 0.f)); // Hairline curve

        fSubpixelX = 0.f;
        fSubpixelY = 0.f;
        fAngle = 0.f;
    }

    void onDraw(SkCanvas* canvas) override {
        static constexpr SkScalar kWidths[] = { 2.0f, 1.0f, 0.75f, .5f, .25f, 0.125f };
        static constexpr SkScalar kAngles[] = { 0.f, 45.f, 90.f };
        static constexpr SkScalar kOffset = ShapeRenderer::kTileWidth + ShapeRenderer::kTileHeight;

        SkScalar gridWidth = SK_ARRAY_COUNT(kWidths) * ShapeRenderer::kTileWidth;
        SkScalar gridHeight = fShapes.count() * ShapeRenderer::kTileHeight;

        // Move away from screen edge
        canvas->translate(kOffset, kOffset);

        // Draw three grids, each using a fixed angle adjustment to the animating angle parameter
        // FIXME add labels based on shape, angle, and draw mode
        canvas->save();
        for (size_t i = 0; i < SK_ARRAY_COUNT(kAngles); ++i) {
            SkScalar angleOffset = kAngles[i];
            canvas->translate(kOffset + gridWidth, 0.f);

            // Draw the grid of shapes and widths with the animating parameters
            for (int j = 0; j < fShapes.count(); ++j) {
                for (size_t k = 0; k < SK_ARRAY_COUNT(kWidths); ++k) {
                    canvas->save();
                    canvas->translate(k * ShapeRenderer::kTileWidth,
                                      j * ShapeRenderer::kTileHeight);
                    fShapes[j]->draw(
                            canvas, kWidths[k], fSubpixelX, fSubpixelY, fAngle + angleOffset);
                    canvas->restore();
                }
            }
        }
        canvas->restore();

        canvas->translate(0.f, kOffset + gridHeight);

        // FIXME handle supersampling at 2, 4, and 8x
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        SkScalar t = timer.scaled(1.f, 5.f); // 1 second per side + 1 second for rotate
        SkScalar alpha = SkScalarFraction(t);

        if (t > 4.f) {
            // Adjust angle, 15 degrees per second
            fAngle = SkScalarMod(fStartAngle + alpha * 10.f, 360.f);
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
            // The time has reset for the angular rotation, so remember new start angle
            fStartAngle = fAngle;
        }

        return true;
    }

private:
    SkTArray<std::unique_ptr<ShapeRenderer>> fShapes;

    // Animated properties to stress the AA algorithms
    SkScalar fSubpixelX;
    SkScalar fSubpixelY;
    SkScalar fAngle;
    SkScalar fStartAngle;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM( return new ThinAAGM; )

}
