/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "src/base/SkRandom.h"
#include "src/base/SkVx.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTaskGroup.h"
#include "tools/ToolUtils.h"
#include "tools/fonts/FontToolUtils.h"
#include "tools/viewer/Slide.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static text from paths.
class PathTextSlide : public Slide {
    constexpr static int kNumPaths = 1500;
    SkSize fSize;

public:
    PathTextSlide() { fName = "PathText"; }

    virtual void reset() {
        for (Glyph& glyph : fGlyphs) {
            glyph.reset(fRand, fSize.width(), fSize.height());
        }
        fGlyphAnimator->reset(&fRand, fSize.width(), fSize.height());
    }

    void load(SkScalar w, SkScalar h) final {
        fSize = {w, h};

        SkFont defaultFont = ToolUtils::DefaultFont();
        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeWithNoDevice(defaultFont);
        SkBulkGlyphMetricsAndPaths pathMaker{strikeSpec};
        SkPath glyphPaths[52];
        for (int i = 0; i < 52; ++i) {
            // I and l are rects on OS X ...
            char c = "aQCDEFGH7JKLMNOPBRZTUVWXYSAbcdefghijk1mnopqrstuvwxyz"[i];
            SkGlyphID id(defaultFont.unicharToGlyph(c));
            const SkGlyph* glyph = pathMaker.glyph(id);
            if (glyph->path()) {
                glyphPaths[i] = *glyph->path();
            }
        }

        for (int i = 0; i < kNumPaths; ++i) {
            const SkPath& p = glyphPaths[i % 52];
            fGlyphs[i].init(fRand, p);
        }
        this->reset();
    }

    void resize(SkScalar w, SkScalar h) final {
        fSize = {w, h};
        this->reset();
    }

    bool onChar(SkUnichar) override;

    bool animate(double nanos) final {
        return fGlyphAnimator->animate(nanos, fSize.width(), fSize.height());
    }

    void draw(SkCanvas* canvas) override {
        if (fDoClip) {
            SkPath deviceSpaceClipPath = fClipPath;
            deviceSpaceClipPath.transform(SkMatrix::Scale(fSize.width(), fSize.height()));
            canvas->save();
            canvas->clipPath(deviceSpaceClipPath, SkClipOp::kDifference, true);
            canvas->clear(SK_ColorBLACK);
            canvas->restore();
            canvas->clipPath(deviceSpaceClipPath, SkClipOp::kIntersect, true);
        }
        fGlyphAnimator->draw(canvas);
    }

protected:
    struct Glyph {
        void init(SkRandom& rand, const SkPath& path);
        void reset(SkRandom& rand, int w, int h);

        SkPath     fPath;
        SkPaint    fPaint;
        SkPoint    fPosition;
        SkScalar   fZoom;
        SkScalar   fSpin;
        SkPoint    fMidpt;
    };

    class GlyphAnimator {
    public:
        GlyphAnimator(Glyph* glyphs) : fGlyphs(glyphs) {}
        virtual void reset(SkRandom*, int screenWidth, int screenHeight) {}
        virtual bool animate(double nanos, int screenWidth, int screenHeight) { return false; }
        virtual void draw(SkCanvas* canvas) {
            for (int i = 0; i < kNumPaths; ++i) {
                Glyph& glyph = fGlyphs[i];
                SkAutoCanvasRestore acr(canvas, true);
                canvas->translate(glyph.fPosition.x(), glyph.fPosition.y());
                canvas->scale(glyph.fZoom, glyph.fZoom);
                canvas->rotate(glyph.fSpin);
                canvas->translate(-glyph.fMidpt.x(), -glyph.fMidpt.y());
                canvas->drawPath(glyph.fPath, glyph.fPaint);
            }
        }
        virtual ~GlyphAnimator() {}

    protected:
        Glyph* const fGlyphs;
    };

    class MovingGlyphAnimator;
    class WavyGlyphAnimator;

    Glyph fGlyphs[kNumPaths];
    SkRandom fRand{25};
    SkPath fClipPath = ToolUtils::make_star(SkRect{0, 0, 1, 1}, 11, 3);
    bool fDoClip = false;
    std::unique_ptr<GlyphAnimator> fGlyphAnimator = std::make_unique<GlyphAnimator>(fGlyphs);
};

void PathTextSlide::Glyph::init(SkRandom& rand, const SkPath& path) {
    fPath = path;
    fPaint.setAntiAlias(true);
    fPaint.setColor(rand.nextU() | 0x80808080);
}

void PathTextSlide::Glyph::reset(SkRandom& rand, int w, int h) {
    int screensize = std::max(w, h);
    const SkRect& bounds = fPath.getBounds();
    SkScalar t;

    fPosition = {rand.nextF() * w, rand.nextF() * h};
    t = pow(rand.nextF(), 100);
    fZoom = ((1 - t) * screensize / 50 + t * screensize / 3) /
            std::max(bounds.width(), bounds.height());
    fSpin = rand.nextF() * 360;
    fMidpt = {bounds.centerX(), bounds.centerY()};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Text from paths with animated transformation matrices.
class PathTextSlide::MovingGlyphAnimator : public PathTextSlide::GlyphAnimator {
public:
    MovingGlyphAnimator(Glyph* glyphs)
            : GlyphAnimator(glyphs)
            , fFrontMatrices(new SkMatrix[kNumPaths])
            , fBackMatrices(new SkMatrix[kNumPaths]) {
    }

    ~MovingGlyphAnimator() override {
        fBackgroundAnimationTask.wait();
    }

    void reset(SkRandom* rand, int screenWidth, int screenHeight) override {
        const SkScalar screensize = static_cast<SkScalar>(std::max(screenWidth, screenHeight));

        for (auto& v : fVelocities) {
            for (SkScalar* d : {&v.fDx, &v.fDy}) {
                SkScalar t = pow(rand->nextF(), 3);
                *d = ((1 - t) / 60 + t / 10) * (rand->nextBool() ? screensize : -screensize);
            }

            SkScalar t = pow(rand->nextF(), 25);
            v.fDSpin = ((1 - t) * 360 / 7.5 + t * 360 / 1.5) * (rand->nextBool() ? 1 : -1);
        }

        // Get valid front data.
        fBackgroundAnimationTask.wait();
        this->runAnimationTask(0, 0, screenWidth, screenHeight);
        std::copy_n(fBackMatrices.get(), kNumPaths, fFrontMatrices.get());
        fLastTick = 0;
    }

    bool animate(double nanos, int screenWidth, int screenHeight) final {
        fBackgroundAnimationTask.wait();
        this->swapAnimationBuffers();

        const double tsec = 1e-9 * nanos;
        const double dt = fLastTick ? (1e-9 * nanos - fLastTick) : 0;
        fBackgroundAnimationTask.add(std::bind(&MovingGlyphAnimator::runAnimationTask, this, tsec,
                                               dt, screenWidth, screenHeight));
        fLastTick = 1e-9 * nanos;
        return true;
    }

    /**
     * Called on a background thread. Here we can only modify fBackMatrices.
     */
    virtual void runAnimationTask(double t, double dt, int w, int h) {
        for (int idx = 0; idx < kNumPaths; ++idx) {
            Velocity* v = &fVelocities[idx];
            Glyph* glyph = &fGlyphs[idx];
            SkMatrix* backMatrix = &fBackMatrices[idx];

            glyph->fPosition.fX += v->fDx * dt;
            if (glyph->fPosition.x() < 0) {
                glyph->fPosition.fX -= 2 * glyph->fPosition.x();
                v->fDx = -v->fDx;
            } else if (glyph->fPosition.x() > w) {
                glyph->fPosition.fX -= 2 * (glyph->fPosition.x() - w);
                v->fDx = -v->fDx;
            }

            glyph->fPosition.fY += v->fDy * dt;
            if (glyph->fPosition.y() < 0) {
                glyph->fPosition.fY -= 2 * glyph->fPosition.y();
                v->fDy = -v->fDy;
            } else if (glyph->fPosition.y() > h) {
                glyph->fPosition.fY -= 2 * (glyph->fPosition.y() - h);
                v->fDy = -v->fDy;
            }

            glyph->fSpin += v->fDSpin * dt;

            backMatrix->setTranslate(glyph->fPosition.x(), glyph->fPosition.y());
            backMatrix->preScale(glyph->fZoom, glyph->fZoom);
            backMatrix->preRotate(glyph->fSpin);
            backMatrix->preTranslate(-glyph->fMidpt.x(), -glyph->fMidpt.y());
        }
    }

    virtual void swapAnimationBuffers() {
        std::swap(fFrontMatrices, fBackMatrices);
    }

    void draw(SkCanvas* canvas) override {
        for (int i = 0; i < kNumPaths; ++i) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->concat(fFrontMatrices[i]);
            canvas->drawPath(fGlyphs[i].fPath, fGlyphs[i].fPaint);
        }
    }

protected:
    struct Velocity {
        SkScalar fDx, fDy;
        SkScalar fDSpin;
    };

    Velocity fVelocities[kNumPaths];
    std::unique_ptr<SkMatrix[]> fFrontMatrices;
    std::unique_ptr<SkMatrix[]> fBackMatrices;
    SkTaskGroup fBackgroundAnimationTask;
    double fLastTick;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// Text from paths with animated control points.
class PathTextSlide::WavyGlyphAnimator : public PathTextSlide::MovingGlyphAnimator {
public:
    WavyGlyphAnimator(Glyph* glyphs)
            : MovingGlyphAnimator(glyphs)
            , fFrontPaths(new SkPath[kNumPaths])
            , fBackPaths(new SkPath[kNumPaths]) {
    }

    ~WavyGlyphAnimator() override {
        fBackgroundAnimationTask.wait();
    }

    void reset(SkRandom* rand, int screenWidth, int screenHeight) override {
        fWaves.reset(*rand, screenWidth, screenHeight);
        this->MovingGlyphAnimator::reset(rand, screenWidth, screenHeight);
        std::copy(fBackPaths.get(), fBackPaths.get() + kNumPaths, fFrontPaths.get());
    }

    /**
     * Called on a background thread. Here we can only modify fBackPaths.
     */
    void runAnimationTask(double t, double dt, int width, int height) override {
        const float tsec = static_cast<float>(t);
        this->MovingGlyphAnimator::runAnimationTask(t, 0.5 * dt, width, height);

        for (int i = 0; i < kNumPaths; ++i) {
            const Glyph& glyph = fGlyphs[i];
            const SkMatrix& backMatrix = fBackMatrices[i];

            const skvx::float2 matrix[3] = {
                skvx::float2(backMatrix.getScaleX(), backMatrix.getSkewY()),
                skvx::float2(backMatrix.getSkewX(), backMatrix.getScaleY()),
                skvx::float2(backMatrix.getTranslateX(), backMatrix.getTranslateY())
            };

            SkPath* backpath = &fBackPaths[i];
            backpath->reset();
            backpath->setFillType(SkPathFillType::kEvenOdd);

            for (auto [verb, pts, w] : SkPathPriv::Iterate(glyph.fPath)) {
                switch (verb) {
                    case SkPathVerb::kMove: {
                        SkPoint pt = fWaves.apply(tsec, matrix, pts[0]);
                        backpath->moveTo(pt.x(), pt.y());
                        break;
                    }
                    case SkPathVerb::kLine: {
                        SkPoint endpt = fWaves.apply(tsec, matrix, pts[1]);
                        backpath->lineTo(endpt.x(), endpt.y());
                        break;
                    }
                    case SkPathVerb::kQuad: {
                        SkPoint controlPt = fWaves.apply(tsec, matrix, pts[1]);
                        SkPoint endpt = fWaves.apply(tsec, matrix, pts[2]);
                        backpath->quadTo(controlPt.x(), controlPt.y(), endpt.x(), endpt.y());
                        break;
                    }
                    case SkPathVerb::kClose: {
                        backpath->close();
                        break;
                    }
                    case SkPathVerb::kCubic:
                    case SkPathVerb::kConic:
                        SK_ABORT("Unexpected path verb");
                        break;
                }
            }
        }
    }

    void swapAnimationBuffers() override {
        this->MovingGlyphAnimator::swapAnimationBuffers();
        std::swap(fFrontPaths, fBackPaths);
    }

    void draw(SkCanvas* canvas) override {
        for (int i = 0; i < kNumPaths; ++i) {
            canvas->drawPath(fFrontPaths[i], fGlyphs[i].fPaint);
        }
    }

private:
    /**
     * Describes 4 stacked sine waves that can offset a point as a function of wall time.
     */
    class Waves {
    public:
        void reset(SkRandom& rand, int w, int h);
        SkPoint apply(float tsec, const skvx::float2 matrix[3], const SkPoint& pt) const;

    private:
        constexpr static double kAverageAngle = SK_ScalarPI / 8.0;
        constexpr static double kMaxOffsetAngle = SK_ScalarPI / 3.0;

        float fAmplitudes[4];
        float fFrequencies[4];
        float fDirsX[4];
        float fDirsY[4];
        float fSpeeds[4];
        float fOffsets[4];
    };

    std::unique_ptr<SkPath[]> fFrontPaths;
    std::unique_ptr<SkPath[]> fBackPaths;
    Waves fWaves;
};

void PathTextSlide::WavyGlyphAnimator::Waves::reset(SkRandom& rand, int w, int h) {
    const double pixelsPerMeter = 0.06 * std::max(w, h);
    const double medianWavelength = 8 * pixelsPerMeter;
    const double medianWaveAmplitude = 0.05 * 4 * pixelsPerMeter;
    const double gravity = 9.8 * pixelsPerMeter;

    for (int i = 0; i < 4; ++i) {
        const double offsetAngle = (rand.nextF() * 2 - 1) * kMaxOffsetAngle;
        const double intensity = pow(2, rand.nextF() * 2 - 1);
        const double wavelength = intensity * medianWavelength;

        fAmplitudes[i] = intensity * medianWaveAmplitude;
        fFrequencies[i] = 2 * SK_ScalarPI / wavelength;
        fDirsX[i] = cosf(kAverageAngle + offsetAngle);
        fDirsY[i] = sinf(kAverageAngle + offsetAngle);
        fSpeeds[i] = -sqrt(gravity * 2 * SK_ScalarPI / wavelength);
        fOffsets[i] = rand.nextF() * 2 * SK_ScalarPI;
    }
}

SkPoint PathTextSlide::WavyGlyphAnimator::Waves::apply(float tsec, const skvx::float2 matrix[3],
                                                  const SkPoint& pt) const {
    constexpr static int kTablePeriod = 1 << 12;
    static float sin2table[kTablePeriod + 1];
    static SkOnce initTable;
    initTable([]() {
        for (int i = 0; i <= kTablePeriod; ++i) {
            const double sintheta = sin(i * (SK_ScalarPI / kTablePeriod));
            sin2table[i] = static_cast<float>(sintheta * sintheta - 0.5);
        }
    });

     const auto amplitudes = skvx::float4::Load(fAmplitudes);
     const auto frequencies = skvx::float4::Load(fFrequencies);
     const auto dirsX = skvx::float4::Load(fDirsX);
     const auto dirsY = skvx::float4::Load(fDirsY);
     const auto speeds = skvx::float4::Load(fSpeeds);
     const auto offsets = skvx::float4::Load(fOffsets);

    float devicePt[2];
    (matrix[0] * pt.x() + matrix[1] * pt.y() + matrix[2]).store(devicePt);

    const skvx::float4 t = abs(frequencies * (dirsX * devicePt[0] + dirsY * devicePt[1]) +
                               speeds * tsec + offsets) * (float(kTablePeriod) / SK_ScalarPI);

    const skvx::int4 ipart = skvx::cast<int32_t>(t);
    const skvx::float4 fpart = t - skvx::cast<float>(ipart);

    int32_t indices[4];
    (ipart & (kTablePeriod-1)).store(indices);

    const skvx::float4 left(sin2table[indices[0]], sin2table[indices[1]],
                            sin2table[indices[2]], sin2table[indices[3]]);
    const skvx::float4 right(sin2table[indices[0] + 1], sin2table[indices[1] + 1],
                             sin2table[indices[2] + 1], sin2table[indices[3] + 1]);
    const auto height = amplitudes * (left * (1.f - fpart) + right * fpart);

    auto dy = height * dirsY;
    auto dx = height * dirsX;

    float offsetY[4], offsetX[4];
    (dy + skvx::shuffle<2,3,0,1>(dy)).store(offsetY); // accumulate.
    (dx + skvx::shuffle<2,3,0,1>(dx)).store(offsetX);

    return {devicePt[0] + offsetY[0] + offsetY[1], devicePt[1] - offsetX[0] - offsetX[1]};
}

bool PathTextSlide::onChar(SkUnichar unichar) {
    switch (unichar) {
        case 'X':
            fDoClip = !fDoClip;
            return true;
        case 'S':
            fGlyphAnimator = std::make_unique<GlyphAnimator>(fGlyphs);
            fGlyphAnimator->reset(&fRand, fSize.width(), fSize.height());
            return true;
        case 'M':
            fGlyphAnimator = std::make_unique<MovingGlyphAnimator>(fGlyphs);
            fGlyphAnimator->reset(&fRand, fSize.width(), fSize.height());
            return true;
        case 'W':
            fGlyphAnimator = std::make_unique<WavyGlyphAnimator>(fGlyphs);
            fGlyphAnimator->reset(&fRand, fSize.width(), fSize.height());
            return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DEF_SLIDE( return new PathTextSlide; )
