/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skcpu_Recorder_DEFINED
#define skcpu_Recorder_DEFINED

#include "include/core/SkRecorder.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkAPI.h"

class SkSurface;
class SkSurfaceProps;
struct SkImageInfo;

#include <cstddef>

namespace skcpu {

class SK_API Recorder : public SkRecorder {
public:
    /** Returns a non-null global context. Can be used as a means of transitioning onto
     * new APIs when a skcpu::Context/Recorder has not been piped down into the code paths
     */
    static Recorder* TODO();

    SkRecorder::Type type() const final { return SkRecorder::Type::kCPU; }
    skcpu::Recorder* cpuRecorder() final { return this; }

    /** Allocates a bitmap-backed SkSurface. SkCanvas returned by SkSurface draws directly into
     *  those allocated pixels, which are zeroed before use. Pixel memory size is imageInfo.height()
     *  times imageInfo.minRowBytes() or rowBytes, if provided and non-zero.
     *
     *  Pixel memory is deleted when SkSurface is deleted.
     *
     *  Validity constraints include:
     *    - info dimensions are greater than zero;
     *    - info contains SkColorType and SkAlphaType supported by raster surface.
     *
     *  @param imageInfo  width, height, SkColorType, SkAlphaType, SkColorSpace,
     *                    of raster surface; width and height must be greater than zero
     *  @param rowBytes   interval from one SkSurface row to the next.
     *  @param props      LCD striping orientation and setting for device independent fonts;
     *                    may be nullptr
     *  @return           SkSurface if parameters are valid and memory was allocated, else nullptr.
     */
    sk_sp<SkSurface> makeBitmapSurface(const SkImageInfo& imageInfo,
                                       size_t rowBytes,
                                       const SkSurfaceProps* surfaceProps);
    sk_sp<SkSurface> makeBitmapSurface(const SkImageInfo& imageInfo,
                                       const SkSurfaceProps* surfaceProps = nullptr);
};

inline Recorder* AsRecorder(SkRecorder* recorder) {
    if (!recorder) {
        return nullptr;
    }
    if (recorder->type() != SkRecorder::Type::kCPU) {
        return nullptr;
    }
    return static_cast<Recorder*>(recorder);
}

}  // namespace skcpu

#endif
