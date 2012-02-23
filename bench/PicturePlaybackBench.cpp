/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkCanvas.h"
#include "SkColor.h"
#include "SkPaint.h"
#include "SkPicture.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkString.h"

// This is designed to emulate about 4 screens of textual content


class PicturePlaybackBench : public SkBenchmark {
public:
    PicturePlaybackBench(void* param, const char name[]) : INHERITED(param) {
        fName.printf("picture_playback_%s", name);
        fPictureWidth = SkIntToScalar(PICTURE_WIDTH);
        fPictureHeight = SkIntToScalar(PICTURE_HEIGHT);
        fTextSize = SkIntToScalar(TEXT_SIZE);
    }

    enum {
        N = SkBENCHLOOP(1000),   // number of times to playback the picture
        PICTURE_WIDTH = 1000,
        PICTURE_HEIGHT = 4000,
        TEXT_SIZE = 10
    };
protected:
    virtual const char* onGetName() {
        return fName.c_str();
    }

    virtual void onDraw(SkCanvas* canvas) {

        SkPicture picture;

        SkCanvas* pCanvas = picture.beginRecording(PICTURE_WIDTH, PICTURE_HEIGHT);
        recordCanvas(pCanvas);
        picture.endRecording();

        const SkPoint translateDelta = getTranslateDelta();

        for (int i = 0; i < N; i++) {
            picture.draw(canvas);
            canvas->translate(translateDelta.fX, translateDelta.fY);
        }
    }

    virtual void recordCanvas(SkCanvas* canvas) = 0;
    virtual SkPoint getTranslateDelta() {
        SkIPoint canvasSize = onGetSize();
        return SkPoint::Make(SkIntToScalar((PICTURE_WIDTH - canvasSize.fX)/N),
                             SkIntToScalar((PICTURE_HEIGHT- canvasSize.fY)/N));
    }

    SkString fName;
    SkScalar fPictureWidth;
    SkScalar fPictureHeight;
    SkScalar fTextSize;
private:
    typedef SkBenchmark INHERITED;
};


class TextPlaybackBench : public PicturePlaybackBench {
public:
    TextPlaybackBench(void* param) : INHERITED(param, "drawText") { }
protected:
    virtual void recordCanvas(SkCanvas* canvas) {
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
    PosTextPlaybackBench(void* param, bool drawPosH)
        : fDrawPosH(drawPosH)
        , INHERITED(param, drawPosH ? "drawPosTextH" : "drawPosText") { }
protected:
    virtual void recordCanvas(SkCanvas* canvas) {
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
                        pos[i].set(x + advX, y + SkIntToScalar(i));
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

static SkBenchmark* Fact0(void* p) { return new TextPlaybackBench(p); }
static SkBenchmark* Fact1(void* p) { return new PosTextPlaybackBench(p, true); }
static SkBenchmark* Fact2(void* p) { return new PosTextPlaybackBench(p, false); }

static BenchRegistry gReg0(Fact0);
static BenchRegistry gReg1(Fact1);
static BenchRegistry gReg2(Fact2);

