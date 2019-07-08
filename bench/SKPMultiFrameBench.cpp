/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/SKPMultiFrameBench.h"
#include "include/core/SkSurface.h"
#include "include/core/SkCanvas.h"
#include "tools/flags/CommandLineFlags.h"
#include "src/utils/SkMultiPictureDocument.h"
#include "tools/SkSharingProc.h"

SKPMultiFrameBench::SKPMultiFrameBench(const SkString& name, const SkString& path,
    const SkIRect& clip)
    : fUniqueName(name)
    , fDevBounds(clip) {
    // Load the multi frame skp at the given filename.
    std::unique_ptr<SkStreamAsset> stream = SkStream::MakeFromFile(path.c_str());
    if (!stream) {
        return;
    }

    // Attempt to deserialize with an image sharing serial proc.
    auto deserialContext = std::make_unique<SkSharingDeserialContext>();
    SkDeserialProcs procs;
    procs.fImageProc = SkSharingDeserialContext::deserializeImage;
    procs.fImageCtx = deserialContext.get();

    int page_count = SkMultiPictureDocumentReadPageCount(stream.get());
    if (!page_count) {
        return;
    }

    fFrames.reserve(page_count);
    if (!SkMultiPictureDocumentRead(stream.get(), fFrames.data(), page_count, &procs)) {
        return;
    }
}

const char* SKPMultiFrameBench::onGetUniqueName() {
    return fUniqueName.c_str();
}

void SKPMultiFrameBench::onPerCanvasPreDraw(SkCanvas* canvas) {
    fDevBounds = canvas->getDeviceClipBounds();
    SkAssertResult(!fDevBounds.isEmpty());
}

void SKPMultiFrameBench::onDraw(int loops, SkCanvas* canvas) {

    canvas->drawPicture(fFrames[fFrameIndex].fPicture);
    fFrameIndex = (fFrameIndex + 1) % fFrames.size();
}