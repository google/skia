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
    SK_ATTR_DEPRECATED("use Create(SkShader*, const CropRect*)")
    static SkImageFilter* Create(SkShader* s, const SkRect& rect);
    static SkImageFilter* Create(SkShader* s, const CropRect* rect = NULL);

    bool affectsTransparentBlack() const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkRectShaderImageFilter)

protected:
    virtual ~SkRectShaderImageFilter();

    void flatten(SkWriteBuffer&) const override;
    bool onFilterImage(Proxy*, const SkBitmap& src, const Context&, SkBitmap* result,
                       SkIPoint* loc) const override;

private:
    SkRectShaderImageFilter(SkShader* s, const CropRect* rect);

    SkShader*  fShader;

    typedef SkImageFilter INHERITED;
};

#endif
