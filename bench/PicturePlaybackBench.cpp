/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "Benchmark.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkPoint.h"
#include "SkRandom.h"
#include "SkRect.h"
#include "SkString.h"

// This is designed to emulate about 4 screens of textual content


class PicturePlaybackBench : public Benchmark {
public:
    PicturePlaybackBench(const char name[])  {
        fName.printf("picture_playback_%s", name);
        fPictureWidth = SkIntToScalar(PICTURE_WIDTH);
        fPictureHeight = SkIntToScalar(PICTURE_HEIGHT);
        fTextSize = SkIntToScalar(TEXT_SIZE);
    }

    enum {
        PICTURE_WIDTH = 1000,
        PICTURE_HEIGHT = 4000,
        TEXT_SIZE = 10
    };
protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(int loops, SkCanvas* canvas) {

        SkPictureRecorder recorder;
        SkCanvas* pCanvas = recorder.beginRecording(PICTURE_WIDTH, PICTURE_HEIGHT, nullptr, 0);
        this->recordCanvas(pCanvas);
        sk_sp<SkPicture> picture(recorder.finishRecordingAsPicture());

        const SkPoint translateDelta = getTranslateDelta(loops);

        for (int i = 0; i < loops; i++) {
            picture->playback(canvas);
            canvas->translate(translateDelta.fX, translateDelta.fY);
        }
    }

    virtual void recordCanvas(SkCanvas* canvas) = 0;
    virtual SkPoint getTranslateDelta(int N) {
        SkIPoint canvasSize = onGetSize();
        return SkPoint::Make(SkIntToScalar((PICTURE_WIDTH - canvasSize.fX)/N),
                             SkIntToScalar((PICTURE_HEIGHT- canvasSize.fY)/N));
    }

    SkString fName;
    SkScalar fPictureWidth;
    SkScalar fPictureHeight;
    SkScalar fTextSize;
private:
    typedef Benchmark INHERITED;
};


class TextPlaybackBench : public PicturePlaybackBench {
public:
    TextPlaybackBench() : INHERITED("drawText") { }
protected:
    void recordCanvas(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setTextSize(fTextSize);
        paint.setColor(SK_ColorBLACK);

        const char* text = "Hamburgefons";
        size_t len = strlen(text);
        const SkScalar textWidth = paint.measureText(text, len);

        for (SkScalar x = 0; x < fPictureWidth; x += textWidth) {
            for (SkScalar y = 0; y < fPictureHeight; y += fTextSize) {
                canvas->drawText(text, len, x, y, paint);
            }
        }
    }
private:
    typedef PicturePlaybackBench INHERITED;
};

class PosTextPlaybackBench : public PicturePlaybackBench {
public:
    PosTextPlaybackBench(bool drawPosH)
        : INHERITED(drawPosH ? "drawPosTextH" : "drawPosText")
        , fDrawPosH(drawPosH) { }
protected:
    void recordCanvas(SkCanvas* canvas) override {
        SkPaint paint;
        paint.setTextSize(fTextSize);
        paint.setColor(SK_ColorBLACK);

        const char* text = "Hamburgefons";
        size_t len = strlen(text);
        const SkScalar textWidth = paint.measureText(text, len);

        SkScalar* adv = new SkScalar[len];
        paint.getTextWidths(text, len, adv);

        for (SkScalar x = 0; x < fPictureWidth; x += textWidth) {
            for (SkScalar y = 0; y < fPictureHeight; y += fTextSize) {

                SkPoint* pos = new SkPoint[len];
                SkScalar advX = 0;

                for (size_t i = 0; i < len; i++) {
                    if (fDrawPosH)
                        pos[i].set(x + advX, y);
                    else
                        pos[i].set(x + advX, y + i);
                    advX += adv[i];
                }

                canvas->drawPosText(text, len, pos, paint);
                delete[] pos;
            }
        }
        delete[] adv;
    }
private:
    bool fDrawPosH;
    typedef PicturePlaybackBench INHERITED;
};


///////////////////////////////////////////////////////////////////////////////

DEF_BENCH( return new TextPlaybackBench(); )
DEF_BENCH( return new PosTextPlaybackBench(true); )
DEF_BENCH( return new PosTextPlaybackBench(false); )

// Chrome draws into small tiles with impl-side painting.
// This benchmark measures the relative performance of our bounding-box hierarchies,
// both when querying tiles perfectly and when not.
enum BBH  { kNone, kRTree };
enum Mode { kTiled, kRandom };
class TiledPlaybackBench : public Benchmark {
public:
    TiledPlaybackBench(BBH bbh, Mode mode) : fBBH(bbh), fMode(mode), fName("tiled_playback") {
        switch (fBBH) {
            case kNone:     fName.append("_none"    ); break;
            case kRTree:    fName.append("_rtree"   ); break;
        }
        switch (fMode) {
            case kTiled:  fName.append("_tiled" ); break;
            case kRandom: fName.append("_random"); break;
        }
    }

    const char* onGetName() override { return fName.c_str(); }
    SkIPoint onGetSize() override { return SkIPoint::Make(1024,1024); }

    void onDelayedSetup() override {
        SkAutoTDelete<SkBBHFactory> factory;
        switch (fBBH) {
            case kNone:                                                 break;
            case kRTree:    factory.reset(new SkRTreeFactory);          break;
        }

        SkPictureRecorder recorder;
        SkCanvas* canvas = recorder.beginRecording(1024, 1024, factory);
            SkRandom rand;
            for (int i = 0; i < 10000; i++) {
                SkScalar x = rand.nextRangeScalar(0, 1024),
                         y = rand.nextRangeScalar(0, 1024),
                         w = rand.nextRangeScalar(0, 128),
                         h = rand.nextRangeScalar(0, 128);
                SkPaint paint;
                paint.setColor(rand.nextU());
                paint.setAlpha(0xFF);
                canvas->drawRect(SkRect::MakeXYWH(x,y,w,h), paint);
            }
        fPic = recorder.finishRecordingAsPicture();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        for (int i = 0; i < loops; i++) {
            // This inner loop guarantees we make the same choices for all bench variants.
            SkRandom rand;
            for (int j = 0; j < 10; j++) {
                SkScalar x = 0, y = 0;
                switch (fMode) {
                    case kTiled:  x = SkScalar(256 * rand.nextULessThan(4));
                                  y = SkScalar(256 * rand.nextULessThan(4));
                                  break;
                    case kRandom: x = rand.nextRangeScalar(0, 768);
                                  y = rand.nextRangeScalar(0, 768);
                                  break;
                }
                SkAutoCanvasRestore ar(canvas, true/*save now*/);
                canvas->clipRect(SkRect::MakeXYWH(x,y,256,256));
                fPic->playback(canvas);
            }
        }
    }

private:
    BBH                 fBBH;
    Mode                fMode;
    SkString            fName;
    sk_sp<SkPicture>    fPic;
};

DEF_BENCH( return new TiledPlaybackBench(kNone,     kRandom); )
DEF_BENCH( return new TiledPlaybackBench(kNone,     kTiled ); )
DEF_BENCH( return new TiledPlaybackBench(kRTree,    kRandom); )
DEF_BENCH( return new TiledPlaybackBench(kRTree,    kTiled ); )
