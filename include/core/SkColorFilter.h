/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilter_DEFINED
#define SkColorFilter_DEFINED

#include "include/core/SkBlendMode.h"
#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkRefCnt.h"

class GrColorInfo;
class GrFragmentProcessor;
class GrRecordingContext;
class SkArenaAlloc;
class SkBitmap;
class SkColorMatrix;
class SkColorSpace;
struct SkStageRec;

namespace skvm {
    class Builder;
    struct F32;
    struct Uniforms;
}

/**
 *  ColorFilters are optional objects in the drawing pipeline. When present in
 *  a paint, they are called with the "src" colors, and return new colors, which
 *  are then passed onto the next stage (either ImageFilter or Xfermode).
 *
 *  All subclasses are required to be reentrant-safe : it must be legal to share
 *  the same instance between several threads.
 */
class SK_API SkColorFilter : public SkFlattenable {
public:
    // DEPRECATED. skbug.com/8941

    bool asColorMode(SkColor* color, SkBlendMode* mode) const {
        return this->onAsAColorMode(color, mode);
    }

    /** If the filter can be represented by a source color plus Mode, this
     *  returns true, and sets (if not NULL) the color and mode appropriately.
     *  If not, this returns false and ignores the parameters.
     */
    bool asAColorMode(SkColor* color, SkBlendMode* mode) const {
        return this->onAsAColorMode(color, mode);
    }

    /** If the filter can be represented by a 5x4 matrix, this
     *  returns true, and sets the matrix appropriately.
     *  If not, this returns false and ignores the parameter.
     */
    bool asAColorMatrix(float matrix[20]) const {
        return this->onAsAColorMatrix(matrix);
    }

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const;

    bool program(skvm::Builder*,
                 SkColorSpace* dstCS,
                 skvm::Uniforms* uniforms, SkArenaAlloc* alloc,
                 skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32* a) const;

    enum Flags {
        /** If set the filter methods will not change the alpha channel of the colors.
        */
        kAlphaUnchanged_Flag = 1 << 0,
    };

    /** Returns the flags for this filter. Override in subclasses to return custom flags.
    */
    virtual uint32_t getFlags() const { return 0; }

    SkColor filterColor(SkColor) const;

    /**
     * Converts the src color (in src colorspace), into the dst colorspace,
     * then applies this filter to it, returning the filtered color in the dst colorspace.
     */
    SkColor4f filterColor4f(const SkColor4f& srcColor, SkColorSpace* srcCS,
                            SkColorSpace* dstCS) const;

    /** Construct a colorfilter whose effect is to first apply the inner filter and then apply
     *  this filter, applied to the output of the inner filter.
     *
     *  result = this(inner(...))
     *
     *  Due to internal limits, it is possible that this will return NULL, so the caller must
     *  always check.
     */
    sk_sp<SkColorFilter> makeComposed(sk_sp<SkColorFilter> inner) const;

#if SK_SUPPORT_GPU
    /**
     *  A subclass may implement this factory function to work with the GPU backend. It returns
     *  a GrFragmentProcessor that implemets the color filter in GPU shader code.
     *
     *  The fragment processor receives a premultiplied input color and produces a premultiplied
     *  output color.
     *
     *  A null return indicates that the color filter isn't implemented for the GPU backend.
     */
    virtual std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(
            GrRecordingContext*, const GrColorInfo& dstColorInfo) const;
#endif

    bool affectsTransparentBlack() const {
        return this->filterColor(SK_ColorTRANSPARENT) != SK_ColorTRANSPARENT;
    }

    static void RegisterFlattenables();

    static SkFlattenable::Type GetFlattenableType() {
        return kSkColorFilter_Type;
    }

    SkFlattenable::Type getFlattenableType() const override {
        return kSkColorFilter_Type;
    }

    static sk_sp<SkColorFilter> Deserialize(const void* data, size_t size,
                                          const SkDeserialProcs* procs = nullptr) {
        return sk_sp<SkColorFilter>(static_cast<SkColorFilter*>(
                                  SkFlattenable::Deserialize(
                                  kSkColorFilter_Type, data, size, procs).release()));
    }

protected:
    SkColorFilter() {}

    virtual bool onAsAColorMatrix(float[20]) const;
    virtual bool onAsAColorMode(SkColor* color, SkBlendMode* bmode) const;

private:
    /*
     *  Returns 1 if this is a single filter (not a composition of other filters), otherwise it
     *  reutrns the number of leaf-node filters in a composition. This should be the same value
     *  as the number of GrFragmentProcessors returned by asFragmentProcessors's array parameter.
     *
     *  e.g. compose(filter, compose(compose(filter, filter), filter)) --> 4
     */
    virtual int privateComposedFilterCount() const { return 1; }

    virtual bool onAppendStages(const SkStageRec& rec, bool shaderIsOpaque) const = 0;

    virtual bool onProgram(skvm::Builder*,
                           SkColorSpace* dstCS,
                           skvm::Uniforms* uniforms, SkArenaAlloc* alloc,
                           skvm::F32* r, skvm::F32* g, skvm::F32* b, skvm::F32* a) const;

    friend class SkComposeColorFilter;

    typedef SkFlattenable INHERITED;
};

class SK_API SkColorFilters {
public:
    static sk_sp<SkColorFilter> Compose(sk_sp<SkColorFilter> outer, sk_sp<SkColorFilter> inner) {
        return outer ? outer->makeComposed(inner) : inner;
    }
    static sk_sp<SkColorFilter> Blend(SkColor c, SkBlendMode mode);
    static sk_sp<SkColorFilter> Matrix(const SkColorMatrix&);
    static sk_sp<SkColorFilter> Matrix(const float rowMajor[20]);

    // A version of Matrix which operates in HSLA space instead of RGBA.
    // I.e. HSLA-to-RGBA(Matrix(RGBA-to-HSLA(input))).
    static sk_sp<SkColorFilter> HSLAMatrix(const float rowMajor[20]);

    static sk_sp<SkColorFilter> LinearToSRGBGamma();
    static sk_sp<SkColorFilter> SRGBToLinearGamma();
    static sk_sp<SkColorFilter> Lerp(float t, sk_sp<SkColorFilter> dst, sk_sp<SkColorFilter> src);

private:
    SkColorFilters() = delete;
};

#endif
