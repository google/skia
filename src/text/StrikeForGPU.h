/*
 * Copyright 2019 Google Inc.
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

#include <memory>
#include <optional>
#include <variant>

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

    // Reset the sk_sp<SkStrike> to nullptr.
    void resetStrike();

    // Return a descriptor used to look up the SkStrike.
    const SkDescriptor& descriptor() const;

private:
    std::variant<sk_sp<SkStrike>, std::unique_ptr<SkStrikeSpec>> fStrikeOrSpec;
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

// prepareForPathDrawing uses this union to convert glyph ids to paths.
union IDOrPath {
    IDOrPath() {}
    IDOrPath(SkGlyphID glyphID) : fGlyphID{glyphID} {}

    // PathOpSubmitter takes care of destroying the paths.
    ~IDOrPath() {}
    SkGlyphID fGlyphID;
    SkPath fPath;
};

// prepareForDrawableDrawing uses this union to convert glyph ids to drawables.
union IDOrDrawable {
    SkGlyphID fGlyphID;
    SkDrawable* fDrawable;
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
