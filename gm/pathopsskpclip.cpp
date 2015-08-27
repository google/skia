/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkBitmap.h"
#include "SkCanvas.h"
#include "SkClipStack.h"
#include "SkDevice.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkRect.h"

namespace skiagm {

class PathOpsSkpClipGM : public GM {
public:
    PathOpsSkpClipGM() {
    }

protected:
    SkString onShortName() override {
        return SkString("pathopsskpclip");
    }

    SkISize onISize() override {
        return SkISize::Make(1200, 900);
    }

    void onDraw(SkCanvas* canvas) override {
        SkPictureRecorder recorder;
        SkCanvas* rec = recorder.beginRecording(1200, 900, nullptr, 0);
        SkPath p;
        SkRect r = {
            SkIntToScalar(100),
            SkIntToScalar(200),
            SkIntToScalar(400),
            SkIntToScalar(700)
        };
        p.addRoundRect(r, SkIntToScalar(50), SkIntToScalar(50));
        rec->clipPath(p, SkRegion::kIntersect_Op, true);
        rec->translate(SkIntToScalar(250), SkIntToScalar(250));
        rec->clipPath(p, SkRegion::kIntersect_Op, true);
        rec->drawColor(0xffff0000);
        SkAutoTUnref<SkPicture> pict(recorder.endRecording());

        canvas->setAllowSimplifyClip(true);
        canvas->save();
        canvas->drawPicture(pict);
        canvas->restore();

        canvas->setAllowSimplifyClip(false);
        canvas->save();
        canvas->translate(SkIntToScalar(1200 / 2), 0);
        canvas->drawPicture(pict);
        canvas->restore();
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new PathOpsSkpClipGM; }
static GMRegistry reg(MyFactory);

}
