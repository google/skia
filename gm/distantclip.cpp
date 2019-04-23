
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"

namespace skiagm {

class DistantClipGM : public GM {
public:
    DistantClipGM() { }

protected:

    SkString onShortName() {
        return SkString("distantclip");
    }

    SkISize onISize() { return SkISize::Make(100, 100); }

    virtual void onDraw(SkCanvas* canvas) {
        constexpr SkScalar kOffset = 35000.0f;
        constexpr SkScalar kExtents = 1000.0f;

        SkPictureRecorder recorder;
        // We record a picture of huge vertical extents in which we clear the canvas to red, create
        // a 'extents' by 'extents' round rect clip at a vertical offset of 'offset', then draw
        // green into that.
        SkCanvas* rec = recorder.beginRecording(kExtents, kOffset + kExtents, nullptr, 0);
        rec->drawColor(SK_ColorRED);
        rec->save();
        SkRect r = SkRect::MakeXYWH(-kExtents, kOffset - kExtents, 2 * kExtents, 2 * kExtents);
        SkPath p;
        p.addRoundRect(r, 5, 5);
        rec->clipPath(p, true);
        rec->drawColor(SK_ColorGREEN);
        rec->restore();
        sk_sp<SkPicture> pict(recorder.finishRecordingAsPicture());

        // Next we play that picture into another picture of the same size.
        pict->playback(recorder.beginRecording(pict->cullRect().width(),
                                               pict->cullRect().height(),
                                               nullptr, 0));
        sk_sp<SkPicture> pict2(recorder.finishRecordingAsPicture());

        // Finally we play the part of that second picture that should be green into the canvas.
        canvas->save();
        canvas->translate(kExtents / 2, -(kOffset - kExtents / 2));
        pict2->playback(canvas);
        canvas->restore();

        // If the image is red, we erroneously decided the clipPath was empty and didn't record
        // the green drawColor, if it's green we're all good.
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

DEF_GM( return new DistantClipGM; )

}
