/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrRecordingContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/base/SkRandom.h"
#include "tools/ToolUtils.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Surface.h"
#endif

namespace skiagm {

/*
 * This GM exercises SkCanvas::discard() by creating an offscreen SkSurface and repeatedly
 * discarding it, drawing to it, and then drawing it to the main canvas.
 */
class DiscardGM : public GM {

public:
    DiscardGM() {}

protected:
    SkString getName() const override { return SkString("discard"); }

    SkISize getISize() override { return SkISize::Make(100, 100); }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {

        SkISize size = this->getISize();
        size.fWidth /= 10;
        size.fHeight /= 10;
        SkImageInfo info = SkImageInfo::MakeN32Premul(size);
        sk_sp<SkSurface> surface;

        auto dContext = GrAsDirectContext(canvas->recordingContext());
        if (dContext && !dContext->abandoned()) {
            surface = SkSurfaces::RenderTarget(dContext, skgpu::Budgeted::kNo, info);
        }

#if defined(SK_GRAPHITE)
        auto recorder = canvas->recorder();
        if (recorder) {
            surface = SkSurfaces::RenderTarget(recorder, info);
        }
#endif

        if (!surface) {
            surface = SkSurfaces::Raster(info);
        }
        if (!surface) {
            *errorMsg = "Could not create surface.";
            return DrawResult::kFail;
        }

        canvas->clear(SK_ColorBLACK);

        SkRandom rand;
        for (int x = 0; x < 10; ++x) {
            for (int y = 0; y < 10; ++y) {
              surface->getCanvas()->discard();
              // Make something that isn't too close to the background color, black.
              SkColor color = ToolUtils::color_to_565(rand.nextU() | 0xFF404040);
              switch (rand.nextULessThan(3)) {
                  case 0:
                      surface->getCanvas()->drawColor(color);
                      break;
                  case 1:
                      surface->getCanvas()->clear(color);
                      break;
                  case 2:
                      SkPaint paint;
                      paint.setShader(SkShaders::Color(color));
                      surface->getCanvas()->drawPaint(paint);
                      break;
              }
              surface->draw(canvas, 10.f*x, 10.f*y);
            }
        }

        surface->getCanvas()->discard();
        return DrawResult::kOk;
    }
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new DiscardGM;)

}  // namespace skiagm
