/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef gr_instanced_InstancedRenderingTypes_DEFINED
#define gr_instanced_InstancedRenderingTypes_DEFINED

#include "GrTypes.h"
#include "GrTypesPriv.h"
#include "SkRRect.h"

namespace gr_instanced {

/**
 * Per-vertex data. These values get fed into normal vertex attribs.
 */
struct ShapeVertex {
    float     fX, fY;  //!< Shape coordinates.
    int32_t   fAttrs;  //!< Shape-specific vertex attributes, if needed.
};

/**
 * Per-instance data. These values get fed into instanced vertex attribs.
 */
struct Instance {
    uint32_t   fInfo;               //!< Packed info about the instance. See InfoBits.
    float      fShapeMatrix2x3[6];  //!< Maps canonical shape coords -> device space coords.
    uint32_t   fColor;              //!< Color to be written out by the primitive processor.
    float      fLocalRect[4];       //!< Local coords rect that spans [-1, +1] in shape coords.
};

enum class Attrib : uint8_t {
    kShapeCoords,
    kVertexAttrs,
    kInstanceInfo,
    kShapeMatrixX,
    kShapeMatrixY,
    kColor,
    kLocalRect
};
constexpr int kNumAttribs = 1 + (int)Attrib::kLocalRect;

enum class ShapeType : uint8_t {
    kRect,
    kOval,
    kSimpleRRect,
    kNinePatch,
    kComplexRRect
};
constexpr int kNumShapeTypes = 1 + (int)ShapeType::kComplexRRect;

inline static ShapeType GetRRectShapeType(const SkRRect& rrect) {
    SkASSERT(rrect.getType() >= SkRRect::kRect_Type &&
             rrect.getType() <= SkRRect::kComplex_Type);
    return static_cast<ShapeType>(rrect.getType() - 1);

    GR_STATIC_ASSERT((int)ShapeType::kRect == SkRRect::kRect_Type - 1);
    GR_STATIC_ASSERT((int)ShapeType::kOval == SkRRect::kOval_Type - 1);
    GR_STATIC_ASSERT((int)ShapeType::kSimpleRRect == SkRRect::kSimple_Type - 1);
    GR_STATIC_ASSERT((int)ShapeType::kNinePatch == SkRRect::kNinePatch_Type - 1);
    GR_STATIC_ASSERT((int)ShapeType::kComplexRRect == SkRRect::kComplex_Type - 1);
    GR_STATIC_ASSERT(kNumShapeTypes == SkRRect::kComplex_Type);
}

enum ShapeFlag {
    kRect_ShapeFlag          = (1 << (int)ShapeType::kRect),
    kOval_ShapeFlag          = (1 << (int)ShapeType::kOval),
    kSimpleRRect_ShapeFlag   = (1 << (int)ShapeType::kSimpleRRect),
    kNinePatch_ShapeFlag     = (1 << (int)ShapeType::kNinePatch),
    kComplexRRect_ShapeFlag  = (1 << (int)ShapeType::kComplexRRect),

    kRRect_ShapesMask = kSimpleRRect_ShapeFlag | kNinePatch_ShapeFlag | kComplexRRect_ShapeFlag
};

constexpr uint8_t GetShapeFlag(ShapeType type) { return 1 << (int)type; }

/**
 * Defines what data is stored at which bits in the fInfo field of the instanced data.
 */
enum InfoBits {
    kShapeType_InfoBit       = 29,
    kInnerShapeType_InfoBit  = 27,
    kPerspective_InfoBit     = 26,
    kLocalMatrix_InfoBit     = 25,
    kParamsIdx_InfoBit       =  0
};

enum InfoMasks {
    kShapeType_InfoMask       = 0u - (1 << kShapeType_InfoBit),
    kInnerShapeType_InfoMask  = (1 << kShapeType_InfoBit) - (1 << kInnerShapeType_InfoBit),
    kPerspective_InfoFlag     = (1 << kPerspective_InfoBit),
    kLocalMatrix_InfoFlag     = (1 << kLocalMatrix_InfoBit),
    kParamsIdx_InfoMask       = (1 << kLocalMatrix_InfoBit) - 1
};

GR_STATIC_ASSERT((kNumShapeTypes - 1) <= (uint32_t)kShapeType_InfoMask >> kShapeType_InfoBit);
GR_STATIC_ASSERT((int)ShapeType::kSimpleRRect <=
                 kInnerShapeType_InfoMask >> kInnerShapeType_InfoBit);

/**
 * Additional parameters required by some instances (e.g. round rect radii, perspective column,
 * local matrix). These are accessed via texel buffer.
 */
struct ParamsTexel {
    float fX, fY, fZ, fW;
};

GR_STATIC_ASSERT(0 == offsetof(ParamsTexel, fX));
GR_STATIC_ASSERT(4 * 4 == sizeof(ParamsTexel));

/**
 * Tracks all information needed in order to draw a op of instances. This struct also serves
 * as an all-in-one shader key for the op.
 */
struct OpInfo {
    OpInfo() : fData(0) {}
    explicit OpInfo(uint32_t data) : fData(data) {}

    static bool CanCombine(const OpInfo& a, const OpInfo& b);

    bool isSimpleRects() const {
        return !((fShapeTypes & ~kRect_ShapeFlag) | fInnerShapeTypes);
    }

    GrAAType aaType() const { return static_cast<GrAAType>(fAAType); }
    void setAAType(GrAAType aaType) { fAAType = static_cast<uint8_t>(aaType); }

    union {
        struct {
            uint8_t         fAAType;  // GrAAType
            uint8_t         fShapeTypes;
            uint8_t         fInnerShapeTypes;
            bool            fHasPerspective               : 1;
            bool            fHasLocalMatrix               : 1;
            bool            fHasParams                    : 1;
            bool            fNonSquare                    : 1;
            bool            fUsesLocalCoords              : 1;
            bool            fCannotTweakAlphaForCoverage  : 1;
            bool            fCannotDiscard                : 1;
        };
        uint32_t fData;
    };
};

inline bool OpInfo::CanCombine(const OpInfo& a, const OpInfo& b) {
    if (a.fAAType != b.fAAType) {
        return false;
    }
    if (SkToBool(a.fInnerShapeTypes) != SkToBool(b.fInnerShapeTypes)) {
        // GrInstanceProcessor can't currently combine draws with and without inner shapes.
        return false;
    }
    if (a.fCannotDiscard != b.fCannotDiscard) {
        // For stencil draws, the use of discard can be a requirement.
        return false;
    }
    return true;
}

inline OpInfo operator|(const OpInfo& a, const OpInfo& b) {
    SkASSERT(OpInfo::CanCombine(a, b));
    return OpInfo(a.fData | b.fData);
}

// This is required since all the data must fit into 32 bits of a shader key.
GR_STATIC_ASSERT(sizeof(uint32_t) == sizeof(OpInfo));
GR_STATIC_ASSERT(kNumShapeTypes <= 8);

struct IndexRange {
    bool operator ==(const IndexRange& that) const {
        SkASSERT(fStart != that.fStart || fCount == that.fCount);
        return fStart == that.fStart;
    }
    bool operator !=(const IndexRange& that) const { return !(*this == that); }

    bool isEmpty() const { return fCount <= 0; }
    int end() { return fStart + fCount; }

    int16_t fStart;
    int16_t fCount;
};

}

#endif
