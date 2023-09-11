/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCapabilities.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurface.h"
#include "include/utils/SkNoDrawCanvas.h"
#include "src/image/SkSurface_Base.h"

class SkCanvas;
class SkPaint;
class SkPixmap;
struct SkIRect;
struct SkSamplingOptions;

class SkNullSurface : public SkSurface_Base {
public:
    SkNullSurface(int width, int height) : SkSurface_Base(width, height, nullptr) {}

    // From SkSurface_Base.h
    SkSurface_Base::Type type() const override { return SkSurface_Base::Type::kNull; }

protected:
    SkCanvas* onNewCanvas() override {
        return new SkNoDrawCanvas(this->width(), this->height());
    }
    sk_sp<SkSurface> onNewSurface(const SkImageInfo& info) override {
        return SkSurfaces::Null(info.width(), info.height());
    }
    sk_sp<SkImage> onNewImageSnapshot(const SkIRect* subsetOrNull) override { return nullptr; }
    void onWritePixels(const SkPixmap&, int x, int y) override {}
    void onDraw(SkCanvas*, SkScalar, SkScalar, const SkSamplingOptions&, const SkPaint*) override {}
    bool onCopyOnWrite(ContentChangeMode) override { return true; }
    sk_sp<const SkCapabilities> onCapabilities() override {
        // Not really, but we have to return *something*
        return SkCapabilities::RasterBackend();
    }
    SkImageInfo imageInfo() const override {
        return SkImageInfo::MakeUnknown(this->width(), this->height());
    }
};

namespace SkSurfaces {

sk_sp<SkSurface> Null(int width, int height) {
    if (width < 1 || height < 1) {
        return nullptr;
    }
    return sk_sp<SkSurface>(new SkNullSurface(width, height));
}

}
