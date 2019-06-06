/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/utils/SkRandom.h"
#include "samplecode/Sample.h"
#include "src/core/SkStrike.h"
#include "src/core/SkStrikeCache.h"
#include "src/core/SkStrikeSpec.h"
#include "src/core/SkTaskGroup.h"
#include "tools/ToolUtils.h"
#include "tools/timer/AnimTimer.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static text from paths.
class PathText : public Sample {
public:
    constexpr static int kNumPaths = 1500;
    virtual const char* getName() const { return "PathText"; }

    PathText() {}

    virtual void reset() {
        for (Glyph& glyph : fGlyphs) {
            glyph.reset(fRand, this->width(), this->height());
        }
    }

    void onOnceBeforeDraw() final {
        SkFont defaultFont;
        SkStrikeSpec strikeSpec = SkStrikeSpec::MakeWithNoDevice(defaultFont);
        auto cache = strikeSpec.findOrCreateExclusiveStrike();
        SkPath glyphPaths[52];
        for (int i = 0; i < 52; ++i) {
            // I and l are rects on OS X ...
            char c = "aQCDEFGH7JKLMNOPBRZTUVWXYSAbcdefghijk1mnopqrstuvwxyz"[i];
            SkPackedGlyphID id(defaultFont.unicharToGlyph(c));
            sk_ignore_unused_variable(cache->getScalerContext()->getPath(id, &glyphPaths[i]));
        }

        for (int i = 0; i < kNumPaths; ++i) {
            const SkPath& p = glyphPaths[i % 52];
            fGlyphs[i].init(fRand, p);
        }

        this->INHERITED::onOnceBeforeDraw();
        this->reset();
    }
    void onSizeChange() final { this->INHERITED::onSizeChange(); this->reset(); }

    bool onQuery(Sample::Event* evt) final {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, this->getName());
            return true;
        }
        SkUnichar unichar;
        if (Sample::CharQ(*evt, &unichar)) {
            if (unichar == 'X') {
                fDoClip = !fDoClip;
                return true;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
        if (fDoClip) {
            SkPath deviceSpaceClipPath = fClipPath;
            deviceSpaceClipPath.transform(SkMatrix::MakeScale(this->width(), this->height()));
            canvas->save();
            canvas->clipPath(deviceSpaceClipPath, SkClipOp::kDifference, true);
            canvas->clear(SK_ColorBLACK);
            canvas->restore();
            canvas->clipPath(deviceSpaceClipPath, SkClipOp::kIntersect, true);
        }
        this->drawGlyphs(canvas);
    }

    virtual void drawGlyphs(SkCanvas* canvas) {
        for (Glyph& glyph : fGlyphs) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->translate(glyph.fPosition.x(), glyph.fPosition.y());
            canvas->scale(glyph.fZoom, glyph.fZoom);
            canvas->rotate(glyph.fSpin);
            canvas->translate(-glyph.fMidpt.x(), -glyph.fMidpt.y());
            canvas->drawPath(glyph.fPath, glyph.fPaint);
        }
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

    Glyph      fGlyphs[kNumPaths];
    SkRandom   fRand{25};
    SkPath     fClipPath = ToolUtils::make_star(SkRect{0, 0, 1, 1}, 11, 3);
    bool       fDoClip = false;

    typedef Sample INHERITED;
};

void PathText::Glyph::init(SkRandom& rand, const SkPath& path) {
    fPath = path;
    fPaint.setAntiAlias(true);
    fPaint.setColor(rand.nextU() | 0x80808080);
}

void PathText::Glyph::reset(SkRandom& rand, int w, int h) {
    int screensize = SkTMax(w, h);
    const SkRect& bounds = fPath.getBounds();
    SkScalar t;

    fPosition = {rand.nextF() * w, rand.nextF() * h};
    t = pow(rand.nextF(), 100);
    fZoom = ((1 - t) * screensize / 50 + t * screensize / 3) /
            SkTMax(bounds.width(), bounds.height());
    fSpin = rand.nextF() * 360;
    fMidpt = {bounds.centerX(), bounds.centerY()};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Text from paths with animated transformation matrices.
class MovingPathText : public PathText {
public:
    const char* getName() const override { return "MovingPathText"; }

    MovingPathText()
        : fFrontMatrices(kNumPaths)
        , fBackMatrices(kNumPaths) {
    }

    ~MovingPathText() override {
        fBackgroundAnimationTask.wait();
    }

    void reset() override {
        const SkScalar screensize = static_cast<SkScalar>(SkTMax(this->width(), this->height()));
        this->INHERITED::reset();

        for (auto& v : fVelocities) {
            for (SkScalar* d : {&v.fDx, &v.fDy}) {
                SkScalar t = pow(fRand.nextF(), 3);
                *d = ((1 - t) / 60 + t / 10) * (fRand.nextBool() ? screensize : -screensize);
            }

            SkScalar t = pow(fRand.nextF(), 25);
            v.fDSpin = ((1 - t) * 360 / 7.5 + t * 360 / 1.5) * (fRand.nextBool() ? 1 : -1);
        }

        // Get valid front data.
        fBackgroundAnimationTask.wait();
        this->runAnimationTask(0, 0, this->width(), this->height());
        memcpy(fFrontMatrices, fBackMatrices, kNumPaths * sizeof(SkMatrix));
        fLastTick = 0;
    }

    bool onAnimate(const AnimTimer& timer) final {
        fBackgroundAnimationTask.wait();
        this->swapAnimationBuffers();

        const double tsec = timer.secs();
        const double dt = fLastTick ? (timer.secs() - fLastTick) : 0;
        fBackgroundAnimationTask.add(std::bind(&MovingPathText::runAnimationTask, this, tsec,
                                               dt, this->width(), this->height()));
        fLastTick = timer.secs();
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

    void drawGlyphs(SkCanvas* canvas) override {
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

    Velocity                  fVelocities[kNumPaths];
    SkAutoTMalloc<SkMatrix>   fFrontMatrices;
    SkAutoTMalloc<SkMatrix>   fBackMatrices;
    SkTaskGroup               fBackgroundAnimationTask;
    double                    fLastTick;

    typedef PathText INHERITED;
};


////////////////////////////////////////////////////////////////////////////////////////////////////
// Text from paths with animated control points.
class WavyPathText : public MovingPathText {
public:
    const char* getName() const override { return "WavyPathText"; }

    WavyPathText()
        : fFrontPaths(kNumPaths)
        , fBackPaths(kNumPaths) {}

    ~WavyPathText() override {
        fBackgroundAnimationTask.wait();
    }

    void reset() override {
        fWaves.reset(fRand, this->width(), this->height());
        this->INHERITED::reset();
        std::copy(fBackPaths.get(), fBackPaths.get() + kNumPaths, fFrontPaths.get());
    }

    /**
     * Called on a background thread. Here we can only modify fBackPaths.
     */
    void runAnimationTask(double t, double dt, int w, int h) override {
        const float tsec = static_cast<float>(t);
        this->INHERITED::runAnimationTask(t, 0.5 * dt, w, h);

        for (int i = 0; i < kNumPaths; ++i) {
            const Glyph& glyph = fGlyphs[i];
            const SkMatrix& backMatrix = fBackMatrices[i];

            const Sk2f matrix[3] = {
                Sk2f(backMatrix.getScaleX(), backMatrix.getSkewY()),
                Sk2f(backMatrix.getSkewX(), backMatrix.getScaleY()),
                Sk2f(backMatrix.getTranslateX(), backMatrix.getTranslateY())
            };

            SkPath* backpath = &fBackPaths[i];
            backpath->reset();
            backpath->setFillType(SkPath::kEvenOdd_FillType);

            SkPath::RawIter iter(glyph.fPath);
            SkPath::Verb verb;
            SkPoint pts[4];

            while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
                switch (verb) {
                    case SkPath::kMove_Verb: {
                        SkPoint pt = fWaves.apply(tsec, matrix, pts[0]);
                        backpath->moveTo(pt.x(), pt.y());
                        break;
                    }
                    case SkPath::kLine_Verb: {
                        SkPoint endpt = fWaves.apply(tsec, matrix, pts[1]);
                        backpath->lineTo(endpt.x(), endpt.y());
                        break;
                    }
                    case SkPath::kQuad_Verb: {
                        SkPoint controlPt = fWaves.apply(tsec, matrix, pts[1]);
                        SkPoint endpt = fWaves.apply(tsec, matrix, pts[2]);
                        backpath->quadTo(controlPt.x(), controlPt.y(), endpt.x(), endpt.y());
                        break;
                    }
                    case SkPath::kClose_Verb: {
                        backpath->close();
                        break;
                    }
                    case SkPath::kCubic_Verb:
                    case SkPath::kConic_Verb:
                    case SkPath::kDone_Verb:
                        SK_ABORT("Unexpected path verb");
                        break;
                }
            }
        }
    }

    void swapAnimationBuffers() override {
        this->INHERITED::swapAnimationBuffers();
        std::swap(fFrontPaths, fBackPaths);
    }

    void drawGlyphs(SkCanvas* canvas) override {
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
        SkPoint apply(float tsec, const Sk2f matrix[3], const SkPoint& pt) const;

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

    SkAutoTArray<SkPath>   fFrontPaths;
    SkAutoTArray<SkPath>   fBackPaths;
    Waves                  fWaves;

    typedef MovingPathText INHERITED;
};

void WavyPathText::Waves::reset(SkRandom& rand, int w, int h) {
    const double pixelsPerMeter = 0.06 * SkTMax(w, h);
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

SkPoint WavyPathText::Waves::apply(float tsec, const Sk2f matrix[3], const SkPoint& pt) const {
    constexpr static int kTablePeriod = 1 << 12;
    static float sin2table[kTablePeriod + 1];
    static SkOnce initTable;
    initTable([]() {
        for (int i = 0; i <= kTablePeriod; ++i) {
            const double sintheta = sin(i * (SK_ScalarPI / kTablePeriod));
            sin2table[i] = static_cast<float>(sintheta * sintheta - 0.5);
        }
    });

     const Sk4f amplitudes = Sk4f::Load(fAmplitudes);
     const Sk4f frequencies = Sk4f::Load(fFrequencies);
     const Sk4f dirsX = Sk4f::Load(fDirsX);
     const Sk4f dirsY = Sk4f::Load(fDirsY);
     const Sk4f speeds = Sk4f::Load(fSpeeds);
     const Sk4f offsets = Sk4f::Load(fOffsets);

    float devicePt[2];
    (matrix[0] * pt.x() + matrix[1] * pt.y() + matrix[2]).store(devicePt);

    const Sk4f t = (frequencies * (dirsX * devicePt[0] + dirsY * devicePt[1]) +
                    speeds * tsec +
                    offsets).abs() * (float(kTablePeriod) / float(SK_ScalarPI));

    const Sk4i ipart = SkNx_cast<int>(t);
    const Sk4f fpart = t - SkNx_cast<float>(ipart);

    int32_t indices[4];
    (ipart & (kTablePeriod-1)).store(indices);

    const Sk4f left(sin2table[indices[0]], sin2table[indices[1]],
                    sin2table[indices[2]], sin2table[indices[3]]);
    const Sk4f right(sin2table[indices[0] + 1], sin2table[indices[1] + 1],
                     sin2table[indices[2] + 1], sin2table[indices[3] + 1]);
    const Sk4f height = amplitudes * (left * (1.f - fpart) + right * fpart);

    Sk4f dy = height * dirsY;
    Sk4f dx = height * dirsX;

    float offsetY[4], offsetX[4];
    (dy + SkNx_shuffle<2,3,0,1>(dy)).store(offsetY); // accumulate.
    (dx + SkNx_shuffle<2,3,0,1>(dx)).store(offsetX);

    return {devicePt[0] + offsetY[0] + offsetY[1], devicePt[1] - offsetX[0] - offsetX[1]};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new WavyPathText; )
DEF_SAMPLE( return new MovingPathText; )
DEF_SAMPLE( return new PathText; )
