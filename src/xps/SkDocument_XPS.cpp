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

class SkDocument_XPS : public SkDocument {
public:
    SkDocument_XPS(SkWStream* stream, SkDocument::XPSParameters args)
        : SkDocument(stream, nullptr), fDevice(args.fFactory)
    {
        constexpr double kInchesPerMeter = 10000.0 / 254.0;

        SkScalar pointsPerMeter = SkDoubleToScalar(72.0 * kInchesPerMeter);
        fUnitsPerMeter.set(pointsPerMeter, pointsPerMeter);

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
    if (!dst || !args.fFactory || args.fRasterPixelsPerInch <= 0.0f) {
        return nullptr;
    }
    return sk_make_sp<SkDocument_XPS>(dst, std::move(args));
}

#endif//defined(SK_BUILD_FOR_WIN32)
