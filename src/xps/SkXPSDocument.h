/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXPSDocument_DEFINED
#define SkXPSDocument_DEFINED

#include "SkTypes.h"

#ifdef SK_BUILD_FOR_WIN

#include "SkDocument.h"
#include "SkXPSDevice.h"
#include "SkTScopedComPtr.h"

#include <XpsObjectModel.h>

class SkXPSDocument final : public SkDocument {
public:
    SkXPSDocument(SkWStream*, SkScalar dpi, SkTScopedComPtr<IXpsOMObjectFactory>);
    virtual ~SkXPSDocument();

protected:
    SkCanvas* onBeginPage(SkScalar w, SkScalar h, const SkRect&) override;
    void onEndPage() override;
    void onClose(SkWStream*) override;
    void onAbort() override;

private:
    SkTScopedComPtr<IXpsOMObjectFactory> fXpsFactory;
    SkXPSDevice fDevice;
    std::unique_ptr<SkCanvas> fCanvas;
    SkVector fUnitsPerMeter;
    SkVector fPixelsPerMeter;
};

#endif  // SK_BUILD_FOR_WIN
#endif  // SkXPSDocument_DEFINED
