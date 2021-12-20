/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef GrStencilSettings_DEFINED
#define GrStencilSettings_DEFINED

#include "include/core/SkRegion.h"
#include "src/gpu/GrUserStencilSettings.h"

namespace skgpu {
class KeyBuilder;
}

enum class GrStencilTest : uint16_t {
    kAlways,
    kNever,
    kGreater,
    kGEqual,
    kLess,
    kLEqual,
    kEqual,
    kNotEqual
};
static constexpr int kGrStencilTestCount = 1 + (int)GrStencilTest::kNotEqual;

enum class GrStencilOp : uint8_t {
    kKeep,
    kZero,
    kReplace, // Replace stencil value with fRef (only the bits enabled in fWriteMask).
    kInvert,
    kIncWrap,
    kDecWrap,
    // NOTE: clamping occurs before the write mask. So if the MSB is zero and masked out, stencil
    // values will still wrap when using clamping ops.
    kIncClamp,
    kDecClamp
};
static constexpr int kGrStencilOpCount = 1 + (int)GrStencilOp::kDecClamp;

/**
 * This class defines concrete stencil settings that map directly to the underlying hardware. It
 * is deduced from user stencil settings, stencil clip status, and the number of bits in the
 * target stencil buffer.
 */
class GrStencilSettings {
public:
    GrStencilSettings() { this->setDisabled(); }
    GrStencilSettings(const GrUserStencilSettings& user, bool hasStencilClip, int numStencilBits) {
        this->reset(user, hasStencilClip, numStencilBits);
    }
    GrStencilSettings(const GrStencilSettings& that) { this->reset(that); }
    GrStencilSettings& operator=(const GrStencilSettings& that) { this->reset(that); return *this; }

    void invalidate() { fFlags |= kInvalid_PrivateFlag; }
    void setDisabled() { fFlags = kAll_StencilFlags; }
    void reset(const GrUserStencilSettings&, bool hasStencilClip, int numStencilBits);
    void reset(const GrStencilSettings&);

    bool isValid() const { return !(fFlags & kInvalid_PrivateFlag); }
    bool isDisabled() const { SkASSERT(this->isValid()); return fFlags & kDisabled_StencilFlag; }
    bool doesWrite() const { SkASSERT(this->isValid());
                             return !(fFlags & kNoModifyStencil_StencilFlag); }
    bool isTwoSided() const { SkASSERT(this->isValid());
                              return !(fFlags & kSingleSided_StencilFlag); }
    bool usesWrapOp() const { SkASSERT(this->isValid());
                              return !(fFlags & kNoWrapOps_StencilFlag); }

    void genKey(skgpu::KeyBuilder* b, bool includeRefsAndMasks) const;

    bool operator!=(const GrStencilSettings& that) const { return !(*this == that); }
    bool operator==(const GrStencilSettings&) const;

    struct Face : public GrTStencilFaceSettings<GrStencilTest, GrStencilOp> {
        void reset(const GrUserStencilSettings::Face&, bool useStencilClip, int numStencilBits);
        void setDisabled();
    };

    const Face& singleSidedFace() const {
        SkASSERT(!this->isDisabled());
        SkASSERT(!this->isTwoSided());
        return fCWFace;
    }
    // Returns the stencil settings for triangles that wind clockwise in "post-origin" space.
    // (i.e., the space that results after a potential y-axis flip on device space for bottom-left
    // origins.)
    const Face& postOriginCWFace(GrSurfaceOrigin origin) const {
        SkASSERT(this->isTwoSided());
        return (kTopLeft_GrSurfaceOrigin == origin) ? fCWFace : fCCWFace;
    }
    // Returns the stencil settings for triangles that wind counter-clockwise in "post-origin"
    // space. (i.e., the space that results after a potential y-axis flip on device space for
    // bottom-left origins.)
    const Face& postOriginCCWFace(GrSurfaceOrigin origin) const {
        SkASSERT(this->isTwoSided());
        return (kTopLeft_GrSurfaceOrigin == origin) ? fCCWFace : fCWFace;
    }

    /** Gets the user stencil settings to directly set the clip bit. */
    static const GrUserStencilSettings* SetClipBitSettings(bool setToInside);

private:
    // Internal flag for backends to optionally mark their tracked stencil state as invalid.
    // NOTE: This value is outside the declared range of GrStencilFlags, but since that type is
    // explicitly backed by 'int', it can still represent this constant. clang 11 complains about
    // mixing enum types in bit operations, so this works around that.
    inline static constexpr GrStencilFlags kInvalid_PrivateFlag =
            static_cast<GrStencilFlags>(kLast_StencilFlag << 1);

    uint32_t   fFlags;
    Face       fCWFace;
    Face       fCCWFace;
};

#endif
