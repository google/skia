#ifndef _SkTestImageFilters_h
#define _SkTestImageFilters_h

#include "SkImageFilter.h"
#include "SkPoint.h"

// Fun mode that scales down (only) and then scales back up to look pixelated
class SK_API SkDownSampleImageFilter : public SkImageFilter {
public:
    static SkDownSampleImageFilter* Create(SkScalar scale, SkImageFilter* input = NULL) {
        return SkNEW_ARGS(SkDownSampleImageFilter, (scale, input));
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDownSampleImageFilter)

protected:
    SkDownSampleImageFilter(SkScalar scale, SkImageFilter* input)
      : INHERITED(1, &input), fScale(scale) {}
    SkDownSampleImageFilter(SkReadBuffer& buffer);
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;

private:
    SkScalar fScale;

    typedef SkImageFilter INHERITED;
};

#endif
