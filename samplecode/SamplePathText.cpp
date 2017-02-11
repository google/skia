/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkCanvas.h"
#include "SkGlyphCache.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkRandom.h"
#include "SkTaskGroup.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// Static text from paths.
class PathText : public SampleView {
public:
    constexpr static int kNumPaths = 1500;
    virtual const char* getName() const { return "PathText"; }

    PathText() : fRand(25) {
        SkPaint defaultPaint;
        SkAutoGlyphCache agc(defaultPaint, nullptr, &SkMatrix::I());
        SkGlyphCache* cache = agc.getCache();
        SkPath glyphPaths[52];
        for (int i = 0; i < 52; ++i) {
            // I and l are rects on OS X ...
            char c = "aQCDEFGH7JKLMNOPBRZTUVWXYSAbcdefghijk1mnopqrstuvwxyz"[i];
            SkGlyphID id = cache->unicharToGlyph(c);
            cache->getScalerContext()->getPath(SkPackedGlyphID(id), &glyphPaths[i]);
        }

        for (int i = 0; i < kNumPaths; ++i) {
            const SkPath& p = glyphPaths[i % 52];
            fGlyphs[i].init(fRand, p);
        }
    }

    virtual void reset() {
        for (Glyph& glyph : fGlyphs) {
            glyph.reset(fRand, this->width(), this->height());
        }
    }

    void onOnceBeforeDraw() final { this->INHERITED::onOnceBeforeDraw(); this->reset(); }
    void onSizeChange() final { this->INHERITED::onSizeChange(); this->reset(); }

    bool onQuery(SkEvent* evt) final {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, this->getName());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {
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
    SkRandom   fRand;

    typedef SampleView INHERITED;
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

    ~MovingPathText() {
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

    bool onAnimate(const SkAnimTimer& timer) final {
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
        fFrontMatrices.swap(fBackMatrices);
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkAutoCanvasRestore acr(canvas, true);
        for (int i = 0; i < kNumPaths; ++i) {
            canvas->setMatrix(fFrontMatrices[i]);
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

    ~WavyPathText() {
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
                        SkFAIL("Unexpected path verb");
                        break;
                }
            }
        }
    }

    void swapAnimationBuffers() override {
        this->INHERITED::swapAnimationBuffers();
        fFrontPaths.swap(fBackPaths);
    }

    void onDrawContent(SkCanvas* canvas) override {
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
        constexpr static double kAverageAngle = M_PI / 8.0;
        constexpr static double kMaxOffsetAngle = M_PI / 3.0;

        Sk4f fAmplitudes;
        Sk4f fFrequencies;
        Sk4f fDirsX;
        Sk4f fDirsY;
        Sk4f fSpeeds;
        Sk4f fOffsets;
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

    float amplitudes[4];
    float frequencies[4];
    float dirsX[4];
    float dirsY[4];
    float speeds[4];
    float offsets[4];

    for (int i = 0; i < 4; ++i) {
        const double offsetAngle = (rand.nextF() * 2 - 1) * kMaxOffsetAngle;
        const double intensity = pow(2, rand.nextF() * 2 - 1);
        const double wavelength = intensity * medianWavelength;

        amplitudes[i] = intensity * medianWaveAmplitude;
        frequencies[i] = 2 * M_PI / wavelength;
        dirsX[i] = cosf(kAverageAngle + offsetAngle);
        dirsY[i] = sinf(kAverageAngle + offsetAngle);
        speeds[i] = -sqrt(gravity * 2 * M_PI / wavelength);
        offsets[i] = rand.nextF() * 2 * M_PI;
    }

    fAmplitudes = Sk4f::Load(amplitudes);
    fFrequencies = Sk4f::Load(frequencies);
    fDirsX = Sk4f::Load(dirsX);
    fDirsY = Sk4f::Load(dirsY);
    fSpeeds = Sk4f::Load(speeds);
    fOffsets = Sk4f::Load(offsets);
}

SkPoint WavyPathText::Waves::apply(float tsec, const Sk2f matrix[3], const SkPoint& pt) const {
    constexpr int tableSize = 4096;
    static float sin2table[tableSize];
    static SkOnce initTable;
    initTable([]() {
        for (int i = 0; i <= tableSize; ++i) {
            const double sintheta = sin(i * (M_PI / tableSize));
            sin2table[i] = static_cast<float>(sintheta * sintheta - 0.5);
        }
    });

    float devicePt[2];
    (matrix[0] * pt.x() + matrix[1] * pt.y() + matrix[2]).store(devicePt);

    const Sk4f t = (fFrequencies * (fDirsX * devicePt[0] + fDirsY * devicePt[1]) +
                    fSpeeds * tsec +
                    fOffsets).abs() * (float(tableSize) / float(M_PI));

    const Sk4i ipart = SkNx_cast<int>(t);
    const Sk4f fpart = t - SkNx_cast<float>(ipart);

    int32_t indices[4];
    (ipart & (tableSize-1)).store(indices);

    const Sk4f left(sin2table[indices[0]], sin2table[indices[1]],
                    sin2table[indices[2]], sin2table[indices[3]]);
    const Sk4f right(sin2table[indices[0] + 1], sin2table[indices[1] + 1],
                     sin2table[indices[2] + 1], sin2table[indices[3] + 1]);
    const Sk4f height = fAmplitudes * (left * (1.f - fpart) + right * fpart);

    Sk4f dy = height * fDirsY;
    Sk4f dx = height * fDirsX;
    dy += SkNx_shuffle<2,3,0,1>(dy); // accumulate.
    dx += SkNx_shuffle<2,3,0,1>(dx);

    float offsetY[4], offsetX[4];
    dy.store(offsetY);
    dx.store(offsetX);

    return {devicePt[0] + offsetY[0] + offsetY[1], devicePt[1] - offsetX[0] - offsetX[1]};
}

////////////////////////////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new WavyPathText; )
DEF_SAMPLE( return new MovingPathText; )
DEF_SAMPLE( return new PathText; )
