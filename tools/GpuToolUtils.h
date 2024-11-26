/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GpuToolUtils_DEFINED
#define GpuToolUtils_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkRefCnt.h"

#if defined(SK_GANESH)
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#endif

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#endif

namespace ToolUtils {

// We do not put this in a .cpp file because otherwise the defines from the client
// (e.g. dm) won't be seen if this is compiled into a "core" tools library.
inline sk_sp<SkImage> MakeTextureImage(SkCanvas* canvas, sk_sp<SkImage> orig) {
    if (!orig) {
        return nullptr;
    }

#if defined(SK_GANESH)
    if (canvas->recordingContext() && canvas->recordingContext()->asDirectContext()) {
        GrDirectContext* dContext = canvas->recordingContext()->asDirectContext();
        const GrCaps* caps = dContext->priv().caps();

        if (orig->width() >= caps->maxTextureSize() || orig->height() >= caps->maxTextureSize()) {
            // Ganesh is able to tile large SkImage draws. Always forcing SkImages to be uploaded
            // prevents this feature from being tested by our tools. For now, leave excessively
            // large SkImages as bitmaps.
            return orig;
        }

        return SkImages::TextureFromImage(dContext, orig);
    }
#endif
#if defined(SK_GRAPHITE)
    if (canvas->recorder()) {
        return SkImages::TextureFromImage(canvas->recorder(), orig, {/*fMipmapped=*/false});
    }
#endif
    return orig;
}
}  // namespace ToolUtils

#endif
