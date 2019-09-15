/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"

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
        p.addRRect(SkRRect::MakeRectXY(r, 50, 50));
        rec->clipPath(p, true);
        rec->translate(SkIntToScalar(250), SkIntToScalar(250));
        rec->clipPath(p, true);
        rec->drawColor(0xffff0000);
        sk_sp<SkPicture> pict(recorder.finishRecordingAsPicture());

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

DEF_GM( return new PathOpsSkpClipGM; )

}
