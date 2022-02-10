/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Surface_Graphite_DEFINED
#define skgpu_Surface_Graphite_DEFINED

#include "src/image/SkSurface_Base.h"

namespace skgpu {

class Context;
class Device;
class Recorder;

class Surface final : public SkSurface_Base {
public:
    Surface(sk_sp<Device>);
    ~Surface() override;

    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&) override;
    sk_sp<SkImage> onNewImageSnapshot(const SkIRect* subset) override;
    void onWritePixels(const SkPixmap&, int x, int y) override;
    bool onCopyOnWrite(ContentChangeMode) override;
    bool onReadPixels(Context*, Recorder*, const SkPixmap& dst, int srcX, int srcY);

private:
    sk_sp<Device> fDevice;
};

} // namespace skgpu

#endif // skgpu_Surface_Graphite_DEFINED
