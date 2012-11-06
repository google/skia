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

#include "SkXfermode.h"

class SK_API SkMergeImageFilter : public SkImageFilter {
public:
    SkMergeImageFilter(SkImageFilter* first, SkImageFilter* second,
                       SkXfermode::Mode = SkXfermode::kSrcOver_Mode);
    SkMergeImageFilter(SkImageFilter* filters[], int count,
                       const SkXfermode::Mode modes[] = NULL);
    virtual ~SkMergeImageFilter();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkMergeImageFilter)

protected:
    SkMergeImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) SK_OVERRIDE;

private:
    uint8_t*            fModes; // SkXfermode::Mode
    int                 fCount;

    // private storage, to avoid dynamically allocating storage for our copy
    // of the filters and modes (unless fCount is so large we can't fit).
    intptr_t    fStorage[16];

    void initAllocModes();
    void initModes(const SkXfermode::Mode []);

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
