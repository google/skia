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

class GrProcessorKeyBuilder;

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

    void genKey(GrProcessorKeyBuilder* b) const;

    bool operator!=(const GrStencilSettings& that) const { return !(*this == that); }
    bool operator==(const GrStencilSettings&) const;

    struct Face : public GrTStencilFaceSettings<GrStencilTest, GrStencilOp> {
        void reset(const GrUserStencilSettings::Face&, bool useStencilClip, int numStencilBits);
        void setDisabled();
    };

    const Face& frontAndBack() const {
        SkASSERT(!this->isDisabled());
        SkASSERT(!this->isTwoSided());
        return fFront;
    }
    const Face& front(GrSurfaceOrigin origin) const {
        SkASSERT(this->isTwoSided());
        return (kTopLeft_GrSurfaceOrigin == origin) ? fFront : fBack;
    }
    const Face& back(GrSurfaceOrigin origin) const {
        SkASSERT(this->isTwoSided());
        return (kTopLeft_GrSurfaceOrigin == origin) ? fBack : fFront;
    }

    /**
     * Given a thing to draw into the stencil clip, a fill type, and a set op
     * this function determines:
     *      1. Whether the thing can be draw directly to the stencil clip or
     *      needs to be drawn to the client portion of the stencil first.
     *      2. How many passes are needed.
     *      3. What those passes are.
     *
     * @param op                the set op to combine this element with the existing clip
     * @param canBeDirect       can the caller draw this element directly (without using stencil)?
     * @param invertedFill      is this path inverted
     * @param drawDirectToClip  out: true if caller should draw the element directly, false if it
     *                          should draw it into the user stencil bits first.
     *
     * @return a null-terminated array of settings for stencil passes.
     *
     *         If drawDirectToClip is false, the caller must first draw the element into the user
     *         stencil bits, and then cover the clip area with multiple passes using the returned
     *         stencil settings.
     *
     *         If drawDirectToClip is true, the returned array will only have one pass and the
     *         caller should use those stencil settings while drawing the element directly.
     */
    static GrUserStencilSettings const* const* GetClipPasses(SkRegion::Op op,
                                                             bool canBeDirect,
                                                             bool invertedFill,
                                                             bool* drawDirectToClip);

    /** Gets the user stencil settings to directly set the clip bit. */
    static const GrUserStencilSettings* SetClipBitSettings(bool setToInside);

private:
    // Internal flag for backends to optionally mark their tracked stencil state as invalid.
    enum { kInvalid_PrivateFlag = (kLast_StencilFlag << 1) };

    uint32_t   fFlags;
    Face       fFront;
    Face       fBack;
};

#endif
