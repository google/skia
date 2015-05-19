/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLayerInfo_DEFINED
#define SkLayerInfo_DEFINED

#include "SkBigPicture.h"
#include "SkTArray.h"

// This class stores information about the saveLayer/restore pairs found
// within an SkPicture. It is used by Ganesh to perform layer hoisting.
class SkLayerInfo : public SkBigPicture::AccelData {
public:
    // Information about a given saveLayer/restore block in an SkPicture
    class BlockInfo {
    public:
        BlockInfo() : fPicture(NULL), fPaint(NULL), fKey(NULL), fKeySize(0) {}
        ~BlockInfo() { SkSafeUnref(fPicture); SkDELETE(fPaint); SkDELETE_ARRAY(fKey); }

        // The picture owning the layer. If the owning picture is the top-most
        // one (i.e., the picture for which this SkLayerInfo was created) then
        // this pointer is NULL. If it is a nested picture then the pointer
        // is non-NULL and owns a ref on the picture.
        const SkPicture* fPicture;
        // The device space bounds of this layer.
        SkRect fBounds;
        // If not-empty, the optional bounds parameter passed in to the saveLayer
        // call.
        SkRect fSrcBounds;
        // The pre-matrix begins as the identity and accumulates the transforms
        // of the containing SkPictures (if any). This matrix state has to be
        // part of the initial matrix during replay so that it will be
        // preserved across setMatrix calls.
        SkMatrix fPreMat;
        // The matrix state (in the leaf picture) in which this layer's draws
        // must occur. It will/can be overridden by setMatrix calls in the
        // layer itself. It does not include the translation needed to map the
        // layer's top-left point to the origin (which must be part of the
        // initial matrix).
        SkMatrix fLocalMat;
        // The paint to use on restore. Can be NULL since it is optional.
        const SkPaint* fPaint;
        // The index of this saveLayer in the picture.
        size_t  fSaveLayerOpID;
        // The index of the matching restore in the picture.
        size_t  fRestoreOpID;
        // True if this saveLayer has at least one other saveLayer nested within it.
        // False otherwise.
        bool    fHasNestedLayers;
        // True if this saveLayer is nested within another. False otherwise.
        bool    fIsNested;
        // The variable length key for this saveLayer block. It stores the
        // thread of drawPicture and saveLayer operation indices that lead to this
        // saveLayer (including its own op index). The BlockInfo owns this memory.
        unsigned* fKey;
        int     fKeySize;  // # of ints
    };

    SkLayerInfo() {}

    BlockInfo& addBlock() { return fBlocks.push_back(); }

    int numBlocks() const { return fBlocks.count(); }

    const BlockInfo& block(int index) const {
        SkASSERT(index < fBlocks.count());

        return fBlocks[index];
    }

private:
    SkTArray<BlockInfo, true> fBlocks;

    typedef SkBigPicture::AccelData INHERITED;
};

#endif // SkLayerInfo_DEFINED
