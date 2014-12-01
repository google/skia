#ifndef _SkTestImageFilters_h
#define _SkTestImageFilters_h

#include "SkImageFilter.h"
#include "SkPoint.h"

// Fun mode that scales down (only) and then scales back up to look pixelated
class SK_API SkDownSampleImageFilter : public SkImageFilter {
public:
    static SkDownSampleImageFilter* Create(SkScalar scale, SkImageFilter* input = NULL) {
        if (!SkScalarIsFinite(scale)) {
            return NULL;
        }
        // we don't support scale in this range
        if (scale > SK_Scalar1 || scale <= 0) {
            return NULL;
        }
        return SkNEW_ARGS(SkDownSampleImageFilter, (scale, input));
    }

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkDownSampleImageFilter)

protected:
    SkDownSampleImageFilter(SkScalar scale, SkImageFilter* input)
      : INHERITED(1, &input), fScale(scale) {}
#ifdef SK_SUPPORT_LEGACY_DEEPFLATTENING
    SkDownSampleImageFilter(SkReadBuffer& buffer);
#endif
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const Context&,
                               SkBitmap* result, SkIPoint* loc) const SK_OVERRIDE;

private:
    SkScalar fScale;

    typedef SkImageFilter INHERITED;
};

#endif
