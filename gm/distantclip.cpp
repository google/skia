

/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPicture.h"

namespace skiagm {

class DistantClipGM : public GM {
public:
    DistantClipGM() { }

protected:

    SkString onShortName() {
        return SkString("distantclip");
    }

    SkISize onISize() { return make_isize(100, 100); }

    virtual void onDraw(SkCanvas* canvas) {
        int offset = 35000;
        int extents = 1000;

        // We record a picture of huge vertical extents in which we clear the canvas to red, create
        // a 'extents' by 'extents' round rect clip at a vertical offset of 'offset', then draw
        // green into that.
        SkPicture pict;
        SkCanvas* rec = pict.beginRecording(100, offset + extents);
        rec->drawColor(0xffff0000);
        rec->save();
        SkRect r = {-extents, offset - extents, extents, offset + extents};
        SkPath p;
        p.addRoundRect(r, 5, 5);
        rec->clipPath(p, SkRegion::kIntersect_Op, true);
        rec->drawColor(0xff00ff00);
        rec->restore();
        pict.endRecording();

        // Next we play that picture into another picture of the same size.
        SkPicture pict2;
        pict.draw(pict2.beginRecording(100, offset + extents));
        pict2.endRecording();

        // Finally we play the part of that second picture that should be green into the canvas.
        canvas->save();
        canvas->translate(extents / 2, -(offset - extents / 2));
        pict2.draw(canvas);
        canvas->restore();

        // If the image is red, we erroneously decided the clipPath was empty and didn't record
        // the green drawColor, if it's green we're all good.
    }

private:
    typedef GM INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new DistantClipGM; }
static GMRegistry reg(MyFactory);

}
