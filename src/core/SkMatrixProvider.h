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
    virtual ~SkMatrixProvider() {}

    virtual const SkMatrix& localToDevice() const = 0;
    virtual bool getLocalToMarker(uint32_t id, SkM44* localToMarker) const = 0;
};

class SkOverrideDeviceMatrixProvider : public SkMatrixProvider {
public:
    SkOverrideDeviceMatrixProvider(const SkMatrixProvider& parent, const SkMatrix& localToDevice)
        : fParent(parent)
        , fLocalToDevice(localToDevice) {}

    const SkMatrix& localToDevice() const override { return fLocalToDevice; }
    bool getLocalToMarker(uint32_t id, SkM44* localToMarker) const override {
        return fParent.getLocalToMarker(id, localToMarker);
    }

private:
    const SkMatrixProvider& fParent;
    SkMatrix                fLocalToDevice;
};

class SkPreConcatMatrixProvider : public SkMatrixProvider {
public:
    SkPreConcatMatrixProvider(const SkMatrixProvider& parent, const SkMatrix& preMatrix)
            : fParent(parent)
            , fPreMatrix(preMatrix) {
        fLocalToDevice.setConcat(fParent.localToDevice(), preMatrix);
    }

    const SkMatrix& localToDevice() const override { return fLocalToDevice; }
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
    SkMatrix                fPreMatrix;
    SkMatrix                fLocalToDevice;
};

class SkSimpleMatrixProvider : public SkMatrixProvider {
public:
    SkSimpleMatrixProvider(const SkMatrix& localToDevice) : fLocalToDevice(localToDevice) {}

    const SkMatrix& localToDevice() const override { return fLocalToDevice; }
    bool getLocalToMarker(uint32_t, SkM44*) const override { return false; }

private:
    SkMatrix fLocalToDevice;
};

#endif
