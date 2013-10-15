/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRectShaderImageFilter_DEFINED
#define SkRectShaderImageFilter_DEFINED

#include "SkImageFilter.h"
#include "SkRect.h"

class SkShader;

class SK_API SkRectShaderImageFilter : public SkImageFilter {
public:
    /** Create a new image filter which fills the given rectangle with pixels
     *  produced by the given SkShader. If no rectangle is specified, an output
     *  is produced with the same bounds as the input primitive (even though 
     *  the input primitive's pixels are not used for processing).
     *  @param s     Shader to call for processing. Cannot be NULL. Will be
     *               ref'ed by the new image filter.
     *  @param rect  Rectangle of output pixels in which to apply the shader.
     *               If NULL or a given crop edge is not specified, the source
     *               primitive's bounds are used instead.
     */
    /* DEPRECATED */ static SkRectShaderImageFilter* Create(SkShader* s, const SkRect& rect);
    static SkRectShaderImageFilter* Create(SkShader* s, const CropRect* rect = NULL);
    virtual ~SkRectShaderImageFilter();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkRectShaderImageFilter)

protected:
    SkRectShaderImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;

private:
    SkRectShaderImageFilter(SkShader* s, const CropRect* rect);
    SkShader*  fShader;

    typedef SkImageFilter INHERITED;
};

#endif
