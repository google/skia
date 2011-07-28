
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


/*  this guy is pulled in multiple times, with the following symbols defined each time:

    #define BITMAP_CLASSNAME_PREFIX(name)           ARGB32##name
    #defube BITMAP_PIXEL_TO_PMCOLOR(bitmap, x, y)   *bitmap.getAddr32(x, y)
*/

class BITMAP_CLASSNAME_PREFIX(_Point_Sampler) : public SkBitmapSampler {
public:
    BITMAP_CLASSNAME_PREFIX(_Point_Sampler)(const SkBitmap& bm, SkShader::TileMode tmx, SkShader::TileMode tmy)
        : SkBitmapSampler(bm, false, tmx, tmy)
    {
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        x = fTileProcX(SkFixedFloor(x), fMaxX);
        y = fTileProcY(SkFixedFloor(y), fMaxY);
        return BITMAP_PIXEL_TO_PMCOLOR(fBitmap, x, y);
    }
};


class BITMAP_CLASSNAME_PREFIX(_Point_Clamp_Sampler) : public SkBitmapSampler {
public:
    BITMAP_CLASSNAME_PREFIX(_Point_Clamp_Sampler)(const SkBitmap& bm)
        : SkBitmapSampler(bm, false, SkShader::kClamp_TileMode, SkShader::kClamp_TileMode)
    {
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        x = do_clamp(SkFixedFloor(x), fMaxX);
        y = do_clamp(SkFixedFloor(y), fMaxY);
        return BITMAP_PIXEL_TO_PMCOLOR(fBitmap, x, y);
    }
};

class BITMAP_CLASSNAME_PREFIX(_Point_Repeat_Pow2_Sampler) : public SkBitmapSampler {
public:
    BITMAP_CLASSNAME_PREFIX(_Point_Repeat_Pow2_Sampler)(const SkBitmap& bm)
        : SkBitmapSampler(bm, false, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode)
    {
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        x = do_repeat_pow2(SkFixedFloor(x), fMaxX);
        y = do_repeat_pow2(SkFixedFloor(y), fMaxY);
        return BITMAP_PIXEL_TO_PMCOLOR(fBitmap, x, y);
    }
};

class BITMAP_CLASSNAME_PREFIX(_Point_Repeat_Mod_Sampler) : public SkBitmapSampler {
public:
    BITMAP_CLASSNAME_PREFIX(_Point_Repeat_Mod_Sampler)(const SkBitmap& bm)
        : SkBitmapSampler(bm, false, SkShader::kRepeat_TileMode, SkShader::kRepeat_TileMode)
    {
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        x = do_repeat_mod(SkFixedFloor(x), fMaxX);
        y = do_repeat_mod(SkFixedFloor(y), fMaxY);
        return BITMAP_PIXEL_TO_PMCOLOR(fBitmap, x, y);
    }
};

class BITMAP_CLASSNAME_PREFIX(_Point_Mirror_Pow2_Sampler) : public SkBitmapSampler {
public:
    BITMAP_CLASSNAME_PREFIX(_Point_Mirror_Pow2_Sampler)(const SkBitmap& bm)
        : SkBitmapSampler(bm, false, SkShader::kMirror_TileMode, SkShader::kMirror_TileMode)
    {
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        x = do_mirror_pow2(SkFixedFloor(x), fMaxX);
        y = do_mirror_pow2(SkFixedFloor(y), fMaxY);
        return BITMAP_PIXEL_TO_PMCOLOR(fBitmap, x, y);
    }
};

class BITMAP_CLASSNAME_PREFIX(_Point_Mirror_Mod_Sampler) : public SkBitmapSampler {
public:
    BITMAP_CLASSNAME_PREFIX(_Point_Mirror_Mod_Sampler)(const SkBitmap& bm)
        : SkBitmapSampler(bm, false, SkShader::kMirror_TileMode, SkShader::kMirror_TileMode)
    {
    }

    virtual SkPMColor sample(SkFixed x, SkFixed y) const
    {
        x = do_mirror_mod(SkFixedFloor(x), fMaxX);
        y = do_mirror_mod(SkFixedFloor(y), fMaxY);
        return BITMAP_PIXEL_TO_PMCOLOR(fBitmap, x, y);
    }
};

#undef BITMAP_CLASSNAME_PREFIX
#undef BITMAP_PIXEL_TO_PMCOLOR
