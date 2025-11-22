/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/gpu/graphite/ClipAtlasManager.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkClipOp.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkSize.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkPoint_impl.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/ProxyCache.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Shape.h"

namespace skgpu::graphite {

static constexpr int kClipAtlasWidth = 2048;
static constexpr int kClipAtlasHeight = 2048;

ClipAtlasManager::ClipAtlasManager(Recorder* recorder)
        : fRecorder(recorder)
        , fPathKeyAtlasMgr(kClipAtlasWidth, kClipAtlasHeight,
                           /*plotWidth=*/kClipAtlasWidth/2, /*plotHeight=*/kClipAtlasHeight/2,
                           DrawAtlas::UseStorageTextures::kNo,
                           "PathKeyClipAtlas", recorder->priv().caps())
        // Examining the results from the top 20 or so webpages, the SaveRecord keyed clips
        // tend to be considerably smaller and rarer, so we use a smaller atlas here.
        , fSaveRecordKeyAtlasMgr(kClipAtlasWidth/2, kClipAtlasHeight/2,
                                 /*plotWidth=*/kClipAtlasWidth/2, /*plotHeight=*/kClipAtlasHeight/2,
                                 DrawAtlas::UseStorageTextures::kNo,
                                 "SaveRecordKeyClipAtlas", recorder->priv().caps()) {}

namespace {
// Needed to ensure that we have surrounding context, e.g. for inverse clips this would be solid.
constexpr int kEntryPadding = 1;

// If the first element to draw is an intersect, we clear to 0 and will draw it directly with
// coverage 1 (subsequent intersect elements will be inverse-filled and draw 0 outside).
// If the first element to draw is a difference, we clear to 1, and in all cases we draw the
// difference element directly with coverage 0.
SkAlpha initial_alpha_for_elements(const ClipStack::ElementList& elements) {
    SkASSERT(!elements.empty());
    return elements[0]->fOp == SkClipOp::kIntersect ? 0x00 : 0xFF;
}

void render_elements(RasterMaskHelper* helper, const ClipStack::ElementList& elements) {
    SkASSERT(!elements.empty());
    bool isFirst = true;
    for (const auto& ePtr : elements) {
        const auto& e = *ePtr;
        uint8_t alpha;
        bool invert;
        if (e.fOp == SkClipOp::kIntersect) {
            // Intersect modifies pixels outside of its geometry. If this is the first element,
            // we can draw directly with coverage 1 since we cleared to 0. Otherwise we draw the
            // inverse-filled shape with 0 coverage to erase everything outside the element.
            if (isFirst) {
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
        if (invert != e.fShape.inverted()) {
            Shape inverted(e.fShape);
            inverted.setInverted(invert);
            helper->drawClip(inverted, e.fLocalToDevice, alpha);
        } else {
            helper->drawClip(e.fShape, e.fLocalToDevice, alpha);
        }
        isFirst = false;
    }
}

} // anonymous namespace

sk_sp<TextureProxy> ClipAtlasManager::findOrCreateEntry(uint32_t stackRecordID,
                                                        const ClipStack::ElementList* elementList,
                                                        SkIRect maskDeviceBounds,
                                                        SkIPoint* outPos) {
    // For the ClipAtlas cache, we don't include the bounds in the key
    skgpu::UniqueKey maskKey;
    bool usesPathKey;
    // The keyBounds are the maskDeviceBounds relative to the full transformed mask. We use this
    // to ensure we capture the situation where the maskDeviceBounds are equal in two cases but
    // actually enclose different regions of the full mask due to a difference in integer
    // translation (which is not captured in the key) in the element transforms.
    SkIRect keyBounds;
    maskKey = GenerateClipMaskKey(stackRecordID, elementList, maskDeviceBounds,
                                  /*includeBounds=*/false, &keyBounds, &usesPathKey);

    sk_sp<TextureProxy> atlasProxy;
    if (usesPathKey) {
        atlasProxy = fPathKeyAtlasMgr.findOrCreateEntry(fRecorder, maskKey, elementList,
                                                        maskDeviceBounds, keyBounds, outPos);
    } else {
        atlasProxy = fSaveRecordKeyAtlasMgr.findOrCreateEntry(fRecorder, maskKey, elementList,
                                                              maskDeviceBounds, keyBounds, outPos);
    }
    if (atlasProxy) {
        return atlasProxy;
    }

    // We need to include the bounds in the key when using the ProxyCache
    maskKey = GenerateClipMaskKey(stackRecordID, elementList, maskDeviceBounds,
                                  /*includeBounds=*/true, &keyBounds, &usesPathKey);

    const struct ClipDrawContext {
        const ClipStack::ElementList* fElementList;
        SkIRect fMaskDeviceBounds;
    } context = {elementList, maskDeviceBounds};
    sk_sp<TextureProxy> proxy = fRecorder->priv().proxyCache()->findOrCreateCachedProxy(
            fRecorder, maskKey, &context,
            [](const void* ctx) {
                const ClipDrawContext* cdc = static_cast<const ClipDrawContext*>(ctx);
                auto translate =
                        -cdc->fMaskDeviceBounds.topLeft() + SkIVector{kEntryPadding, kEntryPadding};
                auto [bm, helper] =
                        RasterMaskHelper::Allocate(cdc->fMaskDeviceBounds.size(),
                                                   translate,
                                                   0,
                                                   initial_alpha_for_elements(*cdc->fElementList));

                render_elements(&helper, *cdc->fElementList);
                bm.setImmutable();
                return bm;
            });
    *outPos = { kEntryPadding, kEntryPadding };

    return proxy;
}

bool ClipAtlasManager::recordUploads(DrawContext* dc) {
    return fPathKeyAtlasMgr.recordUploads(dc, fRecorder) ||
           fSaveRecordKeyAtlasMgr.recordUploads(dc, fRecorder);
}

void ClipAtlasManager::compact() {
    fPathKeyAtlasMgr.compact(fRecorder);
    fSaveRecordKeyAtlasMgr.compact(fRecorder);
}

void ClipAtlasManager::freeGpuResources() {
    fPathKeyAtlasMgr.freeGpuResources(fRecorder);
    fSaveRecordKeyAtlasMgr.freeGpuResources(fRecorder);
}

void ClipAtlasManager::evictAtlases() {
    fPathKeyAtlasMgr.evictAll();
    fSaveRecordKeyAtlasMgr.evictAll();
}

//////////////////////////////////////////////////////////////////////////////////////////////

ClipAtlasManager::DrawAtlasMgr::DrawAtlasMgr(size_t width, size_t height,
                                             size_t plotWidth, size_t plotHeight,
                                             DrawAtlas::UseStorageTextures useStorageTextures,
                                             std::string_view label, const Caps* caps) {
    static constexpr SkColorType kColorType = kAlpha_8_SkColorType;
    fDrawAtlas = DrawAtlas::Make(kColorType,
                                 SkColorTypeBytesPerPixel(kColorType),
                                 width, height,
                                 plotWidth, plotHeight,
                                 /*generationCounter=*/this,
                                 caps->allowMultipleAtlasTextures() ?
                                         DrawAtlas::AllowMultitexturing::kYes :
                                         DrawAtlas::AllowMultitexturing::kNo,
                                 useStorageTextures,
                                 /*evictor=*/this,
                                 label);
    SkASSERT(fDrawAtlas);
    fKeyLists.resize(fDrawAtlas->numPlots() * fDrawAtlas->maxPages());
    for (int i = 0; i < fKeyLists.size(); ++i) {
        fKeyLists[i].reset();
    }
}

sk_sp<TextureProxy> ClipAtlasManager::DrawAtlasMgr::findOrCreateEntry(
            Recorder* recorder,
            const skgpu::UniqueKey& maskKey,
            const ClipStack::ElementList* elementList,
            SkIRect maskDeviceBounds,
            SkIRect keyBounds,
            SkIPoint* outPos) {
    MaskHashEntry* entry = fMaskCache.find(maskKey);
    while (entry) {
        // If this entry is large enough to contain the clip, use it
        if (entry->fBounds.contains(keyBounds)) {
            SkIPoint topLeft = entry->fLocator.topLeft();
            // We need to adjust the returned outPos to reflect the subset we're using
            SkIPoint subsetRelativePos = keyBounds.topLeft() - entry->fBounds.topLeft();
            *outPos = SkIPoint::Make(topLeft.x() + kEntryPadding + subsetRelativePos.x(),
                                     topLeft.y() + kEntryPadding + subsetRelativePos.y());
            fDrawAtlas->setLastUseToken(entry->fLocator,
                                        recorder->priv().tokenTracker()->nextFlushToken());
            return fDrawAtlas->getProxies()[entry->fLocator.pageIndex()];
        }
        entry = entry->fNext;
    }

    AtlasLocator locator;
    sk_sp<TextureProxy> proxy = this->addToAtlas(recorder, elementList, maskDeviceBounds, outPos,
                                                 &locator);
    if (!proxy) {
        return nullptr;
    }

    // Look up again (in case this entry got purged during addToAtlas())
    MaskHashEntry* entryList = fMaskCache.find(maskKey);

    // Add locator and bounds to MaskCache.
    if (entryList) {
        // Add new list entry to the end. This will sort them from smallest bounds to largest,
        // so that when we search above we'll pick the one with the smallest bounds that contains
        // the clip.
        MaskHashEntry* currEntry = entryList;
        while (currEntry->fNext) {
            currEntry = currEntry->fNext;
        }
        SkASSERT(currEntry);
        SkASSERT(currEntry->fNext == nullptr); // Should be at the end
        currEntry->fNext = new MaskHashEntry{keyBounds, locator, nullptr};
        ++fHashEntryCount;
    } else {
        MaskHashEntry newEntry{keyBounds, locator, nullptr};
        fMaskCache.set(maskKey, newEntry);
        ++fHashEntryCount;
    }

    // Add key to Plot's MaskKeyList.
    uint32_t index = fDrawAtlas->getListIndex(locator.plotLocator());
    MaskKeyEntry* keyEntry = new MaskKeyEntry{maskKey, keyBounds};
    fKeyLists[index].addToTail(keyEntry);
    ++fListEntryCount;

    SkASSERTF_RELEASE(fHashEntryCount == fListEntryCount,
                      "=ClipAtlas=: Entry counts don't match after add: %d %d",
                      fHashEntryCount, fListEntryCount);

    return proxy;
}

sk_sp<TextureProxy> ClipAtlasManager::DrawAtlasMgr::addToAtlas(
            Recorder* recorder,
            const ClipStack::ElementList* elementsForMask,
            SkIRect maskDeviceBounds,
            SkIPoint* outPos,
            AtlasLocator* locator) {
    // Render mask.
    SkISize maskSize = maskDeviceBounds.size();
    if (maskSize.isEmpty()) {
        return nullptr;
    }
    // Expand to include padding as well (so we clear correctly for inverse clip)
    maskSize.fWidth += 2*kEntryPadding;
    maskSize.fHeight += 2*kEntryPadding;

    // Request space in DrawAtlas, including padding
    DrawAtlas::ErrorCode errorCode = fDrawAtlas->addRect(recorder,
                                                         maskSize.width(),
                                                         maskSize.height(),
                                                         locator);
    if (errorCode != DrawAtlas::ErrorCode::kSucceeded) {
        return nullptr;
    }

    // Rasterize path to the record's pixmap with an inset.
    std::optional<SkColor> clearColor;
    if (SkAlpha alpha =  initial_alpha_for_elements(*elementsForMask); alpha != 0) {
        clearColor = alpha << 24;
    }
    SkPixmap pixmap = fDrawAtlas->prepForRender(*locator, 0, clearColor);

    auto translate = -maskDeviceBounds.topLeft() + SkIVector{kEntryPadding, kEntryPadding};
    RasterMaskHelper helper(pixmap, translate);
    render_elements(&helper, *elementsForMask);

    *outPos = locator->topLeft() + SkIVector{kEntryPadding, kEntryPadding};

    fDrawAtlas->setLastUseToken(*locator,
                                recorder->priv().tokenTracker()->nextFlushToken());

    return fDrawAtlas->getProxies()[locator->pageIndex()];
}

bool ClipAtlasManager::DrawAtlasMgr::recordUploads(DrawContext* dc, Recorder* recorder) {
    return (fDrawAtlas && !fDrawAtlas->recordUploads(dc, recorder));
}

void ClipAtlasManager::DrawAtlasMgr::evict(PlotLocator plotLocator) {
    // Remove all entries for this Plot from the MaskCache
    uint32_t index = fDrawAtlas->getListIndex(plotLocator);
    MaskKeyList::Iter iter;
    iter.init(fKeyLists[index], MaskKeyList::Iter::kHead_IterStart);
    MaskKeyEntry* currKeyEntry;
    while ((currKeyEntry = iter.get())) {
        iter.next();
        MaskHashEntry* currHashEntry = fMaskCache.find(currKeyEntry->fKey);
        SkASSERT(currHashEntry);
        MaskHashEntry* prevHashEntry = nullptr;
        while (currHashEntry) {
            if (currHashEntry->fBounds == currKeyEntry->fBounds) {
                // Remove entry from hash list
                if (prevHashEntry) {
                    prevHashEntry->fNext = currHashEntry->fNext;
                    delete currHashEntry;
                    --fHashEntryCount;
                } else if (currHashEntry->fNext) {
                    MaskHashEntry* next = currHashEntry->fNext;
                    currHashEntry->fBounds = next->fBounds;
                    currHashEntry->fLocator = next->fLocator;
                    currHashEntry->fNext = next->fNext;
                    delete next;
                    --fHashEntryCount;
                } else {
                    // Remove hash entry itself
                    fMaskCache.remove(currKeyEntry->fKey);
                    --fHashEntryCount;
                }
                break;
            }
            prevHashEntry = currHashEntry;
            currHashEntry = currHashEntry->fNext;
        }

        fKeyLists[index].remove(currKeyEntry);
        delete currKeyEntry;
        --fListEntryCount;
        SkASSERTF_RELEASE(fHashEntryCount == fListEntryCount,
                          "=ClipAtlas=: Entry counts don't match after delete: %d %d",
                          fHashEntryCount, fListEntryCount);
    }
}

void ClipAtlasManager::DrawAtlasMgr::evictAll() {
    fDrawAtlas->evictAllPlots();
    SkASSERT(fMaskCache.empty());
}

void ClipAtlasManager::DrawAtlasMgr::compact(Recorder* recorder) {
    auto tokenTracker = recorder->priv().tokenTracker();
    if (fDrawAtlas) {
        fDrawAtlas->compact(tokenTracker->nextFlushToken());
    }
}

void ClipAtlasManager::DrawAtlasMgr::freeGpuResources(Recorder* recorder) {
    auto tokenTracker = recorder->priv().tokenTracker();
    if (fDrawAtlas) {
        fDrawAtlas->freeGpuResources(tokenTracker->nextFlushToken());
    }
}

}  // namespace skgpu::graphite
