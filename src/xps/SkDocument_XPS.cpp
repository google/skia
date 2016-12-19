/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if defined(SK_BUILD_FOR_WIN32)

#include "SkDocument.h"
#include "SkXPSDevice.h"
#include "SkStream.h"

static constexpr double kInchesPerMeter = 1.0 / 0.0254;

class SkDocument_XPS : public SkDocument {
public:
    SkDocument_XPS(SkWStream* stream, SkDocument::XPSParameters args)
        : SkDocument(stream, nullptr), , fDevice(args.fFactory)
    {
        const SkScalar kPointsPerMeter =
                SkDoubleToScalar(args.fCanvasUnitsPerInch * kInchesPerMeter);
        fUnitsPerMeter.set(kPointsPerMeter, kPointsPerMeter);
        SkScalar pixelsPerMeterScale =
                SkDoubleToScalar(args.fRasterPixelsPerInch * kInchesPerMeter);
        fPixelsPerMeter.set(pixelsPerMeterScale, pixelsPerMeterScale);
        fDevice.beginPortfolio(stream);
    }

    virtual ~SkDocument_XPS() {
        // subclasses must call close() in their destructors
        this->close();
    }

protected:
    SkCanvas* onBeginPage(SkScalar width,
                          SkScalar height,
                          const SkRect& trimBox) override {
        fDevice.beginSheet(fUnitsPerMeter, fPixelsPerMeter,
                           SkSize::Make(width, height));
        fCanvas.reset(new SkCanvas(&fDevice));
        fCanvas->clipRect(trimBox);
        fCanvas->translate(trimBox.x(), trimBox.y());
        return fCanvas.get();
    }

    void onEndPage() override {
        SkASSERT(fCanvas.get());
        fCanvas->flush();
        fCanvas.reset(nullptr);
        fDevice.endSheet();
    }

    void onClose(SkWStream*) override {
        SkASSERT(!fCanvas.get());
        (void)fDevice.endPortfolio();
    }

    void onAbort() override {}

private:
    SkXPSDevice fDevice;
    std::unique_ptr<SkCanvas> fCanvas;
    SkVector fUnitsPerMeter;
    SkVector fPixelsPerMeter;
};

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkDocument> SkDocument::MakeXPS(SkWStream* dst, SkDocument::XPSParameters args) {
    return dst ? sk_make_sp<SkDocument_XPS>(dst, std::move(args)) : nullptr;
}

#endif//defined(SK_BUILD_FOR_WIN32)
