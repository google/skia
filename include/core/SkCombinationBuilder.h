/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCombinationBuilder_DEFINED
#define SkCombinationBuilder_DEFINED

#include "include/core/SkTypes.h"

#ifdef SK_ENABLE_PRECOMPILE

#include <functional>
#include <memory>
#include <vector>
#include "include/core/SkBlendMode.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTileMode.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"

class SkArenaAllocWithReset;
class SkCombinationBuilder;
class SkKeyContext;
class SkOption;
class SkPaintParamsKeyBuilder;
class SkShaderCodeDictionary;
class SkUniquePaintParamsID;

namespace skgpu::graphite {
class Context;
}

enum class SkShaderType : uint32_t {
    kSolidColor,

    kLinearGradient,
    kRadialGradient,
    kSweepGradient,
    kConicalGradient,

    kLocalMatrix,
    kImage,
    kBlendShader,

    kLast          = kBlendShader
};

static constexpr int kSkShaderTypeCount = static_cast<int>(SkShaderType::kLast) + 1;

struct SkTileModePair {
    SkTileMode fX;
    SkTileMode fY;

    bool operator==(const SkTileModePair& other) const { return fX == other.fX && fY == other.fY; }
    bool operator!=(const SkTileModePair& other) const { return !(*this == other); }
};

// TODO: add SkShaderID and SkColorFilterID too
class SkBlenderID {
public:
    SkBlenderID() : fID(0) {}  // 0 is an invalid blender ID
    SkBlenderID(const SkBlenderID& src) : fID(src.fID) {}

    bool isValid() const { return fID > 0; }

    bool operator==(const SkBlenderID& other) const { return fID == other.fID; }

    SkBlenderID& operator=(const SkBlenderID& src) {
        fID = src.fID;
        return *this;
    }

private:
    friend class SkShaderCodeDictionary;     // for ctor and asUInt access
    friend class SkCombinationBuilder;       // for asUInt access

    SkBlenderID(uint32_t id) : fID(id) {}

    uint32_t asUInt() const { return fID; }

    uint32_t fID;
};

// When combination options are added to the combination builder an SkCombinationOption
// object is frequently returned. This allows options to be added, recursively, to the
// previously added options.
// Note: SkCombinationOptions are stable memory-wise so, once returned, they are valid
// until SkCombinationBuilder::reset is called.
class SkCombinationOption {
public:
    SkCombinationOption addChildOption(int childIndex, SkShaderType);

    SkCombinationOption addChildOption(int childIndex, SkShaderType,
                                       int minNumStops, int maxNumStops);

    SkCombinationOption addChildOption(int childIndex, SkShaderType,
                                       SkSpan<SkTileModePair> tileModes);

    bool isValid() const { return fDataInArena; }

private:
    friend class SkCombinationBuilder; // for ctor
    friend class CombinationBuilderTestAccess;

    SkCombinationOption(SkCombinationBuilder* builder, SkOption* dataInArena)
            : fBuilder(builder)
            , fDataInArena(dataInArena) {}

    SkShaderType type() const;
    int numChildSlots() const;
    SkDEBUGCODE(int epoch() const;)

    SkCombinationBuilder* fBuilder;
    SkOption* fDataInArena;
};

class SkCombinationBuilder {
public:
    enum class BlendModeGroup {
        kPorterDuff,         // [ kClear .. kScreen ]
        kAdvanced,           // [ kOverlay .. kMultiply ]
        kColorAware,         // [ kHue .. kLuminosity ]
        kAll
    };

    SkCombinationBuilder(SkShaderCodeDictionary*);
    ~SkCombinationBuilder();

    // Blend Modes
    void addOption(SkBlendMode);
    void addOption(SkBlendMode rangeStart, SkBlendMode rangeEnd); // inclusive
    void addOption(BlendModeGroup);

    // TODO: have this variant return an SkCombinationOption object
    void addOption(SkBlenderID);

    // Shaders
    SkCombinationOption addOption(SkShaderType);
    SkCombinationOption addOption(SkShaderType, int minNumStops, int maxNumStops);  // inclusive
    SkCombinationOption addOption(SkShaderType, SkSpan<SkTileModePair> tileModes);

    void reset();

private:
    friend class skgpu::graphite::Context;     // for access to 'buildCombinations'
    friend class SkCombinationOption;          // for 'addOptionInternal' and 'arena'
    friend class CombinationBuilderTestAccess; // for 'num*Combinations' and 'epoch'

    int numShaderCombinations() const;
    int numBlendModeCombinations() const;
    int numCombinations() {
        return this->numShaderCombinations() * this->numBlendModeCombinations();
    }

    // 'desiredCombination' must be less than numCombinations
    void createKey(const SkKeyContext&, int desiredCombination, SkPaintParamsKeyBuilder*);

#ifdef SK_DEBUG
    void dump() const;
    int epoch() const { return fEpoch; }
#endif

    SkArenaAllocWithReset* arena() { return fArena.get(); }

    template<typename T, typename... Args>
    SkOption* allocInArena(Args&&... args);

    SkOption* addOptionInternal(SkShaderType);
    SkOption* addOptionInternal(SkShaderType, int minNumStops, int maxNumStops);
    SkOption* addOptionInternal(SkShaderType, SkSpan<SkTileModePair> tileModes);

    void buildCombinations(SkShaderCodeDictionary*,
                           const std::function<void(SkUniquePaintParamsID)>&);

    SkShaderCodeDictionary* fDictionary;
    std::unique_ptr<SkArenaAllocWithReset> fArena;
    SkTArray<SkOption*> fShaderOptions;

    uint32_t fBlendModes;
    // TODO: store the SkBlender-based blenders in the arena
    SkTHashSet<SkBlenderID> fBlenders;

    SkDEBUGCODE(int fEpoch = 0;)
};

#endif // SK_ENABLE_PRECOMPILE

#endif // SkCombinationBuilder_DEFINED
