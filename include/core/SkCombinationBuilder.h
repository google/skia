/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCombinationBuilder_DEFINED
#define SkCombinationBuilder_DEFINED

#include <functional>
#include <memory>
#include <vector>
#include "include/core/SkBlendMode.h"
#include "include/core/SkTileMode.h"

class SkArenaAllocWithReset;
class SkPaintCombinations;
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
    kBlendShader
};

// TODO: remove this class
struct SkShaderCombo {
    SkShaderCombo() {}
    SkShaderCombo(std::vector<SkShaderType> types,
                  std::vector<SkTileMode> tileModes)
            : fTypes(std::move(types))
            , fTileModes(std::move(tileModes)) {
    }
    std::vector<SkShaderType> fTypes;
    std::vector<SkTileMode> fTileModes;
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
    friend class SkShaderCodeDictionary;   // for ctor and asUInt access
    friend class SkCombinationBuilder;       // for asUInt access
    friend class SkPaintCombinations;        // for asUInt access

    SkBlenderID(uint32_t id) : fID(id) {}

    uint32_t asUInt() const { return fID; }

    uint32_t fID;
};

class SkCombinationBuilder {
public:
    enum class BlendModeGroup {
        kPorterDuff,         // [ kClear .. kScreen ]
        kAdvanced,           // [ kOverlay .. kMultiply ]
        kColorAware,         // [ kHue .. kLuminosity ]
        kAll
    };

#ifdef SK_GRAPHITE_ENABLED
    SkCombinationBuilder(skgpu::graphite::Context*);
#else
    SkCombinationBuilder(SkShaderCodeDictionary*);
#endif

    // Blend Modes
    void add(SkBlendMode);
    void add(SkBlendMode rangeStart, SkBlendMode rangeEnd); // inclusive
    void add(BlendModeGroup);
    void add(SkBlenderID);

    // Shaders
    void add(SkShaderCombo);

    void reset();

private:
    friend class skgpu::graphite::Context; // for access to buildCombinations

    void buildCombinations(SkShaderCodeDictionary*,
                           const std::function<void(SkUniquePaintParamsID)>&) const;

    SkShaderCodeDictionary* fDictionary;
    std::unique_ptr<SkArenaAllocWithReset> fArena;
    SkPaintCombinations* fCombinations;
};

#endif // SkCombinationBuilder_DEFINED
