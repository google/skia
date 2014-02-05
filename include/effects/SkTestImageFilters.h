#ifndef _SkTestImageFilters_h
#define _SkTestImageFilters_h

#include "SkImageFilter.h"
#include "SkPoint.h"

// Fun mode that scales down (only) and then scales back up to look pixelated
class SK_API SkDownSampleImageFilter : public SkImageFilter {
public:
    SkDownSampleImageFilter(SkScalar scale) : INHERITED(0), fScale(scale) {}

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDownSampleImageFilter)

protected:
    SkDownSampleImageFilter(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;

private:
    SkScalar fScale;

    typedef SkImageFilter INHERITED;
};

#endif
