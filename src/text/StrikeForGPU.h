/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_StrikeForGPU_DEFINED
#define sktext_StrikeForGPU_DEFINED

#include "include/core/SkPath.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "src/core/SkGlyph.h"
#include "src/partition_alloc/raw_ptr_exclusion.h"

#include <memory>
#include <optional>
#include <variant>

class SkAutoDescriptor;
class SkDescriptor;
class SkDrawable;
class SkReadBuffer;
class SkStrike;
class SkStrikeCache;
class SkStrikeClient;
class SkStrikeSpec;
class SkWriteBuffer;

namespace sktext {
// -- SkStrikePromise ------------------------------------------------------------------------------
// SkStrikePromise produces an SkStrike when needed by GPU glyph rendering. In ordinary
// operation, it just wraps an SkStrike. When used for remote glyph cache operation, the promise is
// serialized to an SkDescriptor. When SkStrikePromise is deserialized, it uses the descriptor to
// look up the SkStrike.
//
// When deserializing some care must be taken; if the needed SkStrike is removed from the cache,
// then looking up using the descriptor will fail resulting in a deserialization failure. The
// Renderer/GPU system solves this problem by pinning all the strikes needed into the cache.
class SkStrikePromise {
public:
    SkStrikePromise() = delete;
    SkStrikePromise(const SkStrikePromise&) = delete;
    SkStrikePromise& operator=(const SkStrikePromise&) = delete;
    SkStrikePromise(SkStrikePromise&&);
    SkStrikePromise& operator=(SkStrikePromise&&);

    explicit SkStrikePromise(sk_sp<SkStrike>&& strike);
    explicit SkStrikePromise(const SkStrikeSpec& spec);

    // This only works when the GPU code is compiled in.
    static std::optional<SkStrikePromise> MakeFromBuffer(SkReadBuffer& buffer,
                                                         const SkStrikeClient* client,
                                                         SkStrikeCache* strikeCache);
    void flatten(SkWriteBuffer& buffer) const;

    // Do what is needed to return a strike.
    SkStrike* strike();

    // Release the strong reference to SkStrike so SkStrikeCache can evict it, while keeping
    // the descriptor intact for serialization without holding onto heavy font objects.
    void resetStrike();

    // Return a descriptor used to look up the SkStrike.
    const SkDescriptor& descriptor() const;

private:
    // Holds one of:
    // 1) The resolved SkStrike itself.
    // 2) An SkStrikeSpec used to construct the SkStrike on demand.
    // 3) A lightweight SkAutoDescriptor used to retrieve the descriptor for serialization or look
    //    up an existing strike in the cache without pinning font objects in memory.
    std::variant<sk_sp<SkStrike>,
                 std::unique_ptr<SkStrikeSpec>,
                 std::unique_ptr<SkAutoDescriptor>> fStrikeInfo;
};

// -- StrikeForGPU ---------------------------------------------------------------------------------
class StrikeForGPU : public SkRefCnt {
public:
    virtual void lock() = 0;
    virtual void unlock() = 0;

    // Generate a digest for a given packed glyph ID as drawn using the give action type.
    virtual SkGlyphDigest digestFor(skglyph::ActionType, SkPackedGlyphID) = 0;

    // Prepare the glyph to draw an image, and return if the image exists.
    virtual bool prepareForImage(SkGlyph*) = 0;

    // Prepare the glyph to draw a path, and return if the path exists.
    virtual bool prepareForPath(SkGlyph*) = 0;

    // Prepare the glyph to draw a drawable, and return if the drawable exists.
    virtual bool prepareForDrawable(SkGlyph*) = 0;


    virtual const SkDescriptor& getDescriptor() const = 0;

    virtual const SkGlyphPositionRoundingSpec& roundingSpec() const = 0;

    // Return a strike promise.
    virtual SkStrikePromise strikePromise() = 0;
};

// SkStrike::glyphIDsToPaths uses this struct to convert glyph ids to paths without clobbering
// glyph IDs.
struct IDOrPath {
    IDOrPath() = default;
    IDOrPath(SkGlyphID glyphID) : fGlyphID{glyphID} {}
    ~IDOrPath() = default;

    SkPath fPath;
    SkGlyphID fGlyphID{0};
};

// SkStrike::glyphIDsToDrawables uses this struct to convert glyph ids to drawables without
// clobbering glyph IDs.
struct IDOrDrawable {
    IDOrDrawable() = default;
    IDOrDrawable(SkGlyphID glyphID) : fGlyphID{glyphID} {}

    RAW_PTR_EXCLUSION SkDrawable* fDrawable{nullptr};
    SkGlyphID fGlyphID{0};
};

// -- StrikeMutationMonitor ------------------------------------------------------------------------
class StrikeMutationMonitor {
public:
    StrikeMutationMonitor(StrikeForGPU* strike);
    ~StrikeMutationMonitor();

private:
    StrikeForGPU* fStrike;
};

// -- StrikeForGPUCacheInterface -------------------------------------------------------------------
class StrikeForGPUCacheInterface {
public:
    virtual ~StrikeForGPUCacheInterface() = default;
    virtual sk_sp<StrikeForGPU> findOrCreateScopedStrike(const SkStrikeSpec& strikeSpec) = 0;
};
}  // namespace sktext
#endif  // sktext_StrikeForGPU_DEFINED
