/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef sktext_gpu_GlyphVector_DEFINED
#define sktext_gpu_GlyphVector_DEFINED

#include "include/core/SkSpan.h"
#include "src/core/SkGlyph.h"
#include "src/core/SkStrike.h"  // IWYU pragma: keep
#include "src/text/gpu/StrikeCache.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>

class SkReadBuffer;
class SkStrikeClient;
class SkWriteBuffer;

namespace sktext::gpu {
class SubRunAllocator;
class StrikeCache;

/**
 * Concepts used to describe backend-specific (Ganesh v. Graphite) data stored in GlyphVector.
 * The backend stores a type conforming to BackendData in each GlyphVector. This pertains to
 * the whole vector. The instance of the BackendData type is then used to convert from
 * SkPackedGlyphID to a backend-specific type conforming to GlyphType. The instances of
 * the type conforming to GlyphType actually overwrite the packed IDs. However, GlyphType
 * has a requirement to that it can convert back to packed id.
 *
 * Specific requirements for each type (other than size and alignment):
 * BackendData:
 * 1) Has a static method FindStrike that returns a sk_sp<T> where T is a subclass of
 *    TextStrikeBase.
 * 2) Has a member function makeGlyphFromID that takes SkPackedGlyphID and yields
 *    an instance of a type conforming to GlyphType.
 * GlyphType:
 * 1) Is trivially destructible. We could relax this by adding calls to the destructor
 *    if needed it proved useful.
 * 2) Has a method packedID() const that yields (something convertible to)
 *    SkPackedGlyphID.
 *
 * We use concepts rather than a base class for BackendData because it allows us to
 * require the static method on BackendData and it allows us to require that
 * makeFromGlyphID() yields any type that convert back to packedID() without requiring
 * a virtual function/vtable for GlyphType.
 */
namespace GlyphVector_Concepts {

// Both backends currently store pointer-sized types for glyphs and 88 is currently sufficient for
// Ganesh and Graphite's BackendData.
static constexpr size_t kMaxGlyphTypeSize = sizeof(void*);
static constexpr size_t kMaxBackendDataSize = 88;

template <typename T>
concept GlyphType = requires(const T& t) {
    requires sizeof(T)  <= kMaxGlyphTypeSize;
    // We store these in an array with kMaxGlyphTypeSize per entry. So that alignment must be
    // sufficient.
    requires alignof(T) <= kMaxGlyphTypeSize;

    std::is_trivially_destructible_v<T>;

    { t.packedID() } -> std::convertible_to<SkPackedGlyphID>;
};

template <typename T, template <typename...> class Template>
constexpr bool is_specialization_of_v = false;

template <template <typename...> class Template, typename... Args>
constexpr bool is_specialization_of_v<Template<Args...>, Template> = true;

template <typename T>
concept BackendData = requires(T& t,
                               StrikeCache* cache,
                               const SkStrikeSpec& spec,
                               SkPackedGlyphID id) {
    requires alignof(T) <= alignof(max_align_t);
    requires sizeof(T)  <= kMaxBackendDataSize;

    T::FindStrike(cache, spec);
    requires is_specialization_of_v<decltype(T::FindStrike(cache, spec)), sk_sp>;
    requires std::derived_from<typename decltype(T::FindStrike(cache, spec))::element_type, TextStrikeBase>;

    // Must have a makeGlyphFromID that returns a GlyphType
    { t.makeGlyphFromID(id) } -> GlyphType;
};
}  // namespace GlyphVector_Concepts

// -- GlyphVector ----------------------------------------------------------------------------------
// GlyphVector provides a way to delay the lookup of Glyphs until the code is running on the GPU
// in single threaded mode. The GlyphVector is created in a multi-threaded environment, but the
// StrikeCache is only single threaded (and must be single threaded because of the atlas).
//
// Once in the single-threaded GPU environment the glyph packed IDs are converted into a GPU backend
// specific entry type from which each glyph's atlas location can be determined.
class GlyphVector {
    using GlyphBytes = std::array<std::byte, GlyphVector_Concepts::kMaxGlyphTypeSize>;
    using BackendDataBytes = std::array<std::byte, GlyphVector_Concepts::kMaxBackendDataSize>;

    // Get the specific TextStrikeBase-derived strike class and GlyphType-conforming types yieled
    // by a BackendData-conforming type.
    template <GlyphVector_Concepts::BackendData B>
    using Strike = std::invoke_result_t<decltype(B::FindStrike), StrikeCache*, const SkStrikeSpec&>;

    template <GlyphVector_Concepts::BackendData B>
    using Glyph = decltype(std::declval<B>().makeGlyphFromID(SkPackedGlyphID{}));

public:
    static size_t Size(int numGlyphs) {
        SkASSERT(numGlyphs >= 0);
        return SkAlignTo(sizeof(GlyphVector), alignof(max_align_t)) + sizeof(GlyphBytes) * numGlyphs;
    }

    GlyphVector(SkStrikePromise&& strikePromise, SkSpan<GlyphBytes> glyphs);

    // To move in to AtlasSubRun constructor
    GlyphVector(GlyphVector&&);

    GlyphVector(const GlyphVector&&)           = delete;
    GlyphVector& operator=(const GlyphVector&) = delete;
    GlyphVector& operator=(GlyphVector&&)      = delete;

    ~GlyphVector();

    static GlyphVector Make(SkStrikePromise&& promise,
                            SkSpan<const SkPackedGlyphID> glyphs,
                            SubRunAllocator* alloc);

    int glyphCount() const { return SkToInt(fGlyphs.size()); }

    static std::optional<GlyphVector> MakeFromBuffer(SkReadBuffer& buffer,
                                                     const SkStrikeClient* strikeClient,
                                                     SubRunAllocator* alloc);
    void flatten(SkWriteBuffer& buffer) const;

    // This doesn't need to include sizeof(GlyphVector) because this is embedded in each of
    // the sub runs.
    int unflattenSize() const { return Size(fGlyphs.size()); }

    bool hasBackendData() const {
        SkASSERT(SkToBool(fBackendDataReleaser) == SkToBool(fGetGlyphID));
        return SkToBool(fBackendDataReleaser);
    }

    template <GlyphVector_Concepts::BackendData B> const B& accessBackendData() const {
        SkASSERT(this->hasBackendData());
        return *reinterpret_cast<const B*>(fBackendDataBytes.data());
    }

    template <GlyphVector_Concepts::BackendData B> B& accessBackendData() {
        SkASSERT(this->hasBackendData());
        return *reinterpret_cast<B*>(fBackendDataBytes.data());
    }

    template <GlyphVector_Concepts::GlyphType G> SkSpan<const G> accessBackendGlyphs() const {
        SkASSERT(this->hasBackendData());
        auto* first = reinterpret_cast<const G*>(fGlyphs.front().data());
        return {first, fGlyphs.size()};
    }

    template <GlyphVector_Concepts::BackendData B>
    void initBackendData(StrikeCache* cache, auto&&... args)
        requires std::is_constructible_v<B, Strike<B>, decltype(args)...> {
        SkASSERT(!this->hasBackendData());
        SkASSERT(cache);

        SkStrike* strike = fStrikePromise.strike();
        const SkStrikeSpec& spec = strike->strikeSpec();
        auto backendStrike = B::FindStrike(cache, spec);

        auto backendData = new (fBackendDataBytes.data()) B{
            std::move(backendStrike),
            std::forward<decltype(args)>(args)...
        };

        using G = Glyph<B>;

        for (auto& g : fGlyphs) {
            auto id = *reinterpret_cast<SkPackedGlyphID*>(g.data());
            new (g.data()) G{backendData->makeGlyphFromID(id)};
        }

        fBackendDataReleaser = [](std::byte* p) { reinterpret_cast<B*>(p)->~B(); };

        fGetGlyphID = [](const std::byte* b) {
            auto* g = reinterpret_cast<const G*>(b);
            return g->packedID();
        };

        strike->verifyPinnedStrike();
        fStrikePromise.resetStrike();
    }

private:
    friend class GlyphVectorTestingPeer;

    using Releaser = void(std::byte*);

    using GetGlyphID = SkPackedGlyphID(const std::byte*);

    alignas(max_align_t) BackendDataBytes fBackendDataBytes;

    Releaser*   fBackendDataReleaser = nullptr;
    GetGlyphID* fGetGlyphID          = nullptr;

    SkStrikePromise fStrikePromise;

    SkSpan<GlyphBytes> fGlyphs;
};
}  // namespace sktext::gpu
#endif  // sktext_gpu_GlyphVector_DEFINED
