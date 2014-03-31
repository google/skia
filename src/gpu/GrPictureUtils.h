/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrPictureUtils_DEFINED
#define GrPictureUtils_DEFINED

#include "SkPicture.h"
#include "SkTDArray.h"

// This class encapsulates the GPU-backend-specific acceleration data
// for a single SkPicture
class GPUAccelData : public SkPicture::AccelData {
public:
    // Information about a given saveLayer in an SkPicture
    struct SaveLayerInfo {
        // The size of the saveLayer
        SkISize fSize;
        // The ID of this saveLayer in the picture. 0 is an invalid ID.
        size_t  fSaveLayerOpID;
        // The ID of the matching restore in the picture. 0 is an invalid ID.
        size_t  fRestoreOpID;
        // True if this saveLayer has at least one other saveLayer nested within it.
        // False otherwise.
        bool    fHasNestedLayers;
    };

    GPUAccelData(Key key) : INHERITED(key) { }

    void addSaveLayerInfo(const SaveLayerInfo& info) {
        SkASSERT(info.fSaveLayerOpID < info.fRestoreOpID);
        *fSaveLayerInfo.push() = info;
    }

    int numSaveLayers() const { return fSaveLayerInfo.count(); }

    const SaveLayerInfo& saveLayerInfo(int index) const {
        SkASSERT(index < fSaveLayerInfo.count());

        return fSaveLayerInfo[index];
    }

protected:
    SkTDArray<SaveLayerInfo> fSaveLayerInfo;

private:
    typedef SkPicture::AccelData INHERITED;
};

void GatherGPUInfo(SkPicture* pict, GPUAccelData* accelData);

#endif // GrPictureUtils_DEFINED
