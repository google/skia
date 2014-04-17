/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFilterShader_DEFINED
#define SkFilterShader_DEFINED

#include "SkShader.h"

class SkColorFilter;

class SkFilterShader : public SkShader {
public:
    SkFilterShader(SkShader* shader, SkColorFilter* filter);
    virtual ~SkFilterShader();

    virtual uint32_t getFlags() SK_OVERRIDE;
    virtual bool setContext(const SkBitmap&, const SkPaint&,
                            const SkMatrix&) SK_OVERRIDE;
    virtual void endContext() SK_OVERRIDE;
    virtual void shadeSpan(int x, int y, SkPMColor[], int count) SK_OVERRIDE;
    virtual void shadeSpan16(int x, int y, uint16_t[], int count) SK_OVERRIDE;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkFilterShader)

protected:
    SkFilterShader(SkReadBuffer& );
    virtual void flatten(SkWriteBuffer&) const SK_OVERRIDE;

private:
    SkShader*       fShader;
    SkColorFilter*  fFilter;

    typedef SkShader INHERITED;
};

#endif
