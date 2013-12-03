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

    virtual void onDraw(const int loops, SkCanvas* canvas) {

        SkPicture picture;

        SkCanvas* pCanvas = picture.beginRecording(PICTURE_WIDTH, PICTURE_HEIGHT);
        recordCanvas(pCanvas);
        picture.endRecording();

        const SkPoint translateDelta = getTranslateDelta(loops);

        for (int i = 0; i < loops; i++) {
            picture.draw(canvas);
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
    typedef SkBenchmark INHERITED;
};


class TextPlaybackBench : public PicturePlaybackBench {
public:
    TextPlaybackBench() : INHERITED("drawText") { }
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
    PosTextPlaybackBench(bool drawPosH)
        : INHERITED(drawPosH ? "drawPosTextH" : "drawPosText")
        , fDrawPosH(drawPosH) { }
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
