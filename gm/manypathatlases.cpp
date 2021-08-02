/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkPath.h"
#include "include/gpu/GrContextOptions.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrDrawingManager.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "tools/ToolUtils.h"

namespace skiagm {

/**
 * This test originally ensured that the ccpr path cache preserved fill rules properly. CCRP is gone
 * now, but we decided to keep the test.
 */
class ManyPathAtlasesGM : public GpuGM {
public:
    ManyPathAtlasesGM(int maxAtlasSize) : fMaxAtlasSize(maxAtlasSize) {}
private:
    SkString onShortName() override { return SkStringPrintf("manypathatlases_%i", fMaxAtlasSize); }
    SkISize onISize() override { return SkISize::Make(128, 128); }

    void modifyGrContextOptions(GrContextOptions* ctxOptions) override {
        // This will test the case where the atlas runs out of room if fMaxAtlasSize is small.
        ctxOptions->fMaxTextureAtlasSize = fMaxAtlasSize;
    }

    DrawResult onDraw(GrRecordingContext* rContext, SkCanvas* canvas, SkString* errorMsg) override {
        canvas->clear({1,1,0,1});

        // Flush the context to make the DAG empty. This will test the case where we try to add an
        // atlas task to an empty DAG.
        if (auto dContext = rContext->asDirectContext()) {
            dContext->flush();
        }

        SkPath clip = SkPath().moveTo(-50, 20)
                              .cubicTo(-50, -20, 50, -20, 50, 40)
                              .cubicTo(20, 0, -20, 0, -50, 20);
        clip.transform(SkMatrix::Translate(64, 70));
        for (int i = 0; i < 4; ++i) {
            SkPath rotatedClip = clip;
            rotatedClip.transform(SkMatrix::RotateDeg(30 * i + 128, {64, 70}));
            rotatedClip.setIsVolatile(true);
            canvas->clipPath(rotatedClip, SkClipOp::kDifference, true);
        }
        SkPath path = SkPath().moveTo(20, 0)
                              .lineTo(108, 0).cubicTo(108, 20, 108, 20, 128, 20)
                              .lineTo(128, 108).cubicTo(108, 108, 108, 108, 108, 128)
                              .lineTo(20, 128).cubicTo(20, 108, 20, 108, 0, 108)
                              .lineTo(0, 20).cubicTo(20, 20, 20, 20, 20, 0);
        path.setIsVolatile(true);
        SkPaint teal;
        teal.setColor4f({.03f, .91f, .87f, 1});
        teal.setAntiAlias(true);
        canvas->drawPath(path, teal);
        return DrawResult::kOk;
    }

    const int fMaxAtlasSize;
};

DEF_GM( return new ManyPathAtlasesGM(128); )  // Atlas runs out of room.
DEF_GM( return new ManyPathAtlasesGM(2048); )  // Atlas does not run out of room.

}  // namespace skiagm
