/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/ClipAtlasManager.h"

#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkFixed.h"
#include "src/base/SkFloatBits.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {

ClipAtlasManager::ClipAtlasManager(Recorder* recorder) : fRecorder(recorder) {
    static constexpr SkColorType kColorType = kAlpha_8_SkColorType;
    static constexpr int kWidth = 2048;
    static constexpr int kHeight = 2048;

    const Caps* caps = recorder->priv().caps();
    fDrawAtlas = DrawAtlas::Make(kColorType,
                                 SkColorTypeBytesPerPixel(kColorType),
                                 kWidth, kHeight,
                                 /*plotWidth=*/kWidth, /*plotHeight=*/kHeight,
                                 /*generationCounter=*/this,
                                 caps->allowMultipleAtlasTextures() ?
                                         DrawAtlas::AllowMultitexturing::kYes :
                                         DrawAtlas::AllowMultitexturing::kNo,
                                 DrawAtlas::UseStorageTextures::kNo,
                                 /*evictor=*/this,
                                 "ClipAtlas");
    SkASSERT(fDrawAtlas);
    fKeyLists.resize(fDrawAtlas->numPlots() * fDrawAtlas->maxPages());
    for (int i = 0; i < fKeyLists.size(); ++i) {
        fKeyLists[i].reset();
    }
}

namespace {
// TODO: is this necessary for clips?
constexpr int kEntryPadding = 1;
}  // namespace

const TextureProxy* ClipAtlasManager::findOrCreateEntry(uint32_t stackRecordID,
                                                        const ClipStack::ElementList* elementList,
                                                        const Rect& bounds,
                                                        skvx::half2* outPos) {
    skgpu::UniqueKey maskKey = GenerateClipMaskKey(stackRecordID, elementList);

    MaskHashArray* cachedArray = fMaskCache.find(maskKey);
    if (cachedArray) {
        for (int i = 0; i < cachedArray->size(); ++i) {
            MaskHashEntry& entry = (*cachedArray)[i];
            // We can reuse a clip mask if has the same key and our bounds is contained in it
            if (entry.fBounds.contains(bounds)) {
                SkIPoint topLeft = entry.fLocator.topLeft();
                // We need to adjust the returned outPos to reflect the subset we're using
                skvx::float2 subsetRelativePos = bounds.topLeft() - entry.fBounds.topLeft();
                *outPos = skvx::half2(topLeft.x() + kEntryPadding + subsetRelativePos.x(),
                                      topLeft.y() + kEntryPadding + subsetRelativePos.y());
                fDrawAtlas->setLastUseToken(entry.fLocator,
                                            fRecorder->priv().tokenTracker()->nextFlushToken());
                return fDrawAtlas->getProxies()[entry.fLocator.pageIndex()].get();
            }
        }
    }

    AtlasLocator locator;
    const TextureProxy* proxy = this->addToAtlas(elementList, bounds, outPos, &locator);
    if (!proxy) {
        return nullptr;
    }

    // Add locator and bounds to MaskCache.
    if (cachedArray) {
        cachedArray->push_back({bounds, locator});
    } else {
        MaskHashArray initialArray;
        initialArray.push_back({bounds, locator});
        fMaskCache.set(maskKey, initialArray);
    }
    // Add key to Plot's MaskKeyList.
    uint32_t index = fDrawAtlas->getListIndex(locator.plotLocator());
    MaskKeyEntry* keyEntry = new MaskKeyEntry();
    keyEntry->fKey = maskKey;
    keyEntry->fBounds = bounds;
    fKeyLists[index].addToTail(keyEntry);

    return proxy;
}

// Copied and modified from Ganesh ClipStack
void draw_to_sw_mask(RasterMaskHelper* helper,
                     const ClipStack::Element& e,
                     bool clearMask,
                     const SkIRect& resultBounds) {
    // If the first element to draw is an intersect, we clear to 0 and will draw it directly with
    // coverage 1 (subsequent intersect elements will be inverse-filled and draw 0 outside).
    // If the first element to draw is a difference, we clear to 1, and in all cases we draw the
    // difference element directly with coverage 0.
    if (clearMask) {
        helper->clear(e.fOp == SkClipOp::kIntersect ? 0x00 : 0xFF, resultBounds);
    }

    uint8_t alpha;
    bool invert;
    if (e.fOp == SkClipOp::kIntersect) {
        // Intersect modifies pixels outside of its geometry. If this isn't the first op, we
        // draw the inverse-filled shape with 0 coverage to erase everything outside the element
        // But if we are the first element, we can draw directly with coverage 1 since we
        // cleared to 0.
        if (clearMask) {
            alpha = 0xFF;
            invert = false;
        } else {
            alpha = 0x00;
            invert = true;
        }
    } else {
        // For difference ops, can always just subtract the shape directly by drawing 0 coverage
        SkASSERT(e.fOp == SkClipOp::kDifference);
        alpha = 0x00;
        invert = false;
    }

    // Draw the shape; based on how we've initialized the buffer and chosen alpha+invert,
    // every element is drawn with the kReplace_Op
    if (invert) {
        // Must invert the path
        SkASSERT(!e.fShape.inverted());
        // TODO: this is an extra copy effectively, just so we can toggle inversion; would be
        // better perhaps to just call a drawPath() since we know it'll use path rendering w/
        // the inverse fill type.
        Shape inverted(e.fShape);
        inverted.setInverted(true);
        helper->drawClip(inverted, e.fLocalToDevice, alpha, resultBounds);
    } else {
        helper->drawClip(e.fShape, e.fLocalToDevice, alpha, resultBounds);
    }
}

const TextureProxy* ClipAtlasManager::addToAtlas(const ClipStack::ElementList* elementsForMask,
                                                 const Rect& bounds,
                                                 skvx::half2* outPos,
                                                 AtlasLocator* locator) {
    // Render mask.
    skvx::float2 maskSize = bounds.size();
    if (!all(maskSize)) {
        return nullptr;
    }

    SkIRect iShapeBounds = SkIRect::MakeXYWH(0, 0, maskSize.x(), maskSize.y());
    // Outset to take padding into account
    SkIRect iAtlasBounds = iShapeBounds.makeOutset(kEntryPadding, kEntryPadding);

    // Request space in DrawAtlas.
    DrawAtlas::ErrorCode errorCode = fDrawAtlas->addRect(fRecorder,
                                                         iAtlasBounds.width(),
                                                         iAtlasBounds.height(),
                                                         locator);
    if (errorCode != DrawAtlas::ErrorCode::kSucceeded) {
        return nullptr;
    }
    SkIPoint topLeft = locator->topLeft();
    *outPos = skvx::half2(topLeft.x() + kEntryPadding, topLeft.y() + kEntryPadding);

    // Rasterize path to backing pixmap.
    // This pixmap will be the size of the Plot that contains the given rect, not the entire atlas,
    // and hence the position we render at will be relative to that Plot.
    // The value of outPos is relative to the entire texture, to be used for texture coords.
    SkAutoPixmapStorage dst;
    SkIPoint renderPos = fDrawAtlas->prepForRender(*locator, &dst);

    // This will remove the base integer translation from each element's transform
    Rect iBounds = bounds.makeRoundOut();
    RasterMaskHelper helper(&dst);
    if (!helper.init(fDrawAtlas->plotSize(), iBounds.topLeft())) {
        return nullptr;
    }

    // Offset to plot location and draw
    iShapeBounds.offset(renderPos.x() + kEntryPadding, renderPos.y() + kEntryPadding);

    SkASSERT(elementsForMask->size() > 0);
    for (int i = 0; i < elementsForMask->size(); ++i) {
        draw_to_sw_mask(&helper, *(*elementsForMask)[i], i == 0, iShapeBounds);
    }

    fDrawAtlas->setLastUseToken(*locator,
                                fRecorder->priv().tokenTracker()->nextFlushToken());

    return fDrawAtlas->getProxies()[locator->pageIndex()].get();
}

bool ClipAtlasManager::recordUploads(DrawContext* dc) {
    return (fDrawAtlas && !fDrawAtlas->recordUploads(dc, fRecorder));
}

void ClipAtlasManager::evict(PlotLocator plotLocator) {
    // Remove all entries for this Plot from the MaskCache
    uint32_t index = fDrawAtlas->getListIndex(plotLocator);
    MaskKeyList::Iter iter;
    iter.init(fKeyLists[index], MaskKeyList::Iter::kHead_IterStart);
    MaskKeyEntry* currEntry;
    while ((currEntry = iter.get())) {
        iter.next();
        MaskHashArray* cachedArray = fMaskCache.find(currEntry->fKey);
        // Remove this entry from the hashed array
        for (int i = cachedArray->size()-1; i >=0 ; --i) {
            MaskHashEntry& entry = (*cachedArray)[i];
            if (entry.fBounds == currEntry->fBounds) {
                (*cachedArray).removeShuffle(i);
                break;
            }
        }
        // If we removed the last one, remove the hash entry
        if (cachedArray->size() == 0) {
            fMaskCache.remove(currEntry->fKey);
        }
        fKeyLists[index].remove(currEntry);
        delete currEntry;
    }
}

void ClipAtlasManager::evictAtlases() {
    fDrawAtlas->evictAllPlots();
    SkASSERT(fMaskCache.empty());
}

void ClipAtlasManager::compact(bool forceCompact) {
    auto tokenTracker = fRecorder->priv().tokenTracker();
    if (fDrawAtlas) {
        fDrawAtlas->compact(tokenTracker->nextFlushToken(), forceCompact);
    }
}

}  // namespace skgpu::graphite
