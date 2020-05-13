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

class SkMatrixProvider {
public:
    SkMatrixProvider(const SkM44& localToDevice44, const SkMatrix& localToDevice)
        : fLocalToDevice44(localToDevice44)
        , fLocalToDevice(localToDevice) {}

    virtual ~SkMatrixProvider() {}

    // These should return the "same" matrix, as either a 3x3 or 4x4. Most sites in Skia still
    // call localToDevice, and operate on SkMatrix.
    const SkM44& localToDevice44() const { return fLocalToDevice44; }
    const SkMatrix& localToDevice() const { return fLocalToDevice; }

    virtual bool getLocalToMarker(uint32_t id, SkM44* localToMarker) const = 0;

private:
    const SkM44&    fLocalToDevice44;
    const SkMatrix& fLocalToDevice;  // Cached SkMatrix version of above, for legacy usage
};

// Utility class for provider sub-classes that need embedded storage of local-to-device matrices.
class SkSTMatrixProvider : public SkMatrixProvider {
public:
    SkSTMatrixProvider(const SkM44& localToDevice44)
        : SkMatrixProvider(fLocalToDevice44Storage, fLocalToDeviceStorage)
        , fLocalToDevice44Storage(localToDevice44)
        , fLocalToDeviceStorage(localToDevice44.asM33()) {}

    SkSTMatrixProvider(const SkMatrix& localToDevice)
        : SkMatrixProvider(fLocalToDevice44Storage, fLocalToDeviceStorage)
        , fLocalToDevice44Storage(localToDevice)
        , fLocalToDeviceStorage(localToDevice) {}

private:
    const SkM44    fLocalToDevice44Storage;
    const SkMatrix fLocalToDeviceStorage;
};

class SkPostConcatMatrixProvider : public SkSTMatrixProvider {
public:
    SkPostConcatMatrixProvider(const SkMatrixProvider& parent, const SkMatrix& postMatrix)
            : SkSTMatrixProvider(SkM44(postMatrix) * parent.localToDevice44())
            , fParent(parent) {}

    // Assume that the post-matrix doesn't apply to any marked matrices
    bool getLocalToMarker(uint32_t id, SkM44* localToMarker) const override {
        return fParent.getLocalToMarker(id, localToMarker);
    }

private:
    const SkMatrixProvider& fParent;
};

class SkPreConcatMatrixProvider : public SkSTMatrixProvider {
public:
    SkPreConcatMatrixProvider(const SkMatrixProvider& parent, const SkMatrix& preMatrix)
            : SkSTMatrixProvider(parent.localToDevice44() * SkM44(preMatrix))
            , fParent(parent)
            , fPreMatrix(preMatrix) {}

    // Marked matrices are local-to-<something>, so they should also be pre-multiplied
    bool getLocalToMarker(uint32_t id, SkM44* localToMarker) const override {
        if (fParent.getLocalToMarker(id, localToMarker)) {
            if (localToMarker) {
                localToMarker->preConcat(fPreMatrix);
            }
            return true;
        }
        return false;
    }

private:
    const SkMatrixProvider& fParent;
    const SkMatrix          fPreMatrix;
};

class SkSimpleMatrixProvider : public SkSTMatrixProvider {
public:
    SkSimpleMatrixProvider(const SkMatrix& localToDevice)
        : SkSTMatrixProvider(localToDevice) {}

    bool getLocalToMarker(uint32_t, SkM44*) const override { return false; }
};

#endif
