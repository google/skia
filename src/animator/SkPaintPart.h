/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPaintPart_DEFINED
#define SkPaintPart_DEFINED

#include "SkDisplayable.h"
#include "SkMemberInfo.h"
#include "SkPaint.h"
#include "SkShader.h"
#include "SkTypeface.h"
#include "SkXfermode.h"

class SkDrawPaint;
class SkDrawMatrix;

class SkPaintPart : public SkDisplayable {
public:
    SkPaintPart();
    virtual bool add() = 0;
    virtual SkDisplayable* getParent() const;
    virtual bool setParent(SkDisplayable* parent);
#ifdef SK_DEBUG
    virtual bool isPaintPart() const { return true; }
#endif
protected:
    SkDrawPaint* fPaint;
};

class SkDrawMaskFilter : public SkPaintPart {
    DECLARE_EMPTY_MEMBER_INFO(MaskFilter);
    virtual SkMaskFilter* getMaskFilter();
protected:
    bool add() SK_OVERRIDE;
};

class SkDrawPathEffect : public SkPaintPart {
    DECLARE_EMPTY_MEMBER_INFO(PathEffect);
    virtual SkPathEffect* getPathEffect();
protected:
    bool add() SK_OVERRIDE;
};

class SkDrawShader : public SkPaintPart {
    DECLARE_DRAW_MEMBER_INFO(Shader);
    SkDrawShader();
    virtual SkShader* getShader();
protected:
    bool add() SK_OVERRIDE;
    SkMatrix* getMatrix(); // returns NULL if matrix is NULL
    SkDrawMatrix* matrix;
    int /*SkShader::TileMode*/ tileMode;
};

class SkDrawTypeface  : public SkPaintPart {
    DECLARE_DRAW_MEMBER_INFO(Typeface);
    SkDrawTypeface();
#ifdef SK_DUMP_ENABLED
    void dump(SkAnimateMaker *) SK_OVERRIDE;
#endif
    SkTypeface* getTypeface() {
        return SkTypeface::CreateFromName(fontName.c_str(), style); }
protected:
    bool add() SK_OVERRIDE;
    SkString fontName;
    SkTypeface::Style style;
};

#endif // SkPaintPart_DEFINED
