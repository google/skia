/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"
#include "SkRandom.h"

#include "SkCanvas.h"
#include "SkSurface.h"
#include "SkSurfaceProps.h"
#include "GrGLTypes.h"

static void write_pixmap(const char* name, const SkPixmap& src) {
    SkFILEWStream file(name);
    if (!SkEncodeImage(&file, src, SkEncodedImageFormat::kPNG, 100)) {
        SkDebugf("failed to write '%s'\n", name);
    }
}

void foo(GrContext* context) {
    GrGLFramebufferInfo fboInfo;
    fboInfo.fFBOID = 0;

    int width = 512;
    int height = 512;
    SkImageInfo ii = SkImageInfo::MakeN32Premul(width, height);
    int rowBytes = 4 * width;

#if 0
    GrBackendRenderTarget backendRT(width, height, 0, 8, kRGBA_8888_GrPixelConfig, fboInfo);

    SkSurfaceProps props(0, kUnknown_SkPixelGeometry);
    
    sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(
            context, backendRT, kBottomLeft_GrSurfaceOrigin, nullptr, &props));
#else
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, ii));
#endif

    SkCanvas* canvas = surface->getCanvas();

    SkRect r = SkRect::MakeWH(100, 100);
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->clear(SK_ColorWHITE);
    canvas->drawArc(r, 260, 285, false, paint);
    canvas->flush();

    sk_sp<SkImage> img(surface->makeImageSnapshot());

    std::unique_ptr<uint8_t[]> readback(new uint8_t[rowBytes * height]);
    // Clear so we don't accidentally see values from previous iteration.
    memset(readback.get(), 0xFF, rowBytes * height);

    SkPixmap pixels(ii, readback.get(), rowBytes);
    SkAssertResult(img->readPixels(pixels, 0, 0));

    static int i = 0;
    char name[128] = "/sdcard/readback0.png";
    name[16] = '0' + i;
    i++;

    write_pixmap(name, pixels);
}    

#if 0
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(FooTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    foo(context);
}
#endif


namespace skiagm {

// This GM draws a lot of arcs in a 'Z' shape. It particularly exercises
// the 'drawArc' code near a singularly of its processing (i.e., near the
// edge of one of its underlying quads).
class ArcOfZorroGM : public GM {
public:
    ArcOfZorroGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("arcofzorro");
    }

    SkISize onISize() override {
        return SkISize::Make(1000, 1000);
    }

    void onDraw(SkCanvas* canvas) override {
        if (canvas->getGrContext()) {
            foo(canvas->getGrContext());
        } else {
            canvas->clear(SK_ColorGREEN);
        }

    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ArcOfZorroGM;)
}
