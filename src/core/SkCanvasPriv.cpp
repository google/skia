/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkCanvasPriv.h"

#include "src/base/SkAutoMalloc.h"
#include "src/core/SkDevice.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriter32.h"

#include <locale>

SkAutoCanvasMatrixPaint::SkAutoCanvasMatrixPaint(SkCanvas* canvas, const SkMatrix* matrix,
                                                 const SkPaint* paint, const SkRect& bounds)
        : fCanvas(canvas)
        , fSaveCount(canvas->getSaveCount()) {
    if (paint) {
        SkRect newBounds = bounds;
        if (matrix) {
            matrix->mapRect(&newBounds);
        }
        canvas->saveLayer(&newBounds, paint);
    } else if (matrix) {
        canvas->save();
    }

    if (matrix) {
        canvas->concat(*matrix);
    }
}

SkAutoCanvasMatrixPaint::~SkAutoCanvasMatrixPaint() {
    fCanvas->restoreToCount(fSaveCount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

bool SkCanvasPriv::ReadLattice(SkReadBuffer& buffer, SkCanvas::Lattice* lattice) {
    lattice->fXCount = buffer.readInt();
    lattice->fXDivs = buffer.skipT<int32_t>(lattice->fXCount);
    lattice->fYCount = buffer.readInt();
    lattice->fYDivs = buffer.skipT<int32_t>(lattice->fYCount);
    int flagCount = buffer.readInt();
    lattice->fRectTypes = nullptr;
    lattice->fColors = nullptr;
    if (flagCount) {
        lattice->fRectTypes = buffer.skipT<SkCanvas::Lattice::RectType>(flagCount);
        lattice->fColors = buffer.skipT<SkColor>(flagCount);
    }
    lattice->fBounds = buffer.skipT<SkIRect>();
    return buffer.isValid();
}

size_t SkCanvasPriv::WriteLattice(void* buffer, const SkCanvas::Lattice& lattice) {
    int flagCount = lattice.fRectTypes ? (lattice.fXCount + 1) * (lattice.fYCount + 1) : 0;

    const size_t size = (1 + lattice.fXCount + 1 + lattice.fYCount + 1) * sizeof(int32_t) +
                        SkAlign4(flagCount * sizeof(SkCanvas::Lattice::RectType)) +
                        SkAlign4(flagCount * sizeof(SkColor)) +
                        sizeof(SkIRect);

    if (buffer) {
        SkWriter32 writer(buffer, size);
        writer.write32(lattice.fXCount);
        writer.write(lattice.fXDivs, lattice.fXCount * sizeof(uint32_t));
        writer.write32(lattice.fYCount);
        writer.write(lattice.fYDivs, lattice.fYCount * sizeof(uint32_t));
        writer.write32(flagCount);
        writer.writePad(lattice.fRectTypes, flagCount * sizeof(uint8_t));
        writer.write(lattice.fColors, flagCount * sizeof(SkColor));
        SkASSERT(lattice.fBounds);
        writer.write(lattice.fBounds, sizeof(SkIRect));
        SkASSERT(writer.bytesWritten() == size);
    }
    return size;
}

void SkCanvasPriv::WriteLattice(SkWriteBuffer& buffer, const SkCanvas::Lattice& lattice) {
    const size_t size = WriteLattice(nullptr, lattice);
    SkAutoSMalloc<1024> storage(size);
    WriteLattice(storage.get(), lattice);
    buffer.writePad32(storage.get(), size);
}

void SkCanvasPriv::GetDstClipAndMatrixCounts(const SkCanvas::ImageSetEntry set[], int count,
                                             int* totalDstClipCount, int* totalMatrixCount) {
    int dstClipCount = 0;
    int maxMatrixIndex = -1;
    for (int i = 0; i < count; ++i) {
        dstClipCount += 4 * set[i].fHasClip;
        if (set[i].fMatrixIndex > maxMatrixIndex) {
            maxMatrixIndex = set[i].fMatrixIndex;
        }
    }

    *totalDstClipCount = dstClipCount;
    *totalMatrixCount = maxMatrixIndex + 1;
}

#if GR_TEST_UTILS && defined(SK_GANESH)

#include "src/gpu/ganesh/Device_v1.h"

skgpu::ganesh::SurfaceDrawContext* SkCanvasPriv::TopDeviceSurfaceDrawContext(SkCanvas* canvas) {
    if (auto gpuDevice = canvas->topDevice()->asGaneshDevice()) {
        return gpuDevice->surfaceDrawContext();
    }

    return nullptr;
}

skgpu::ganesh::SurfaceFillContext* SkCanvasPriv::TopDeviceSurfaceFillContext(SkCanvas* canvas) {
    if (auto gpuDevice = canvas->topDevice()->asGaneshDevice()) {
        return gpuDevice->surfaceFillContext();
    }

    return nullptr;
}

#endif // GR_TEST_UTILS && defined(SK_GANESH)


#if defined(SK_GANESH)
#include "src/gpu/ganesh/Device_v1.h"

GrRenderTargetProxy* SkCanvasPriv::TopDeviceTargetProxy(SkCanvas* canvas) {
    if (auto gpuDevice = canvas->topDevice()->asGaneshDevice()) {
        return gpuDevice->targetProxy();
    }

    return nullptr;
}

#else // defined(SK_GANESH)

GrRenderTargetProxy* SkCanvasPriv::TopDeviceTargetProxy(SkCanvas* canvas) {
    return nullptr;
}

#endif // defined(SK_GANESH)

#if GRAPHITE_TEST_UTILS
#include "src/gpu/graphite/Device.h"

skgpu::graphite::TextureProxy* SkCanvasPriv::TopDeviceGraphiteTargetProxy(SkCanvas* canvas) {
    if (auto gpuDevice = canvas->topDevice()->asGraphiteDevice()) {
        return gpuDevice->target();
    }
    return nullptr;
}

#endif // GRAPHITE_TEST_UTILS
