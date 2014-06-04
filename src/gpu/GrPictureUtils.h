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
        // True if the SaveLayerInfo is valid. False if either 'fOffset' is
        // invalid (due to a non-invertible CTM) or 'fPaint' is NULL (due
        // to a non-copyable paint).
        bool fValid;
        // The size of the saveLayer
        SkISize fSize;
        // The CTM in which this layer's draws must occur. It already incorporates
        // the translation needed to map the layer's top-left point to the origin.
        SkMatrix fCTM;
        // The offset that needs to be passed to drawBitmap to correctly
        // position the pre-rendered layer. It is in device space.
        SkIPoint fOffset;
        // The paint to use on restore. NULL if the paint was not copyable (and
        // thus that this layer should not be pulled forward).
        const SkPaint* fPaint;
        // The ID of this saveLayer in the picture. 0 is an invalid ID.
        size_t  fSaveLayerOpID;
        // The ID of the matching restore in the picture. 0 is an invalid ID.
        size_t  fRestoreOpID;
        // True if this saveLayer has at least one other saveLayer nested within it.
        // False otherwise.
        bool    fHasNestedLayers;
        // True if this saveLayer is nested within another. False otherwise.
        bool    fIsNested;
    };

    GPUAccelData(Key key) : INHERITED(key) { }

    virtual ~GPUAccelData() {
        for (int i = 0; i < fSaveLayerInfo.count(); ++i) {
            SkDELETE(fSaveLayerInfo[i].fPaint);
        }
    }

    void addSaveLayerInfo(const SaveLayerInfo& info) {
        SkASSERT(info.fSaveLayerOpID < info.fRestoreOpID);
        *fSaveLayerInfo.push() = info;
    }

    int numSaveLayers() const { return fSaveLayerInfo.count(); }

    const SaveLayerInfo& saveLayerInfo(int index) const {
        SkASSERT(index < fSaveLayerInfo.count());

        return fSaveLayerInfo[index];
    }

    // We may, in the future, need to pass in the GPUDevice in order to
    // incorporate the clip and matrix state into the key
    static SkPicture::AccelData::Key ComputeAccelDataKey();

private:
    SkTDArray<SaveLayerInfo> fSaveLayerInfo;

    typedef SkPicture::AccelData INHERITED;
};

void GatherGPUInfo(const SkPicture* pict, GPUAccelData* accelData);

#endif // GrPictureUtils_DEFINED
