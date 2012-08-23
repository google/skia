
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDrawBitmap_DEFINED
#define SkDrawBitmap_DEFINED

#include "SkBoundable.h"
#include "SkBase64.h"
#include "SkBitmap.h"
// #include "SkImageDecoder.h"
#include "SkMemberInfo.h"

class SkBaseBitmap : public SkBoundable {
    DECLARE_MEMBER_INFO(BaseBitmap);
    SkBaseBitmap();
    virtual ~SkBaseBitmap();
    virtual bool draw(SkAnimateMaker& );
protected:
    SkBitmap fBitmap;
    SkScalar x;
    SkScalar y;
private:
    friend class SkDrawTo;
    friend class SkDrawBitmapShader;
    typedef SkBoundable INHERITED;
};

class SkDrawBitmap : public SkBaseBitmap {
    DECLARE_DRAW_MEMBER_INFO(Bitmap);
    SkDrawBitmap();
    virtual ~SkDrawBitmap();
#ifdef SK_DUMP_ENABLED
    virtual void dump(SkAnimateMaker* );
#endif
    virtual void onEndElement(SkAnimateMaker& );
    virtual bool setProperty(int index, SkScriptValue& value);
protected:
    int /*SkBitmap::Config*/ format;
    int32_t height;
    int32_t rowBytes;
    int32_t width;
    SkColor fColor;
    SkBool fColorSet;
    typedef SkBaseBitmap INHERITED;
};

class SkImageBaseBitmap : public SkBaseBitmap {
    DECLARE_MEMBER_INFO(ImageBaseBitmap);
    SkImageBaseBitmap();
    virtual ~SkImageBaseBitmap();
    virtual SkDisplayable* deepCopy(SkAnimateMaker* );
    virtual void dirty();
    virtual bool draw(SkAnimateMaker& );
    virtual bool getProperty(int index, SkScriptValue* value) const;
    virtual void onEndElement(SkAnimateMaker& maker);
private:
    void resolve() const { (const_cast<SkImageBaseBitmap*>(this))->resolve(); }
    void resolve();
protected:
    SkBase64 base64;
    SkString src;
    SkString fLast; // cache of src so that stream isn't unnecessarily decoded
    SkBool fDirty;
    const char* fUriBase;
    typedef SkBaseBitmap INHERITED;
};

#endif // SkDrawBitmap_DEFINED
