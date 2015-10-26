/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DMSrcSink.h"
#include "DMSrcSinkAndroid.h"

#include "SkAndroidSDKCanvas.h"
#include "SkCanvas.h"
#include "SkiaCanvasProxy.h"
#include "SkStream.h"
#include <utils/TestWindowContext.h>

/* These functions are only compiled in the Android Framework. */

namespace DM {

Error HWUISink::draw(const Src& src, SkBitmap* dst, SkWStream*, SkString*) const {
    android::uirenderer::TestWindowContext renderer;
    renderer.initialize(src.size().width(), src.size().height());
    SkCanvas* canvas = renderer.prepareToDraw();
    Error err = src.draw(canvas);
    if (!err.isEmpty()) {
        return err;
    }
    renderer.finishDrawing();
    renderer.fence();
    renderer.capturePixels(dst);
    return "";
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

ViaAndroidSDK::ViaAndroidSDK(Sink* sink) : fSink(sink) { }

Error ViaAndroidSDK::draw(const Src& src,
                          SkBitmap* bitmap,
                          SkWStream* stream,
                          SkString* log) const {
    struct ProxySrc : public Src {
        const Src& fSrc;
        ProxySrc(const Src& src)
            : fSrc(src) {}

        Error draw(SkCanvas* canvas) const override {
            // Pass through HWUI's upper layers to get operational transforms
            SkAutoTDelete<android::Canvas> ac (android::Canvas::create_canvas(canvas));
            SkAutoTUnref<android::uirenderer::SkiaCanvasProxy> scProxy
                (new android::uirenderer::SkiaCanvasProxy(ac));

            // Pass through another proxy to get paint transforms
            SkAndroidSDKCanvas fc;
            fc.reset(scProxy);

            fSrc.draw(&fc);

            return "";
        }
        SkISize size() const override { return fSrc.size(); }
        Name name() const override { sk_throw(); return ""; }
    } proxy(src);

    return fSink->draw(proxy, bitmap, stream, log);
}

}  // namespace DM
