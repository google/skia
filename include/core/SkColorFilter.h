/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorFilter_DEFINED
#define SkColorFilter_DEFINED

#include "SkBlendMode.h"
#include "SkColor.h"
#include "SkFlattenable.h"
#include "SkRefCnt.h"

class GrColorSpaceInfo;
class GrFragmentProcessor;
class GrRecordingContext;
class SkBitmap;
class SkColorSpace;
class SkMixer;
struct SkStageRec;
class SkString;

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
    /** DEPRECATED. skbug.com/8941
     *  If the filter can be represented by a source color plus Mode, this
     *  returns true, and sets (if not NULL) the color and mode appropriately.
     *  If not, this returns false and ignores the parameters.
     */
    virtual bool asColorMode(SkColor* color, SkBlendMode* bmode) const;

    /** DEPRECATED. skbug.com/8941
     *  If the filter can be represented by a 5x4 matrix, this
     *  returns true, and sets the matrix appropriately.
     *  If not, this returns false and ignores the parameter.
     */
    virtual bool asColorMatrix(SkScalar matrix[20]) const;

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const;

    enum Flags {
        /** If set the filter methods will not change the alpha channel of the colors.
        */
        kAlphaUnchanged_Flag = 1 << 0,
    };

    /** Returns the flags for this filter. Override in subclasses to return custom flags.
    */
    virtual uint32_t getFlags() const { return 0; }

    SkColor filterColor(SkColor) const;
    SkColor4f filterColor4f(const SkColor4f&, SkColorSpace*) const;

    /** Create a colorfilter that uses the specified color and mode.
        If the Mode is DST, this function will return NULL (since that
        mode will have no effect on the result).
        @param c    The source color used with the specified mode
        @param mode The blend that is applied to each color in
                        the colorfilter's filterSpan[16,32] methods
        @return colorfilter object that applies the src color and mode,
                    or NULL if the mode will have no effect.
    */
    static sk_sp<SkColorFilter> MakeModeFilter(SkColor c, SkBlendMode mode);

    /** Construct a colorfilter whose effect is to first apply the inner filter and then apply
     *  this filter, applied to the output of the inner filter.
     *
     *  result = this(inner(...))
     *
     *  Due to internal limits, it is possible that this will return NULL, so the caller must
     *  always check.
     */
    sk_sp<SkColorFilter> makeComposed(sk_sp<SkColorFilter> inner) const;

    // DEPRECATED, call makeComposed instead
    static sk_sp<SkColorFilter> MakeComposeFilter(sk_sp<SkColorFilter> outer,
                                                  sk_sp<SkColorFilter> inner) {
        return outer ? outer->makeComposed(inner) : inner;
    }

    /** Construct a color filter that transforms a color by a 4x5 matrix. The matrix is in row-
     *  major order and the translation column is specified in unnormalized, 0...255, space.
     */
    static sk_sp<SkColorFilter> MakeMatrixFilterRowMajor255(const SkScalar array[20]);

    /** Construct a colorfilter that applies the srgb gamma curve to the RGB channels */
    static sk_sp<SkColorFilter> MakeLinearToSRGBGamma();

    /** Construct a colorfilter that applies the inverse of the srgb gamma curve to the
     *  RGB channels
     */
    static sk_sp<SkColorFilter> MakeSRGBToLinearGamma();

    /**
     *  Returns a new filter that returns the weighted average between the outputs of
     *  two other filters. If either is null, then it is treated as an identity filter.
     *
     *  result = cf0(color) * (1 - weight) + cf1(color) * weight
     *
     *  If both filters are null, or if weight is NaN, then null is returned.
     */
    static sk_sp<SkColorFilter> MakeLerp(sk_sp<SkColorFilter> cf0, sk_sp<SkColorFilter> cf1,
                                         float weight);

    /**
     *  Returns a new filter that mixes the output of two other filters. If either filter is null,
     *  then it is treated like an identity filter.
     *
     *  result = mx(cf0(color), cf1(color))
     */
    static sk_sp<SkColorFilter> MakeMixer(sk_sp<SkColorFilter> cf0, sk_sp<SkColorFilter> cf1,
                                          sk_sp<SkMixer> mx);

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
            GrRecordingContext*, const GrColorSpaceInfo& dstColorSpaceInfo) const;
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

    /**
     *  If this subclass can optimally createa composition with the inner filter, return it as
     *  a new filter (which the caller must unref() when it is done). If no such optimization
     *  is known, return NULL.
     *
     *  e.g. result(color) == this_filter(inner(color))
     */
    virtual sk_sp<SkColorFilter> onMakeComposed(sk_sp<SkColorFilter>) const { return nullptr; }

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

    friend class SkComposeColorFilter;

    typedef SkFlattenable INHERITED;
};

#endif
