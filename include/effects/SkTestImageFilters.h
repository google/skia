
#ifndef _SkTestImageFilters_h
#define _SkTestImageFilters_h

#include "SkImageFilter.h"

class SkOffsetImageFilter : public SkImageFilter {
public:
    SkOffsetImageFilter(SkScalar dx, SkScalar dy) {
        fOffset.set(dx, dy);
    }

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkOffsetImageFilter, (buffer));
    }

protected:
    SkOffsetImageFilter(SkFlattenableReadBuffer& buffer);

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) SK_OVERRIDE;
    // overrides from SkFlattenable
    virtual void flatten(SkFlattenableWriteBuffer&) SK_OVERRIDE;
    virtual Factory getFactory() SK_OVERRIDE;

private:
    SkVector fOffset;

    typedef SkImageFilter INHERITED;
};

class SkComposeImageFilter : public SkImageFilter {
public:
    SkComposeImageFilter(SkImageFilter* outer, SkImageFilter* inner) {
        fOuter = outer;
        fInner = inner;
        SkSafeRef(outer);
        SkSafeRef(inner);
    }
    virtual ~SkComposeImageFilter();

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkComposeImageFilter, (buffer));
    }
    
protected:
    SkComposeImageFilter(SkFlattenableReadBuffer& buffer);
    
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) SK_OVERRIDE;
    // overrides from SkFlattenable
    virtual void flatten(SkFlattenableWriteBuffer&) SK_OVERRIDE;
    virtual Factory getFactory() SK_OVERRIDE;
    
private:
    SkImageFilter*  fOuter;
    SkImageFilter*  fInner;
    
    typedef SkImageFilter INHERITED;
};

#include "SkXfermode.h"

class SkMergeImageFilter : public SkImageFilter {
public:
    SkMergeImageFilter(SkImageFilter* first, SkImageFilter* second,
                       SkXfermode::Mode = SkXfermode::kSrcOver_Mode);
    SkMergeImageFilter(SkImageFilter* const filters[], int count,
                       const SkXfermode::Mode modes[] = NULL);
    virtual ~SkMergeImageFilter();
    
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkMergeImageFilter, (buffer));
    }
    
protected:
    SkMergeImageFilter(SkFlattenableReadBuffer& buffer);
    
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;
    virtual bool onFilterBounds(const SkIRect&, const SkMatrix&, SkIRect*) SK_OVERRIDE;
    // overrides from SkFlattenable
    virtual void flatten(SkFlattenableWriteBuffer&) SK_OVERRIDE;
    virtual Factory getFactory() SK_OVERRIDE;
    
private:
    SkImageFilter**     fFilters;
    uint8_t*            fModes; // SkXfermode::Mode
    int                 fCount;

    // private storage, to avoid dynamically allocating storage for our copy
    // of the filters and modes (unless fCount is so large we can't fit).
    intptr_t    fStorage[16];

    void initAlloc(int count, bool hasModes);
    void init(SkImageFilter* const [], int count, const SkXfermode::Mode []);
    
    typedef SkImageFilter INHERITED;
};

class SkColorFilter;

class SkColorFilterImageFilter : public SkImageFilter {
public:
    SkColorFilterImageFilter(SkColorFilter* cf) : fColorFilter(cf) {
        SkSafeRef(cf);
    }
    virtual ~SkColorFilterImageFilter();

    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkColorFilterImageFilter, (buffer));
    }
    
protected:
    SkColorFilterImageFilter(SkFlattenableReadBuffer& buffer);
    
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;
    // overrides from SkFlattenable
    virtual void flatten(SkFlattenableWriteBuffer&) SK_OVERRIDE;
    virtual Factory getFactory() SK_OVERRIDE;
    
private:
    SkColorFilter*  fColorFilter;
    
    typedef SkImageFilter INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

// Fun mode that scales down (only) and then scales back up to look pixelated
class SkDownSampleImageFilter : public SkImageFilter {
public:
    SkDownSampleImageFilter(SkScalar scale) : fScale(scale) {}
    
    static SkFlattenable* CreateProc(SkFlattenableReadBuffer& buffer) {
        return SkNEW_ARGS(SkDownSampleImageFilter, (buffer));
    }
    
protected:
    SkDownSampleImageFilter(SkFlattenableReadBuffer& buffer);
    
    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;
    // overrides from SkFlattenable
    virtual void flatten(SkFlattenableWriteBuffer&) SK_OVERRIDE;
    virtual Factory getFactory()  SK_OVERRIDE;
    
private:
    SkScalar fScale;
    
    typedef SkImageFilter INHERITED;
};

#endif
