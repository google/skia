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
    SkDocument_XPS(SkWStream* stream,
                   void (*doneProc)(SkWStream*, bool),
                   SkScalar dpi)
        : SkDocument(stream, doneProc) {
        const SkScalar kPointsPerMeter = SkDoubleToScalar(360000.0 / 127.0);
        fUnitsPerMeter.set(kPointsPerMeter, kPointsPerMeter);
        SkScalar pixelsPerMeterScale = SkDoubleToScalar(dpi * 5000.0 / 127.0);
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

    bool onClose(SkWStream*) override {
        SkASSERT(!fCanvas.get());
        return fDevice.endPortfolio();
    }

    void onAbort() override {}

private:
    SkXPSDevice fDevice;
    SkAutoTUnref<SkCanvas> fCanvas;
    SkVector fUnitsPerMeter;
    SkVector fPixelsPerMeter;
};

///////////////////////////////////////////////////////////////////////////////

SkDocument* SkDocument::CreateXPS(SkWStream* stream, SkScalar dpi) {
    return stream ? new SkDocument_XPS(stream, nullptr, dpi) : nullptr;
}

static void delete_wstream(SkWStream* stream, bool aborted) { delete stream; }

SkDocument* SkDocument::CreateXPS(const char path[], SkScalar dpi) {
    SkAutoTDelete<SkFILEWStream> stream(new SkFILEWStream(path));
    if (!stream->isValid()) {
        return nullptr;
    }
    return new SkDocument_XPS(stream.release(), delete_wstream, dpi);
}

#endif//defined(SK_BUILD_FOR_WIN32)
