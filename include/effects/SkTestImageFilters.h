#ifndef _SkTestImageFilters_h
#define _SkTestImageFilters_h

#include "SkImageFilter.h"
#include "SkPoint.h"

class SK_API SkComposeImageFilter : public SkImageFilter {
public:
    SkComposeImageFilter(SkImageFilter* outer, SkImageFilter* inner) : INHERITED(outer, inner) {}
    virtual ~SkComposeImageFilter();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkComposeImageFilter)

protected:
    SkComposeImageFilter(SkFlattenableReadBuffer& buffer);

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) SK_OVERRIDE;

private:
    typedef SkImageFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

// Fun mode that scales down (only) and then scales back up to look pixelated
class SK_API SkDownSampleImageFilter : public SkImageFilter {
public:
    SkDownSampleImageFilter(SkScalar scale) : INHERITED(0), fScale(scale) {}

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDownSampleImageFilter)

protected:
    SkDownSampleImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;

private:
    SkScalar fScale;

    typedef SkImageFilter INHERITED;
};

#endif
