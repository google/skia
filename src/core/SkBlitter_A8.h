/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBlitter_A8_DEFINED
#define SkBlitter_A8_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkBlitter.h"

#include <cstdint>

class SkArenaAlloc;
class SkMatrix;
class SkPaint;
class SkShader;
class SkSurfaceProps;
struct SkIRect;
struct SkMask;
enum class SkDrawCoverage : bool;

class SkA8_Coverage_Blitter : public SkBlitter {
public:
    SkA8_Coverage_Blitter(const SkPixmap& device, const SkPaint& paint);
    void blitH(int x, int y, int width) override;
    void blitAntiH(int x, int y, const SkAlpha antialias[], const int16_t runs[]) override;
    void blitV(int x, int y, int height, SkAlpha alpha) override;
    void blitRect(int x, int y, int width, int height) override;
    void blitMask(const SkMask&, const SkIRect&) override;

private:
    const SkPixmap fDevice;
};

SkBlitter* SkA8Blitter_Choose(const SkPixmap& dst,
                              const SkMatrix& ctm,
                              const SkPaint& paint,
                              SkArenaAlloc*,
                              SkDrawCoverage,
                              sk_sp<SkShader> clipShader,
                              const SkSurfaceProps&);

#endif // SkBlitter_A8_DEFINED
