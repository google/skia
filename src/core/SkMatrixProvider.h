/*
* Copyright 2020 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkMatrixProvider_DEFINED
#define SkMatrixProvider_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"

/**
 *  All matrix providers report a flag: "localToDeviceHitsPixelCenters". This is confusing.
 *  It doesn't say anything about the actual matrix in the provider. Instead, it means: "is it safe
 *  to tweak sampling based on the contents of the matrix". In other words, does the device end of
 *  the local-to-device matrix actually map to pixels, AND are the local coordinates being fed to
 *  the shader produced by the inverse of that matrix? For a normal device, this is trivially true.
 *  The matrix may be updated via transforms, but when we draw (and the local coordinates come from
 *  rasterization of primitives against that device), we can know that the device coordinates will
 *  land on pixel centers.
 *
 *  In a few places, the matrix provider is lying about how sampling "works". When we invoke a child
 *  from runtime effects, we give that child a matrix provider with an identity matrix. However --
 *  the coordinates being passed to that child are not the result of device -> local transformed
 *  coordinates. Runtime effects can generate coordinates arbitrarily - even though the provider has
 *  an identity matrix, we can't assume it's safe to (for example) convert linear -> nearest.
 *  Clip shaders are similar - they overwrite the local-to-device matrix (to match what it was when
 *  the clip shader was inserted). The CTM continues to change before drawing, though. In that case,
 *  the two matrices are not inverses, so the local coordinates may not land on texel centers in
 *  the clip shader.
 *
 *  In cases where we need to inhibit filtering optimizations, use SkOverrideDeviceMatrixProvider.
 */
class SkMatrixProvider {
public:
    SkMatrixProvider(const SkMatrix& localToDevice) : SkMatrixProvider(localToDevice, true) {}
    SkMatrixProvider(const SkM44& localToDevice) : SkMatrixProvider(localToDevice, true) {}

    // Required until C++17 copy elision
    SkMatrixProvider(const SkMatrixProvider&) = default;

    // These should return the "same" matrix, as either a 3x3 or 4x4. Most sites in Skia still
    // call localToDevice, and operate on SkMatrix.
    const SkMatrix& localToDevice() const { return fLocalToDevice33; }
    const SkM44& localToDevice44() const { return fLocalToDevice; }

    bool localToDeviceHitsPixelCenters() const { return fHitsPixelCenters; }

protected:
    SkMatrixProvider(const SkMatrix& localToDevice, bool hitsPixelCenters)
        : fLocalToDevice(localToDevice)
        , fLocalToDevice33(localToDevice)
        , fHitsPixelCenters(hitsPixelCenters) {}

    SkMatrixProvider(const SkM44& localToDevice, bool hitsPixelCenters)
        : fLocalToDevice(localToDevice)
        , fLocalToDevice33(localToDevice.asM33())
        , fHitsPixelCenters(hitsPixelCenters) {}

private:
    friend class SkBaseDevice;

    SkM44      fLocalToDevice;
    SkMatrix   fLocalToDevice33;  // Cached SkMatrix version of above, for legacy usage
    const bool fHitsPixelCenters;
};

// Logically, this is equivalent to just making a new SkMatrixProvider, but we keep it distinct.
// This is for cases where there is an existing matrix provider, but we're replacing the
// local-to-device. In that scenario, we can't guarantee that we hit pixel centers anymore.
class SkOverrideDeviceMatrixProvider : public SkMatrixProvider {
public:
    SkOverrideDeviceMatrixProvider(const SkMatrix& localToDevice)
            : SkMatrixProvider(localToDevice, /*hitsPixelCenters=*/false) {}
};

class SkPostTranslateMatrixProvider : public SkMatrixProvider {
public:
    SkPostTranslateMatrixProvider(const SkMatrixProvider& parent, SkScalar dx, SkScalar dy)
            : SkMatrixProvider(SkM44::Translate(dx, dy) * parent.localToDevice44(),
                               parent.localToDeviceHitsPixelCenters()) {}
};

class SkPreConcatMatrixProvider : public SkMatrixProvider {
public:
    SkPreConcatMatrixProvider(const SkMatrixProvider& parent, const SkMatrix& preMatrix)
            : SkMatrixProvider(parent.localToDevice44() * SkM44(preMatrix),
                               parent.localToDeviceHitsPixelCenters()) {}
};

#endif
