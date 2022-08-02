/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_StrikeForGPU_DEFINED
#define sktext_StrikeForGPU_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkPoint.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "src/core/SkGlyph.h"

#include <memory>
#include <optional>
#include <variant>

class SkDescriptor;
class SkDrawableGlyphBuffer;
class SkReadBuffer;
class SkSourceGlyphBuffer;
class SkStrike;
class SkStrikeClient;
class SkStrikeSpec;
class SkWriteBuffer;
struct SkGlyphPositionRoundingSpec;
struct SkScalerContextEffects;

namespace sktext {
// -- StrikeForGPU ---------------------------------------------------------------------------------
class StrikeForGPU {
public:
    virtual ~StrikeForGPU() = default;
    virtual const SkDescriptor& getDescriptor() const = 0;

    // Returns the bounding rectangle of the accepted glyphs. Remember for device masks this
    // rectangle will be in device space, and for transformed masks this rectangle will be in
    // source space.
    virtual SkRect prepareForMaskDrawing(
                SkScalar strikeToSourceScale,
                SkDrawableGlyphBuffer* accepted,
                SkSourceGlyphBuffer* rejected) = 0;

    virtual SkRect prepareForSDFTDrawing(
                SkScalar strikeToSourceScale,
                SkDrawableGlyphBuffer* accepted,
                SkSourceGlyphBuffer* rejected) = 0;

    virtual void prepareForPathDrawing(
            SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) = 0;

    virtual void prepareForDrawableDrawing(
            SkDrawableGlyphBuffer* accepted, SkSourceGlyphBuffer* rejected) = 0;

    virtual const SkGlyphPositionRoundingSpec& roundingSpec() const = 0;

    // Used with SkScopedStrikeForGPU to take action at the end of a scope.
    virtual void onAboutToExitScope() = 0;

    // Return underlying SkStrike for building SubRuns while processing glyph runs.
    virtual sk_sp<SkStrike> getUnderlyingStrike() const = 0;

    // Return the maximum dimension of a span of glyphs.
    virtual SkScalar findMaximumGlyphDimension(SkSpan<const SkGlyphID> glyphs) = 0;

    struct Deleter {
        void operator()(StrikeForGPU* ptr) const {
            ptr->onAboutToExitScope();
        }
    };
};

// -- ScopedStrikeForGPU ---------------------------------------------------------------------------
using ScopedStrikeForGPU = std::unique_ptr<StrikeForGPU, StrikeForGPU::Deleter>;

// prepareForPathDrawing uses this union to convert glyph ids to paths.
union IDOrPath {
    IDOrPath() {}

    // PathOpSubmitter takes care of destroying the paths.
    ~IDOrPath() {}
    SkGlyphID fGlyphID;
    SkPath fPath;
};

// -- StrikeRef ------------------------------------------------------------------------------------
// Hold a ref to either a RemoteStrike or an SkStrike. Use either to flatten a descriptor, but
// when MakeFromBuffer runs look up the SkStrike associated with the descriptor.
class StrikeRef {
public:
    StrikeRef() = delete;
    StrikeRef(sk_sp<SkStrike>&& strike);
    StrikeRef(StrikeForGPU* strike);
    StrikeRef(const StrikeRef&) = delete;
    const StrikeRef& operator=(const StrikeRef&) = delete;
    StrikeRef(StrikeRef&&);
    StrikeRef& operator=(StrikeRef&&);

    // Flatten a descriptor into the buffer.
    void flatten(SkWriteBuffer& buffer) const;

    // Unflatten a descriptor, and create a StrikeRef holding an sk_sp<SkStrike>. The client is
    // used to do SkTypeFace id translation if passed in.
    static std::optional<StrikeRef> MakeFromBuffer(SkReadBuffer& buffer,
                                                   const SkStrikeClient* client);

    // getStrikeAndSetToNullptr can only be used when holding an SkStrike. This will only return
    // the SkStrike the first time, and will return nullptr on all future calls. Once this is
    // called, flatten can not be called.
    sk_sp<SkStrike> getStrikeAndSetToNullptr();

    StrikeForGPU* asStrikeForGPU();

private:
    friend class StrikeRefTestingPeer;
    // A StrikeRef can hold a pointer from a RemoteStrike which is of type SkStrikeForGPU,
    // or it can hold an actual ref to an actual SkStrike.
    std::variant<std::monostate, StrikeForGPU*, sk_sp<SkStrike>> fStrike;
};


// -- StrikeForGPUCacheInterface -------------------------------------------------------------------
class StrikeForGPUCacheInterface {
public:
    virtual ~StrikeForGPUCacheInterface() = default;
    virtual ScopedStrikeForGPU findOrCreateScopedStrike(const SkStrikeSpec& strikeSpec) = 0;
    virtual StrikeRef findOrCreateStrikeRef(const SkStrikeSpec& strikeSpec) = 0;
};
}  // namespace sktext
#endif  // sktext_StrikeForGPU_DEFINED
