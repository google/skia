
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef SkEmptyShader_DEFINED
#define SkEmptyShader_DEFINED

#include "SkShader.h"

/**
 *  \class SkEmptyShader
 *  A Shader that always draws nothing. Its setContext always returns false,
 *  so it never expects that its shadeSpan() methods will get called.
 */
class SK_API SkEmptyShader : public SkShader {
public:
    SkEmptyShader() {}

    virtual uint32_t getFlags() SK_OVERRIDE;
    virtual uint8_t getSpan16Alpha() const SK_OVERRIDE;
    virtual bool setContext(const SkBitmap&, const SkPaint&,
                            const SkMatrix&) SK_OVERRIDE;
    virtual void shadeSpan(int x, int y, SkPMColor span[], int count) SK_OVERRIDE;
    virtual void shadeSpan16(int x, int y, uint16_t span[], int count) SK_OVERRIDE;
    virtual void shadeSpanAlpha(int x, int y, uint8_t alpha[], int count) SK_OVERRIDE;

protected:
    SkEmptyShader(SkFlattenableReadBuffer&);

    virtual Factory getFactory() SK_OVERRIDE;
    virtual void flatten(SkFlattenableWriteBuffer&) SK_OVERRIDE;

private:
    typedef SkShader INHERITED;
};

#endif
