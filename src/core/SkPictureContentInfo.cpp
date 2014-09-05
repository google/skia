/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPaint.h"
#include "SkPathEffect.h"
#include "SkPictureContentInfo.h"

bool SkPictureContentInfo::suitableForGpuRasterization(GrContext* context, const char **reason,
                                                       int sampleCount) const {
    // TODO: the heuristic used here needs to be refined
    static const int kNumPaintWithPathEffectUsesTol = 1;
    static const int kNumAAConcavePaths = 5;

    SkASSERT(fNumAAHairlineConcavePaths <= fNumAAConcavePaths);

    int numNonDashedPathEffects = fNumPaintWithPathEffectUses -
                                  fNumFastPathDashEffects;

    bool suitableForDash = (0 == fNumPaintWithPathEffectUses) ||
                           (numNonDashedPathEffects < kNumPaintWithPathEffectUsesTol
                            && 0 == sampleCount);

    bool ret = suitableForDash &&
                    (fNumAAConcavePaths - fNumAAHairlineConcavePaths)
                    < kNumAAConcavePaths;
    if (!ret && reason) {
        if (!suitableForDash) {
            if (0 != sampleCount) {
                *reason = "Can't use multisample on dash effect.";
            } else {
                *reason = "Too many non dashed path effects.";
            }
        } else if ((fNumAAConcavePaths - fNumAAHairlineConcavePaths)
                    >= kNumAAConcavePaths) {
            *reason = "Too many anti-aliased concave paths.";
        } else {
            *reason = "Unknown reason for GPU unsuitability.";
        }
    }
    return ret;
}

void SkPictureContentInfo::onDrawPoints(size_t count, const SkPaint& paint) {
    if (paint.getPathEffect() != NULL) {
        SkPathEffect::DashInfo info;
        SkPathEffect::DashType dashType = paint.getPathEffect()->asADash(&info);
        if (2 == count && SkPaint::kRound_Cap != paint.getStrokeCap() &&
            SkPathEffect::kDash_DashType == dashType && 2 == info.fCount) {
            ++fNumFastPathDashEffects;
        }
    }
}

void SkPictureContentInfo::onDrawPath(const SkPath& path, const SkPaint& paint) {
    if (paint.isAntiAlias() && !path.isConvex()) {
        ++fNumAAConcavePaths;

        if (SkPaint::kStroke_Style == paint.getStyle() && 0 == paint.getStrokeWidth()) {
            ++fNumAAHairlineConcavePaths;
        }
    }
}

void SkPictureContentInfo::onAddPaintPtr(const SkPaint* paint) {
    if (paint && paint->getPathEffect()) {
        ++fNumPaintWithPathEffectUses;
    }
}

void SkPictureContentInfo::onSaveLayer() {
    *fSaveStack.append() = kSaveLayer_Flag;
}

void SkPictureContentInfo::onSave() {
    *fSaveStack.append() = kSave_Flag;
}

void SkPictureContentInfo::onRestore() {
    SkASSERT(fSaveStack.count() > 0);

    bool containedSaveLayer = fSaveStack.top() & kContainedSaveLayer_Flag;

    if (fSaveStack.top() & kSaveLayer_Flag) {
        ++fNumLayers;
        if (containedSaveLayer) {
            ++fNumInteriorLayers;
        } else {
            ++fNumLeafLayers;
        }
        containedSaveLayer = true;
    }

    fSaveStack.pop();

    if (containedSaveLayer && fSaveStack.count() > 0) {
        fSaveStack.top() |= kContainedSaveLayer_Flag;
    }
}

void SkPictureContentInfo::rescindLastSave() {
    SkASSERT(fSaveStack.count() > 0);
    SkASSERT(fSaveStack.top() & kSave_Flag);

    bool containedSaveLayer = fSaveStack.top() & kContainedSaveLayer_Flag;

    fSaveStack.pop();

    if (containedSaveLayer && fSaveStack.count() > 0) {
        fSaveStack.top() |= kContainedSaveLayer_Flag;
    }
}

void SkPictureContentInfo::rescindLastSaveLayer() {
    SkASSERT(fSaveStack.count() > 0);
    SkASSERT(fSaveStack.top() & kSaveLayer_Flag);

    bool containedSaveLayer = fSaveStack.top() & kContainedSaveLayer_Flag;

    fSaveStack.pop();

    if (containedSaveLayer && fSaveStack.count() > 0) {
        fSaveStack.top() |= kContainedSaveLayer_Flag;
    }
}

void SkPictureContentInfo::set(const SkPictureContentInfo& src) {
    fNumOperations = src.fNumOperations;
    fNumTexts = src.fNumTexts;
    fNumPaintWithPathEffectUses = src.fNumPaintWithPathEffectUses;
    fNumFastPathDashEffects = src.fNumFastPathDashEffects;
    fNumAAConcavePaths = src.fNumAAConcavePaths;
    fNumAAHairlineConcavePaths = src.fNumAAHairlineConcavePaths;
    fNumLayers = src.fNumLayers;
    fNumInteriorLayers = src.fNumInteriorLayers;
    fNumLeafLayers = src.fNumLeafLayers;
    fSaveStack = src.fSaveStack;
}

void SkPictureContentInfo::reset() {
    fNumOperations = 0;
    fNumTexts = 0;
    fNumPaintWithPathEffectUses = 0;
    fNumFastPathDashEffects = 0;
    fNumAAConcavePaths = 0;
    fNumAAHairlineConcavePaths = 0;
    fNumLayers = 0;
    fNumInteriorLayers = 0;
    fNumLeafLayers = 0;
    fSaveStack.rewind();
}

void SkPictureContentInfo::swap(SkPictureContentInfo* other) {
    SkTSwap(fNumOperations, other->fNumOperations);
    SkTSwap(fNumTexts, other->fNumTexts);
    SkTSwap(fNumPaintWithPathEffectUses, other->fNumPaintWithPathEffectUses);
    SkTSwap(fNumFastPathDashEffects, other->fNumFastPathDashEffects);
    SkTSwap(fNumAAConcavePaths, other->fNumAAConcavePaths);
    SkTSwap(fNumAAHairlineConcavePaths, other->fNumAAHairlineConcavePaths);
    SkTSwap(fNumLayers, other->fNumLayers);
    SkTSwap(fNumInteriorLayers, other->fNumInteriorLayers);
    SkTSwap(fNumLeafLayers, other->fNumLeafLayers);
    fSaveStack.swap(other->fSaveStack);
}
