/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkTypes.h"
#if defined(SK_BUILD_FOR_WIN32)

#include "SkXPSDocument.h"
#include "SkStream.h"
#include "SkHRESULT.h"

SkXPSDocument::SkXPSDocument(SkWStream* stream,
                   void (*doneProc)(SkWStream*, bool),
                   SkScalar dpi,
                   SkTScopedComPtr<IXpsOMObjectFactory> xpsFactory)
        : SkDocument(stream, doneProc)
        , fXpsFactory(std::move(xpsFactory))
        , fDevice(SkISize{10000, 10000})
{
    const SkScalar kPointsPerMeter = SkDoubleToScalar(360000.0 / 127.0);
    fUnitsPerMeter.set(kPointsPerMeter, kPointsPerMeter);
    SkScalar pixelsPerMeterScale = SkDoubleToScalar(dpi * 5000.0 / 127.0);
    fPixelsPerMeter.set(pixelsPerMeterScale, pixelsPerMeterScale);
    SkASSERT(fXpsFactory);
    fDevice.beginPortfolio(stream, fXpsFactory.get());
}

SkXPSDocument::~SkXPSDocument() {
    // subclasses must call close() in their destructors
    this->close();
}

SkCanvas* SkXPSDocument::onBeginPage(SkScalar width,
                                     SkScalar height,
                                     const SkRect& trimBox) {
    fDevice.beginSheet(fUnitsPerMeter, fPixelsPerMeter,
                       SkSize::Make(width, height));
    fCanvas.reset(new SkCanvas(&fDevice));
    fCanvas->clipRect(trimBox);
    fCanvas->translate(trimBox.x(), trimBox.y());
    return fCanvas.get();
}

void SkXPSDocument::onEndPage() {
    SkASSERT(fCanvas.get());
    fCanvas->flush();
    fCanvas.reset(nullptr);
    fDevice.endSheet();
}

void SkXPSDocument::onClose(SkWStream*) {
    SkASSERT(!fCanvas.get());
    (void)fDevice.endPortfolio();
}

void SkXPSDocument::onAbort() {}


static SkTScopedComPtr<IXpsOMObjectFactory> make_xps_factory() {
    IXpsOMObjectFactory* factory;
    HRN(CoCreateInstance(CLSID_XpsOMObjectFactory,
                         nullptr,
                         CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&factory)));
    return SkTScopedComPtr<IXpsOMObjectFactory>(factory);
}


///////////////////////////////////////////////////////////////////////////////

// TODO(halcanary, reed): modify the SkDocument API to take a IXpsOMObjectFactory* pointer.
/*
sk_sp<SkDocument> SkDocument::MakeXPS(SkWStream* stream,
                                      IXpsOMObjectFactory* factoryPtr,
                                      SkScalar dpi) {
    SkTScopedComPtr<IXpsOMObjectFactory> factory(SkSafeRefComPtr(factoryPtr));
    return stream && factory
           ? sk_make_sp<SkXPSDocument>(stream, nullptr, dpi, std::move(factory))
           : nullptr;
}
*/

sk_sp<SkDocument> SkDocument::MakeXPS(SkWStream* stream, SkScalar dpi) {
    auto factory = make_xps_factory();
    return stream && factory
           ? sk_make_sp<SkXPSDocument>(stream, nullptr, dpi, std::move(factory))
           : nullptr;
}

sk_sp<SkDocument> SkDocument::MakeXPS(const char path[], SkScalar dpi) {
    std::unique_ptr<SkFILEWStream> stream(new SkFILEWStream(path));
    auto factory = make_xps_factory();
    return stream->isValid() && factory
           ? sk_make_sp<SkXPSDocument>(stream.release(),
                                       [](SkWStream* s, bool) { delete s; },
                                       dpi, std::move(factory))
           : nullptr;
}

#endif//defined(SK_BUILD_FOR_WIN32)
