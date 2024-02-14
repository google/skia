/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_TextBlob_DEFINED
#define sktext_gpu_TextBlob_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkTInternalLList.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/text/gpu/SubRunAllocator.h"
#include "src/text/gpu/SubRunContainer.h"

#include <cstddef>
#include <cstdint>
#include <tuple>

class SkCanvas;
struct SkPoint;
struct SkStrikeDeviceInfo;

namespace sktext {
class GlyphRunList;
class StrikeForGPUCacheInterface;
}

namespace sktext::gpu {
class Slug;

// -- TextBlob -----------------------------------------------------------------------------------
// A TextBlob contains a fully processed SkTextBlob, suitable for nearly immediate drawing
// on the GPU.  These are initially created with valid positions and colors, but with invalid
// texture coordinates.
//
// A TextBlob contains a number of SubRuns that are created in the blob's arena. Each SubRun
// tracks its own glyph and position data.
//
// In these classes, I'm trying to follow the convention about matrices and origins.
// * drawMatrix and drawOrigin - describes transformations for the current draw command.
// * positionMatrix - is equal to drawMatrix * [drawOrigin-as-translation-matrix]
// * initial Matrix - describes the combined initial matrix and origin the TextBlob was created
//                    with.
//
//
class TextBlob final : public SkRefCnt {
public:
    // Key is not used as part of a hash map, so the hash is never taken. It's only used in a
    // list search using operator =().
    struct Key {
        static std::tuple<bool, Key> Make(const GlyphRunList& glyphRunList,
                                          const SkPaint& paint,
                                          const SkMatrix& drawMatrix,
                                          const SkStrikeDeviceInfo& strikeDevice);
        uint32_t fUniqueID;
        // Color may affect the gamma of the mask we generate, but in a fairly limited way.
        // Each color is assigned to on of a fixed number of buckets based on its
        // luminance. For each luminance bucket there is a "canonical color" that
        // represents the bucket.  This functionality is currently only supported for A8
        SkColor fCanonicalColor;
        SkScalar fFrameWidth;
        SkScalar fMiterLimit;
        SkPixelGeometry fPixelGeometry;
        SkMaskFilterBase::BlurRec fBlurRec;
        uint32_t fScalerContextFlags;
        SkMatrix fPositionMatrix;
        // Below here fields are of size 1 byte.
        bool fHasSomeDirectSubRuns;
        bool fHasBlur;
        SkPaint::Style fStyle;
        SkPaint::Join fJoin;

        bool operator==(const Key& other) const;
    };

    SK_DECLARE_INTERNAL_LLIST_INTERFACE(TextBlob);

    // Make a TextBlob and its sub runs.
    static sk_sp<TextBlob> Make(const sktext::GlyphRunList& glyphRunList,
                                const SkPaint& paint,
                                const SkMatrix& positionMatrix,
                                SkStrikeDeviceInfo strikeDeviceInfo,
                                StrikeForGPUCacheInterface* strikeCache);

    TextBlob(SubRunAllocator&& alloc,
             SubRunContainerOwner subRuns,
             int totalMemorySize,
             SkColor initialLuminance);

    ~TextBlob() override;

    // Change memory management to handle the data after TextBlob, but in the same allocation
    // of memory. Only allow placement new.
    void operator delete(void* p);
    void* operator new(size_t);
    void* operator new(size_t, void* p);

    const Key& key() { return fKey; }

    void addKey(const Key& key);

    bool canReuse(const SkPaint& paint, const SkMatrix& positionMatrix) const;

    const Key& key() const;
    size_t size() const { return SkTo<size_t>(fSize); }

    void draw(SkCanvas*,
              SkPoint drawOrigin,
              const SkPaint& paint,
              const AtlasDrawDelegate&);

private:
    friend class TextBlobTools;
    // The allocator must come first because it needs to be destroyed last. Other fields of this
    // structure may have pointers into it.
    SubRunAllocator fAlloc;

    SubRunContainerOwner fSubRuns;

    // Overall size of this struct plus vertices and glyphs at the end.
    const int fSize;

    const SkColor fInitialLuminance;

    Key fKey;
};

sk_sp<sktext::gpu::Slug> MakeSlug(const SkMatrix& drawMatrix,
                                  const sktext::GlyphRunList& glyphRunList,
                                  const SkPaint& initialPaint,
                                  const SkPaint& drawingPaint,
                                  SkStrikeDeviceInfo strikeDeviceInfo,
                                  sktext::StrikeForGPUCacheInterface* strikeCache);
}  // namespace sktext::gpu
#endif  // sktext_gpu_TextBlob_DEFINED
