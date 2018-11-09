/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlitMask.h"
#include "SkColor.h"
#include "SkOpts.h"

SkBlitMask::BlitLCD16RowProc SkBlitMask::BlitLCD16RowFactory(bool isOpaque) {
    BlitLCD16RowProc proc = PlatformBlitRowProcs16(isOpaque);
    if (proc) {
        return proc;
    }

    if (isOpaque) {
        return  SkBlitLCD16OpaqueRow;
    } else {
        return  SkBlitLCD16Row;
    }
}

static void D32_LCD16_Proc(void* dst, size_t dstRB,
                           const void* mask, size_t maskRB,
                           SkColor color, int width, int height) {

    SkPMColor*        dstRow = (SkPMColor*)dst;
    const uint16_t* srcRow = (const uint16_t*)mask;
    SkPMColor       opaqueDst;

    SkBlitMask::BlitLCD16RowProc proc = nullptr;
    bool isOpaque = (0xFF == SkColorGetA(color));
    proc = SkBlitMask::BlitLCD16RowFactory(isOpaque);
    SkASSERT(proc != nullptr);

    if (isOpaque) {
        opaqueDst = SkPreMultiplyColor(color);
    } else {
        opaqueDst = 0;  // ignored
    }

    do {
        proc(dstRow, srcRow, color, width, opaqueDst);
        dstRow = (SkPMColor*)((char*)dstRow + dstRB);
        srcRow = (const uint16_t*)((const char*)srcRow + maskRB);
    } while (--height != 0);
}

bool SkBlitMask::BlitColor(const SkPixmap& device,
                           const SkMask& mask,
                           const SkIRect& clip,
                           SkColor color) {
    int x = clip.fLeft,
        y = clip.fTop;

    if (device.colorType() == kN32_SkColorType && mask.fFormat == SkMask::kA8_Format) {
        SkOpts::blit_mask_d32_a8(device.writable_addr32(x,y), device.rowBytes(),
                                 (const SkAlpha*)mask.getAddr(x,y), mask.fRowBytes,
                                 color, clip.width(), clip.height());
        return true;
    }

    if (device.colorType() == kN32_SkColorType && mask.fFormat == SkMask::kLCD16_Format) {
        D32_LCD16_Proc(device.writable_addr32(x,y), device.rowBytes(),
                       mask.getAddr(x,y), mask.fRowBytes,
                       color, clip.width(), clip.height());
        return true;
    }

    return false;
}
