/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageFilterPriv_DEFINED
#define SkImageFilterPriv_DEFINED

#include "include/core/SkImageFilter.h"

/**
 *  Helper to unflatten the common data, and return nullptr if we fail.
 */
#define SK_IMAGEFILTER_UNFLATTEN_COMMON(localVar, expectedCount)    \
    Common localVar;                                                \
    do {                                                            \
        if (!localVar.unflatten(buffer, expectedCount)) {           \
            return nullptr;                                         \
        }                                                           \
    } while (0)

/**
 * Return an image filter representing this filter applied with the given ctm. This will modify the
 * DAG as needed if this filter does not support complex CTMs and 'ctm' is not simple. The ctm
 * matrix will be decomposed such that ctm = A*B; B will be incorporated directly into the DAG and A
 * must be the ctm set on the context passed to filterImage(). 'remainder' will be set to A.
 *
 * If this filter supports complex ctms, or 'ctm' is not complex, then A = ctm and B = I. When the
 * filter does not support complex ctms, and the ctm is complex, then A represents the extracted
 * simple portion of the ctm, and the complex portion is baked into a new DAG using a matrix filter.
 *
 * This will never return null.
 */
sk_sp<SkImageFilter> SkApplyCTMToFilter(const SkImageFilter* filter, const SkMatrix& ctm,
                                        SkMatrix* remainder);

/**
 * Similar to SkApplyCTMToFilter except this assumes the input content is an existing backdrop image
 * to be filtered. As such,  the input to this filter will also be transformed by B^-1 if the filter
 * can't support complex CTMs, since backdrop content is already in device space and must be
 * transformed back into the CTM's local space.
 */
sk_sp<SkImageFilter> SkApplyCTMToBackdropFilter(const SkImageFilter* filter, const SkMatrix& ctm,
                                                SkMatrix* remainder);

bool SkIsSameFilter(const SkImageFilter* a, const SkImageFilter* b);

// Extra information about the output of a filter DAG. For now, this is just the color space (of
// the original requesting device). This is used when constructing intermediate rendering
// surfaces, so that we ensure we land in a surface that's similar/compatible to the final
// consumer of the DAG's output.
class SkFilterOutputProperties {
public:
    explicit SkFilterOutputProperties(SkColorType colorType, SkColorSpace* colorSpace)
        : fColorType(colorType), fColorSpace(colorSpace) {}

    SkColorType colorType() const { return fColorType; }
    SkColorSpace* colorSpace() const { return fColorSpace; }

private:
    SkColorType fColorType;
    // This will be a pointer to the device's color space, and our lifetime is bounded by the
    // device, so we can store a bare pointer.
    SkColorSpace* fColorSpace;
};

class SkFilterContext {
public:
    SkFilterContext(const SkMatrix& ctm, const SkIRect& clipBounds, SkImageFilterCache* cache,
                    const SkFilterOutputProperties& outputProperties)
        : fCTM(ctm)
        , fClipBounds(clipBounds)
        , fCache(cache)
        , fOutputProperties(outputProperties)
    {}

    const SkMatrix& ctm() const { return fCTM; }
    const SkIRect& clipBounds() const { return fClipBounds; }
    SkImageFilterCache* cache() const { return fCache; }
    const SkFilterOutputProperties& outputProperties() const { return fOutputProperties; }

    /**
     *  Since a context can be build directly, its constructor has no chance to
     *  "return null" if it's given invalid or unsupported inputs. Call this to
     *  know of the the context can be used.
     *
     *  The SkImageFilterCache Key, for example, requires a finite ctm (no infinities
     *  or NaN), so that test is part of isValid.
     */
    bool isValid() const { return fCTM.isFinite(); }

private:
    SkMatrix               fCTM;
    SkIRect                fClipBounds;
    SkImageFilterCache*    fCache;
    SkFilterOutputProperties       fOutputProperties;
};

/** Class that adds methods to SkImageFilter that are only intended for use internal to Skia.
    This class is purely a privileged window into SkImageFilter. It should never have additional
    data members or virtual methods. */
class SkImageFilterPriv {
public:
    /**
     *  Request a new filtered image to be created from the filter, based on the src image.
     *
     *  The context contains the environment in which the filter is occurring. It includes the clip
     *  bounds, CTM and cache.  Offset is the amount to translate the resulting image relative to
     *  the src when it is drawn. This is an out-param.
     *
     *  If the result image cannot be created, or the result would be transparent black, return
     *  null, in which case the offset parameter should be ignored by the caller.
     *
     *  TODO: Right now the imagefilters sometimes return empty result bitmaps/
     *        specialimages. That doesn't seem quite right.
     */
    sk_sp<SkSpecialImage> filterImage(SkSpecialImage* src, const SkFilterContext& context,
                                      SkIPoint* offset) const;

    // Exposes just the behavior of the protected SkImageFilter::onFilterNodeBounds() without
    // going through the filter's inputs.
    SkIRect filterNodeBounds(const SkIRect& srcRect, const SkMatrix& ctm,
                             SkImageFilter::MapDirection dir, const SkIRect* inputRect) const {
        return fFilter->onFilterNodeBounds(srcRect, ctm, dir, inputRect);
    }

#if SK_SUPPORT_GPU
    static sk_sp<SkSpecialImage> DrawWithFP(GrRecordingContext* context,
                                            std::unique_ptr<GrFragmentProcessor> fp,
                                            const SkIRect& bounds,
                                            const SkFilterOutputProperties& outputProperties,
                                            GrProtected isProtected = GrProtected::kNo);

    /**
     *  Returns a version of the passed-in image (possibly the original), that is in a colorspace
     *  with the same gamut as the one from the OutputProperties. This allows filters that do many
     *  texture samples to guarantee that any color space conversion has happened before running.
     */
    static sk_sp<SkSpecialImage> ImageToColorSpace(SkSpecialImage* src,
                                                   const SkFilterOutputProperties&);
#endif

private:
    explicit SkImageFilterPriv(SkImageFilter* filter)
            : fFilter(filter) {}
    SkImageFilterPriv(const SkImageFilterPriv&) {} // unimpl
    SkImageFilterPriv& operator=(const SkImageFilterPriv&); // unimpl

    // No taking addresses of this type.
    const SkImageFilterPriv* operator&() const;
    SkImageFilterPriv* operator&();

    SkImageFilter* fFilter;

    friend class SkImageFilter; // to construct/copy this type
};

inline SkImageFilterPriv SkImageFilter::priv() {
    return SkImageFilterPriv(this);
}

inline const SkImageFilterPriv SkImageFilter::priv() const {
    return SkImageFilterPriv(const_cast<SkImageFilter*>(this));
}


#endif
