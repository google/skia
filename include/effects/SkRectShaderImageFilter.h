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
    /**
     *  The SkShader object will have its refcnt increased as it becomes a member of the
     *  SkRectShaderImageFilter object returned by this function. It cannot be NULL.
     *  The region parameter is used to specify on which region the shader is applied.
     */
    static SkRectShaderImageFilter* Create(SkShader* s, const SkRect& rect);
    virtual ~SkRectShaderImageFilter();

    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkRectShaderImageFilter)

protected:
    SkRectShaderImageFilter(SkFlattenableReadBuffer& buffer);
    virtual void flatten(SkFlattenableWriteBuffer&) const SK_OVERRIDE;

    virtual bool onFilterImage(Proxy*, const SkBitmap& src, const SkMatrix&,
                               SkBitmap* result, SkIPoint* loc) SK_OVERRIDE;

private:
    SkRectShaderImageFilter(SkShader* s, const SkRect& rect);
    SkShader*  fShader;
    SkRect     fRect;

    typedef SkImageFilter INHERITED;
};

#endif
