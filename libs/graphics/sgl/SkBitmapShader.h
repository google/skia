/* libs/graphics/sgl/SkBitmapShader.h
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#ifndef SkBitmapShader_DEFINED
#define SkBitmapShader_DEFINED

#include "SkShader.h"
#include "SkBitmap.h"
#include "SkPaint.h"

class SkBitmapShader : public SkShader {
public:
    SkBitmapShader( const SkBitmap& src, bool transferOwnershipOfPixels,
                    SkPaint::FilterType, TileMode tx, TileMode ty);

    virtual bool        setContext(const SkBitmap&, const SkPaint& paint, const SkMatrix&);
    virtual uint32_t    getFlags() { return fFlags; }

protected:
    const SkBitmap&     getSrcBitmap() const
    {
#ifdef SK_SUPPORT_MIPMAP
        return fMipSrcBitmap;
#else
        return fOrigSrcBitmap;
#endif
    }
    SkPaint::FilterType getFilterType() const { return (SkPaint::FilterType)fFilterType; }
    TileMode            getTileModeX() const { return (TileMode)fTileModeX; }
    TileMode            getTileModeY() const { return (TileMode)fTileModeY; }
    SkFixed             getMipLevel() const 
    {
#ifdef SK_SUPPORT_MIPMAP
        return fMipLevel;
#else
        return 0;
#endif
    }

private:
#ifdef SK_SUPPORT_MIPMAP
    SkFixed     fMipLevel;
    SkBitmap    fMipSrcBitmap; // the chosen level (in setContext)
#endif
    SkBitmap    fOrigSrcBitmap;
    U8          fFilterType;
    U8          fTileModeX;
    U8          fTileModeY;
    U8          fFlags;

    typedef SkShader INHERITED;
};

#endif
