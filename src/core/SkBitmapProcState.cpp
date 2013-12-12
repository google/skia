
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBitmapProcState.h"
#include "SkColorPriv.h"
#include "SkFilterProc.h"
#include "SkPaint.h"
#include "SkShader.h"   // for tilemodes
#include "SkUtilsArm.h"
#include "SkBitmapScaler.h"
#include "SkMipMap.h"
#include "SkPixelRef.h"
#include "SkScaledImageCache.h"

#if !SK_ARM_NEON_IS_NONE
// These are defined in src/opts/SkBitmapProcState_arm_neon.cpp
extern const SkBitmapProcState::SampleProc16 gSkBitmapProcStateSample16_neon[];
extern const SkBitmapProcState::SampleProc32 gSkBitmapProcStateSample32_neon[];
extern void  S16_D16_filter_DX_neon(const SkBitmapProcState&, const uint32_t*, int, uint16_t*);
extern void  Clamp_S16_D16_filter_DX_shaderproc_neon(const SkBitmapProcState&, int, int, uint16_t*, int);
extern void  Repeat_S16_D16_filter_DX_shaderproc_neon(const SkBitmapProcState&, int, int, uint16_t*, int);
extern void  SI8_opaque_D32_filter_DX_neon(const SkBitmapProcState&, const uint32_t*, int, SkPMColor*);
extern void  SI8_opaque_D32_filter_DX_shaderproc_neon(const SkBitmapProcState&, int, int, uint32_t*, int);
extern void  Clamp_SI8_opaque_D32_filter_DX_shaderproc_neon(const SkBitmapProcState&, int, int, uint32_t*, int);
#endif

#define   NAME_WRAP(x)  x
#include "SkBitmapProcState_filter.h"
#include "SkBitmapProcState_procs.h"

///////////////////////////////////////////////////////////////////////////////

// true iff the matrix contains, at most, scale and translate elements
static bool matrix_only_scale_translate(const SkMatrix& m) {
    return m.getType() <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask);
}

/**
 *  For the purposes of drawing bitmaps, if a matrix is "almost" translate
 *  go ahead and treat it as if it were, so that subsequent code can go fast.
 */
static bool just_trans_clamp(const SkMatrix& matrix, const SkBitmap& bitmap) {
    SkASSERT(matrix_only_scale_translate(matrix));

    if (matrix.getType() & SkMatrix::kScale_Mask) {
        SkRect src, dst;
        bitmap.getBounds(&src);

        // Can't call mapRect(), since that will fix up inverted rectangles,
        // e.g. when scale is negative, and we don't want to return true for
        // those.
        matrix.mapPoints(SkTCast<SkPoint*>(&dst),
                         SkTCast<const SkPoint*>(&src),
                         2);

        // Now round all 4 edges to device space, and then compare the device
        // width/height to the original. Note: we must map all 4 and subtract
        // rather than map the "width" and compare, since we care about the
        // phase (in pixel space) that any translate in the matrix might impart.
        SkIRect idst;
        dst.round(&idst);
        return idst.width() == bitmap.width() && idst.height() == bitmap.height();
    }
    // if we got here, we're either kTranslate_Mask or identity
    return true;
}

static bool just_trans_general(const SkMatrix& matrix) {
    SkASSERT(matrix_only_scale_translate(matrix));

    if (matrix.getType() & SkMatrix::kScale_Mask) {
        const SkScalar tol = SK_Scalar1 / 32768;

        if (!SkScalarNearlyZero(matrix[SkMatrix::kMScaleX] - SK_Scalar1, tol)) {
            return false;
        }
        if (!SkScalarNearlyZero(matrix[SkMatrix::kMScaleY] - SK_Scalar1, tol)) {
            return false;
        }
    }
    // if we got here, treat us as either kTranslate_Mask or identity
    return true;
}

///////////////////////////////////////////////////////////////////////////////

static bool valid_for_filtering(unsigned dimension) {
    // for filtering, width and height must fit in 14bits, since we use steal
    // 2 bits from each to store our 4bit subpixel data
    return (dimension & ~0x3FFF) == 0;
}

static SkScalar effective_matrix_scale_sqrd(const SkMatrix& mat) {
    SkPoint v1, v2;

    v1.fX = mat.getScaleX();
    v1.fY = mat.getSkewY();

    v2.fX = mat.getSkewX();
    v2.fY = mat.getScaleY();

    return SkMaxScalar(v1.lengthSqd(), v2.lengthSqd());
}

class AutoScaledCacheUnlocker {
public:
    AutoScaledCacheUnlocker(SkScaledImageCache::ID** idPtr) : fIDPtr(idPtr) {}
    ~AutoScaledCacheUnlocker() {
        if (fIDPtr && *fIDPtr) {
            SkScaledImageCache::Unlock(*fIDPtr);
            *fIDPtr = NULL;
        }
    }

    // forgets the ID, so it won't call Unlock
    void release() {
        fIDPtr = NULL;
    }

private:
    SkScaledImageCache::ID** fIDPtr;
};
#define AutoScaledCacheUnlocker(...) SK_REQUIRE_LOCAL_VAR(AutoScaledCacheUnlocker)

// TODO -- we may want to pass the clip into this function so we only scale
// the portion of the image that we're going to need.  This will complicate
// the interface to the cache, but might be well worth it.

bool SkBitmapProcState::possiblyScaleImage() {
    AutoScaledCacheUnlocker unlocker(&fScaledCacheID);

    SkASSERT(NULL == fBitmap);
    SkASSERT(NULL == fScaledCacheID);

    if (fFilterLevel <= SkPaint::kLow_FilterLevel) {
        return false;
    }

    // Check to see if the transformation matrix is simple, and if we're
    // doing high quality scaling.  If so, do the bitmap scale here and
    // remove the scaling component from the matrix.

    if (SkPaint::kHigh_FilterLevel == fFilterLevel &&
        fInvMatrix.getType() <= (SkMatrix::kScale_Mask | SkMatrix::kTranslate_Mask) &&
        fOrigBitmap.config() == SkBitmap::kARGB_8888_Config) {

        SkScalar invScaleX = fInvMatrix.getScaleX();
        SkScalar invScaleY = fInvMatrix.getScaleY();

        fScaledCacheID = SkScaledImageCache::FindAndLock(fOrigBitmap,
                                                         invScaleX, invScaleY,
                                                         &fScaledBitmap);
        if (fScaledCacheID) {
            fScaledBitmap.lockPixels();
            if (!fScaledBitmap.getPixels()) {
                fScaledBitmap.unlockPixels();
                // found a purged entry (discardablememory?), release it
                SkScaledImageCache::Unlock(fScaledCacheID);
                fScaledCacheID = NULL;
                // fall through to rebuild
            }
        }

        if (NULL == fScaledCacheID) {
            int dest_width  = SkScalarCeilToInt(fOrigBitmap.width() / invScaleX);
            int dest_height = SkScalarCeilToInt(fOrigBitmap.height() / invScaleY);

            // All the criteria are met; let's make a new bitmap.

            SkConvolutionProcs simd;
            sk_bzero(&simd, sizeof(simd));
            this->platformConvolutionProcs(&simd);

            if (!SkBitmapScaler::Resize(&fScaledBitmap,
                                        fOrigBitmap,
                                        SkBitmapScaler::RESIZE_BEST,
                                        dest_width,
                                        dest_height,
                                        simd,
                                        SkScaledImageCache::GetAllocator())) {
                // we failed to create fScaledBitmap, so just return and let
                // the scanline proc handle it.
                return false;

            }
            SkASSERT(NULL != fScaledBitmap.getPixels());
            fScaledCacheID = SkScaledImageCache::AddAndLock(fOrigBitmap,
                                                            invScaleX,
                                                            invScaleY,
                                                            fScaledBitmap);
            if (!fScaledCacheID) {
                fScaledBitmap.reset();
                return false;
            }
            SkASSERT(NULL != fScaledBitmap.getPixels());
        }

        SkASSERT(NULL != fScaledBitmap.getPixels());
        fBitmap = &fScaledBitmap;

        // set the inv matrix type to translate-only;
        fInvMatrix.setTranslate(fInvMatrix.getTranslateX() / fInvMatrix.getScaleX(),
                                fInvMatrix.getTranslateY() / fInvMatrix.getScaleY());

        // no need for any further filtering; we just did it!
        fFilterLevel = SkPaint::kNone_FilterLevel;
        unlocker.release();
        return true;
    }

    /*
     *  If High, then our special-case for scale-only did not take, and so we
     *  have to make a choice:
     *      1. fall back on mipmaps + bilerp
     *      2. fall back on scanline bicubic filter
     *  For now, we compute the "scale" value from the matrix, and have a
     *  threshold to decide when bicubic is better, and when mips are better.
     *  No doubt a fancier decision tree could be used uere.
     *
     *  If Medium, then we just try to build a mipmap and select a level,
     *  setting the filter-level to kLow to signal that we just need bilerp
     *  to process the selected level.
     */

    SkScalar scaleSqd = effective_matrix_scale_sqrd(fInvMatrix);

    if (SkPaint::kHigh_FilterLevel == fFilterLevel) {
        // Set the limit at 0.25 for the CTM... if the CTM is scaling smaller
        // than this, then the mipmaps quality may be greater (certainly faster)
        // so we only keep High quality if the scale is greater than this.
        //
        // Since we're dealing with the inverse, we compare against its inverse.
        const SkScalar bicubicLimit = 4.0f;
        const SkScalar bicubicLimitSqd = bicubicLimit * bicubicLimit;
        if (scaleSqd < bicubicLimitSqd) {  // use bicubic scanline
            return false;
        }

        // else set the filter-level to Medium, since we're scaling down and
        // want to reqeust mipmaps
        fFilterLevel = SkPaint::kMedium_FilterLevel;
    }

    SkASSERT(SkPaint::kMedium_FilterLevel == fFilterLevel);

    /**
     *  Medium quality means use a mipmap for down-scaling, and just bilper
     *  for upscaling. Since we're examining the inverse matrix, we look for
     *  a scale > 1 to indicate down scaling by the CTM.
     */
    if (scaleSqd > SK_Scalar1) {
        const SkMipMap* mip = NULL;

        SkASSERT(NULL == fScaledCacheID);
        fScaledCacheID = SkScaledImageCache::FindAndLockMip(fOrigBitmap, &mip);
        if (!fScaledCacheID) {
            SkASSERT(NULL == mip);
            mip = SkMipMap::Build(fOrigBitmap);
            if (mip) {
                fScaledCacheID = SkScaledImageCache::AddAndLockMip(fOrigBitmap,
                                                                   mip);
                mip->unref();   // the cache took a ref
                SkASSERT(fScaledCacheID);
            }
        } else {
            SkASSERT(mip);
        }

        if (mip) {
            SkScalar levelScale = SkScalarInvert(SkScalarSqrt(scaleSqd));
            SkMipMap::Level level;
            if (mip->extractLevel(levelScale, &level)) {
                SkScalar invScaleFixup = level.fScale;
                fInvMatrix.postScale(invScaleFixup, invScaleFixup);

                fScaledBitmap.setConfig(fOrigBitmap.config(),
                                        level.fWidth, level.fHeight,
                                        level.fRowBytes);
                fScaledBitmap.setPixels(level.fPixels);
                fBitmap = &fScaledBitmap;
                fFilterLevel = SkPaint::kLow_FilterLevel;
                unlocker.release();
                return true;
            }
        }
    }

    return false;
}

static bool get_locked_pixels(const SkBitmap& src, int pow2, SkBitmap* dst) {
    SkPixelRef* pr = src.pixelRef();
    if (pr && pr->decodeInto(pow2, dst)) {
        return true;
    }

    /*
     *  If decodeInto() fails, it is possibe that we have an old subclass that
     *  does not, or cannot, implement that. In that case we fall back to the
     *  older protocol of having the pixelRef handle the caching for us.
     */
    *dst = src;
    dst->lockPixels();
    return SkToBool(dst->getPixels());
}

bool SkBitmapProcState::lockBaseBitmap() {
    AutoScaledCacheUnlocker unlocker(&fScaledCacheID);

    SkPixelRef* pr = fOrigBitmap.pixelRef();

    SkASSERT(NULL == fScaledCacheID);

    if (pr->isLocked() || !pr->implementsDecodeInto()) {
        // fast-case, no need to look in our cache
        fScaledBitmap = fOrigBitmap;
        fScaledBitmap.lockPixels();
        if (NULL == fScaledBitmap.getPixels()) {
            return false;
        }
    } else {
        fScaledCacheID = SkScaledImageCache::FindAndLock(fOrigBitmap,
                                                         SK_Scalar1, SK_Scalar1,
                                                         &fScaledBitmap);
        if (fScaledCacheID) {
            fScaledBitmap.lockPixels();
            if (!fScaledBitmap.getPixels()) {
                fScaledBitmap.unlockPixels();
                // found a purged entry (discardablememory?), release it
                SkScaledImageCache::Unlock(fScaledCacheID);
                fScaledCacheID = NULL;
                // fall through to rebuild
            }
        }

        if (NULL == fScaledCacheID) {
            if (!get_locked_pixels(fOrigBitmap, 0, &fScaledBitmap)) {
                return false;
            }

            // TODO: if fScaled comes back at a different width/height than fOrig,
            // we need to update the matrix we are using to sample from this guy.

            fScaledCacheID = SkScaledImageCache::AddAndLock(fOrigBitmap,
                                                            SK_Scalar1, SK_Scalar1,
                                                            fScaledBitmap);
            if (!fScaledCacheID) {
                fScaledBitmap.reset();
                return false;
            }
        }
    }
    fBitmap = &fScaledBitmap;
    unlocker.release();
    return true;
}

void SkBitmapProcState::endContext() {
    SkDELETE(fBitmapFilter);
    fBitmapFilter = NULL;
    fScaledBitmap.reset();

    if (fScaledCacheID) {
        SkScaledImageCache::Unlock(fScaledCacheID);
        fScaledCacheID = NULL;
    }
}

SkBitmapProcState::~SkBitmapProcState() {
    if (fScaledCacheID) {
        SkScaledImageCache::Unlock(fScaledCacheID);
    }
    SkDELETE(fBitmapFilter);
}

bool SkBitmapProcState::chooseProcs(const SkMatrix& inv, const SkPaint& paint) {
    SkASSERT(fOrigBitmap.width() && fOrigBitmap.height());

    fBitmap = NULL;
    fInvMatrix = inv;
    fFilterLevel = paint.getFilterLevel();

    SkASSERT(NULL == fScaledCacheID);

    // possiblyScaleImage will look to see if it can rescale the image as a
    // preprocess; either by scaling up to the target size, or by selecting
    // a nearby mipmap level.  If it does, it will adjust the working
    // matrix as well as the working bitmap.  It may also adjust the filter
    // quality to avoid re-filtering an already perfectly scaled image.
    if (!this->possiblyScaleImage()) {
        if (!this->lockBaseBitmap()) {
            return false;
        }
    }
    // The above logic should have always assigned fBitmap, but in case it
    // didn't, we check for that now...
    if (NULL == fBitmap) {
        return false;
    }

    bool trivialMatrix = (fInvMatrix.getType() & ~SkMatrix::kTranslate_Mask) == 0;
    bool clampClamp = SkShader::kClamp_TileMode == fTileModeX &&
                      SkShader::kClamp_TileMode == fTileModeY;

    if (!(clampClamp || trivialMatrix)) {
        fInvMatrix.postIDiv(fOrigBitmap.width(), fOrigBitmap.height());
    }

    // Now that all possible changes to the matrix have taken place, check
    // to see if we're really close to a no-scale matrix.  If so, explicitly
    // set it to be so.  Subsequent code may inspect this matrix to choose
    // a faster path in this case.

    // This code will only execute if the matrix has some scale component;
    // if it's already pure translate then we won't do this inversion.

    if (matrix_only_scale_translate(fInvMatrix)) {
        SkMatrix forward;
        if (fInvMatrix.invert(&forward)) {
            if (clampClamp ? just_trans_clamp(forward, *fBitmap)
                            : just_trans_general(forward)) {
                SkScalar tx = -SkScalarRoundToScalar(forward.getTranslateX());
                SkScalar ty = -SkScalarRoundToScalar(forward.getTranslateY());
                fInvMatrix.setTranslate(tx, ty);
            }
        }
    }

    fInvProc        = fInvMatrix.getMapXYProc();
    fInvType        = fInvMatrix.getType();
    fInvSx          = SkScalarToFixed(fInvMatrix.getScaleX());
    fInvSxFractionalInt = SkScalarToFractionalInt(fInvMatrix.getScaleX());
    fInvKy          = SkScalarToFixed(fInvMatrix.getSkewY());
    fInvKyFractionalInt = SkScalarToFractionalInt(fInvMatrix.getSkewY());

    fAlphaScale = SkAlpha255To256(paint.getAlpha());

    fShaderProc32 = NULL;
    fShaderProc16 = NULL;
    fSampleProc32 = NULL;
    fSampleProc16 = NULL;

    // recompute the triviality of the matrix here because we may have
    // changed it!

    trivialMatrix = (fInvMatrix.getType() & ~SkMatrix::kTranslate_Mask) == 0;

    if (SkPaint::kHigh_FilterLevel == fFilterLevel) {
        // If this is still set, that means we wanted HQ sampling
        // but couldn't do it as a preprocess.  Let's try to install
        // the scanline version of the HQ sampler.  If that process fails,
        // downgrade to bilerp.

        // NOTE: Might need to be careful here in the future when we want
        // to have the platform proc have a shot at this; it's possible that
        // the chooseBitmapFilterProc will fail to install a shader but a
        // platform-specific one might succeed, so it might be premature here
        // to fall back to bilerp.  This needs thought.

        if (!this->setBitmapFilterProcs()) {
            fFilterLevel = SkPaint::kLow_FilterLevel;
        }
    }

    if (SkPaint::kLow_FilterLevel == fFilterLevel) {
        // Only try bilerp if the matrix is "interesting" and
        // the image has a suitable size.

        if (fInvType <= SkMatrix::kTranslate_Mask ||
                !valid_for_filtering(fBitmap->width() | fBitmap->height())) {
            fFilterLevel = SkPaint::kNone_FilterLevel;
        }
    }

    // At this point, we know exactly what kind of sampling the per-scanline
    // shader will perform.

    fMatrixProc = this->chooseMatrixProc(trivialMatrix);
    if (NULL == fMatrixProc) {
        return false;
    }

    ///////////////////////////////////////////////////////////////////////

    // No need to do this if we're doing HQ sampling; if filter quality is
    // still set to HQ by the time we get here, then we must have installed
    // the shader procs above and can skip all this.

    if (fFilterLevel < SkPaint::kHigh_FilterLevel) {

        int index = 0;
        if (fAlphaScale < 256) {  // note: this distinction is not used for D16
            index |= 1;
        }
        if (fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) {
            index |= 2;
        }
        if (fFilterLevel > SkPaint::kNone_FilterLevel) {
            index |= 4;
        }
        // bits 3,4,5 encoding the source bitmap format
        switch (fBitmap->config()) {
            case SkBitmap::kARGB_8888_Config:
                index |= 0;
                break;
            case SkBitmap::kRGB_565_Config:
                index |= 8;
                break;
            case SkBitmap::kIndex8_Config:
                index |= 16;
                break;
            case SkBitmap::kARGB_4444_Config:
                index |= 24;
                break;
            case SkBitmap::kA8_Config:
                index |= 32;
                fPaintPMColor = SkPreMultiplyColor(paint.getColor());
                break;
            default:
                return false;
        }

    #if !SK_ARM_NEON_IS_ALWAYS
        static const SampleProc32 gSkBitmapProcStateSample32[] = {
            S32_opaque_D32_nofilter_DXDY,
            S32_alpha_D32_nofilter_DXDY,
            S32_opaque_D32_nofilter_DX,
            S32_alpha_D32_nofilter_DX,
            S32_opaque_D32_filter_DXDY,
            S32_alpha_D32_filter_DXDY,
            S32_opaque_D32_filter_DX,
            S32_alpha_D32_filter_DX,

            S16_opaque_D32_nofilter_DXDY,
            S16_alpha_D32_nofilter_DXDY,
            S16_opaque_D32_nofilter_DX,
            S16_alpha_D32_nofilter_DX,
            S16_opaque_D32_filter_DXDY,
            S16_alpha_D32_filter_DXDY,
            S16_opaque_D32_filter_DX,
            S16_alpha_D32_filter_DX,

            SI8_opaque_D32_nofilter_DXDY,
            SI8_alpha_D32_nofilter_DXDY,
            SI8_opaque_D32_nofilter_DX,
            SI8_alpha_D32_nofilter_DX,
            SI8_opaque_D32_filter_DXDY,
            SI8_alpha_D32_filter_DXDY,
            SI8_opaque_D32_filter_DX,
            SI8_alpha_D32_filter_DX,

            S4444_opaque_D32_nofilter_DXDY,
            S4444_alpha_D32_nofilter_DXDY,
            S4444_opaque_D32_nofilter_DX,
            S4444_alpha_D32_nofilter_DX,
            S4444_opaque_D32_filter_DXDY,
            S4444_alpha_D32_filter_DXDY,
            S4444_opaque_D32_filter_DX,
            S4444_alpha_D32_filter_DX,

            // A8 treats alpha/opaque the same (equally efficient)
            SA8_alpha_D32_nofilter_DXDY,
            SA8_alpha_D32_nofilter_DXDY,
            SA8_alpha_D32_nofilter_DX,
            SA8_alpha_D32_nofilter_DX,
            SA8_alpha_D32_filter_DXDY,
            SA8_alpha_D32_filter_DXDY,
            SA8_alpha_D32_filter_DX,
            SA8_alpha_D32_filter_DX
        };

        static const SampleProc16 gSkBitmapProcStateSample16[] = {
            S32_D16_nofilter_DXDY,
            S32_D16_nofilter_DX,
            S32_D16_filter_DXDY,
            S32_D16_filter_DX,

            S16_D16_nofilter_DXDY,
            S16_D16_nofilter_DX,
            S16_D16_filter_DXDY,
            S16_D16_filter_DX,

            SI8_D16_nofilter_DXDY,
            SI8_D16_nofilter_DX,
            SI8_D16_filter_DXDY,
            SI8_D16_filter_DX,

            // Don't support 4444 -> 565
            NULL, NULL, NULL, NULL,
            // Don't support A8 -> 565
            NULL, NULL, NULL, NULL
        };
    #endif

        fSampleProc32 = SK_ARM_NEON_WRAP(gSkBitmapProcStateSample32)[index];
        index >>= 1;    // shift away any opaque/alpha distinction
        fSampleProc16 = SK_ARM_NEON_WRAP(gSkBitmapProcStateSample16)[index];

        // our special-case shaderprocs
        if (SK_ARM_NEON_WRAP(S16_D16_filter_DX) == fSampleProc16) {
            if (clampClamp) {
                fShaderProc16 = SK_ARM_NEON_WRAP(Clamp_S16_D16_filter_DX_shaderproc);
            } else if (SkShader::kRepeat_TileMode == fTileModeX &&
                       SkShader::kRepeat_TileMode == fTileModeY) {
                fShaderProc16 = SK_ARM_NEON_WRAP(Repeat_S16_D16_filter_DX_shaderproc);
            }
        } else if (SK_ARM_NEON_WRAP(SI8_opaque_D32_filter_DX) == fSampleProc32 && clampClamp) {
            fShaderProc32 = SK_ARM_NEON_WRAP(Clamp_SI8_opaque_D32_filter_DX_shaderproc);
        }

        if (NULL == fShaderProc32) {
            fShaderProc32 = this->chooseShaderProc32();
        }
    }

    // see if our platform has any accelerated overrides
    this->platformProcs();

    return true;
}

static void Clamp_S32_D32_nofilter_trans_shaderproc(const SkBitmapProcState& s,
                                                    int x, int y,
                                                    SkPMColor* SK_RESTRICT colors,
                                                    int count) {
    SkASSERT(((s.fInvType & ~SkMatrix::kTranslate_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(SkPaint::kNone_FilterLevel == s.fFilterLevel);

    const int maxX = s.fBitmap->width() - 1;
    const int maxY = s.fBitmap->height() - 1;
    int ix = s.fFilterOneX + x;
    int iy = SkClampMax(s.fFilterOneY + y, maxY);
#ifdef SK_DEBUG
    {
        SkPoint pt;
        s.fInvProc(s.fInvMatrix, SkIntToScalar(x) + SK_ScalarHalf,
                   SkIntToScalar(y) + SK_ScalarHalf, &pt);
        int iy2 = SkClampMax(SkScalarFloorToInt(pt.fY), maxY);
        int ix2 = SkScalarFloorToInt(pt.fX);

        SkASSERT(iy == iy2);
        SkASSERT(ix == ix2);
    }
#endif
    const SkPMColor* row = s.fBitmap->getAddr32(0, iy);

    // clamp to the left
    if (ix < 0) {
        int n = SkMin32(-ix, count);
        sk_memset32(colors, row[0], n);
        count -= n;
        if (0 == count) {
            return;
        }
        colors += n;
        SkASSERT(-ix == n);
        ix = 0;
    }
    // copy the middle
    if (ix <= maxX) {
        int n = SkMin32(maxX - ix + 1, count);
        memcpy(colors, row + ix, n * sizeof(SkPMColor));
        count -= n;
        if (0 == count) {
            return;
        }
        colors += n;
    }
    SkASSERT(count > 0);
    // clamp to the right
    sk_memset32(colors, row[maxX], count);
}

static inline int sk_int_mod(int x, int n) {
    SkASSERT(n > 0);
    if ((unsigned)x >= (unsigned)n) {
        if (x < 0) {
            x = n + ~(~x % n);
        } else {
            x = x % n;
        }
    }
    return x;
}

static inline int sk_int_mirror(int x, int n) {
    x = sk_int_mod(x, 2 * n);
    if (x >= n) {
        x = n + ~(x - n);
    }
    return x;
}

static void Repeat_S32_D32_nofilter_trans_shaderproc(const SkBitmapProcState& s,
                                                     int x, int y,
                                                     SkPMColor* SK_RESTRICT colors,
                                                     int count) {
    SkASSERT(((s.fInvType & ~SkMatrix::kTranslate_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(SkPaint::kNone_FilterLevel == s.fFilterLevel);

    const int stopX = s.fBitmap->width();
    const int stopY = s.fBitmap->height();
    int ix = s.fFilterOneX + x;
    int iy = sk_int_mod(s.fFilterOneY + y, stopY);
#ifdef SK_DEBUG
    {
        SkPoint pt;
        s.fInvProc(s.fInvMatrix, SkIntToScalar(x) + SK_ScalarHalf,
                   SkIntToScalar(y) + SK_ScalarHalf, &pt);
        int iy2 = sk_int_mod(SkScalarFloorToInt(pt.fY), stopY);
        int ix2 = SkScalarFloorToInt(pt.fX);

        SkASSERT(iy == iy2);
        SkASSERT(ix == ix2);
    }
#endif
    const SkPMColor* row = s.fBitmap->getAddr32(0, iy);

    ix = sk_int_mod(ix, stopX);
    for (;;) {
        int n = SkMin32(stopX - ix, count);
        memcpy(colors, row + ix, n * sizeof(SkPMColor));
        count -= n;
        if (0 == count) {
            return;
        }
        colors += n;
        ix = 0;
    }
}

static void S32_D32_constX_shaderproc(const SkBitmapProcState& s,
                                      int x, int y,
                                      SkPMColor* SK_RESTRICT colors,
                                      int count) {
    SkASSERT((s.fInvType & ~(SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) == 0);
    SkASSERT(s.fInvKy == 0);
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(1 == s.fBitmap->width());

    int iY0;
    int iY1   SK_INIT_TO_AVOID_WARNING;
    int iSubY SK_INIT_TO_AVOID_WARNING;

    if (SkPaint::kNone_FilterLevel != s.fFilterLevel) {
        SkBitmapProcState::MatrixProc mproc = s.getMatrixProc();
        uint32_t xy[2];

        mproc(s, xy, 1, x, y);

        iY0 = xy[0] >> 18;
        iY1 = xy[0] & 0x3FFF;
        iSubY = (xy[0] >> 14) & 0xF;
    } else {
        int yTemp;

        if (s.fInvType > SkMatrix::kTranslate_Mask) {
            SkPoint pt;
            s.fInvProc(s.fInvMatrix,
                       SkIntToScalar(x) + SK_ScalarHalf,
                       SkIntToScalar(y) + SK_ScalarHalf,
                       &pt);
            // When the matrix has a scale component the setup code in
            // chooseProcs multiples the inverse matrix by the inverse of the
            // bitmap's width and height. Since this method is going to do
            // its own tiling and sampling we need to undo that here.
            if (SkShader::kClamp_TileMode != s.fTileModeX ||
                SkShader::kClamp_TileMode != s.fTileModeY) {
                yTemp = SkScalarFloorToInt(pt.fY * s.fBitmap->height());
            } else {
                yTemp = SkScalarFloorToInt(pt.fY);
            }
        } else {
            yTemp = s.fFilterOneY + y;
        }

        const int stopY = s.fBitmap->height();
        switch (s.fTileModeY) {
            case SkShader::kClamp_TileMode:
                iY0 = SkClampMax(yTemp, stopY-1);
                break;
            case SkShader::kRepeat_TileMode:
                iY0 = sk_int_mod(yTemp, stopY);
                break;
            case SkShader::kMirror_TileMode:
            default:
                iY0 = sk_int_mirror(yTemp, stopY);
                break;
        }

#ifdef SK_DEBUG
        {
            SkPoint pt;
            s.fInvProc(s.fInvMatrix,
                       SkIntToScalar(x) + SK_ScalarHalf,
                       SkIntToScalar(y) + SK_ScalarHalf,
                       &pt);
            if (s.fInvType > SkMatrix::kTranslate_Mask &&
                (SkShader::kClamp_TileMode != s.fTileModeX ||
                 SkShader::kClamp_TileMode != s.fTileModeY)) {
                pt.fY *= s.fBitmap->height();
            }
            int iY2;

            switch (s.fTileModeY) {
            case SkShader::kClamp_TileMode:
                iY2 = SkClampMax(SkScalarFloorToInt(pt.fY), stopY-1);
                break;
            case SkShader::kRepeat_TileMode:
                iY2 = sk_int_mod(SkScalarFloorToInt(pt.fY), stopY);
                break;
            case SkShader::kMirror_TileMode:
            default:
                iY2 = sk_int_mirror(SkScalarFloorToInt(pt.fY), stopY);
                break;
            }

            SkASSERT(iY0 == iY2);
        }
#endif
    }

    const SkPMColor* row0 = s.fBitmap->getAddr32(0, iY0);
    SkPMColor color;

    if (SkPaint::kNone_FilterLevel != s.fFilterLevel) {
        const SkPMColor* row1 = s.fBitmap->getAddr32(0, iY1);

        if (s.fAlphaScale < 256) {
            Filter_32_alpha(iSubY, *row0, *row1, &color, s.fAlphaScale);
        } else {
            Filter_32_opaque(iSubY, *row0, *row1, &color);
        }
    } else {
        if (s.fAlphaScale < 256) {
            color = SkAlphaMulQ(*row0, s.fAlphaScale);
        } else {
            color = *row0;
        }
    }

    sk_memset32(colors, color, count);
}

static void DoNothing_shaderproc(const SkBitmapProcState&, int x, int y,
                                 SkPMColor* SK_RESTRICT colors, int count) {
    // if we get called, the matrix is too tricky, so we just draw nothing
    sk_memset32(colors, 0, count);
}

bool SkBitmapProcState::setupForTranslate() {
    SkPoint pt;
    fInvProc(fInvMatrix, SK_ScalarHalf, SK_ScalarHalf, &pt);

    /*
     *  if the translate is larger than our ints, we can get random results, or
     *  worse, we might get 0x80000000, which wreaks havoc on us, since we can't
     *  negate it.
     */
    const SkScalar too_big = SkIntToScalar(1 << 30);
    if (SkScalarAbs(pt.fX) > too_big || SkScalarAbs(pt.fY) > too_big) {
        return false;
    }

    // Since we know we're not filtered, we re-purpose these fields allow
    // us to go from device -> src coordinates w/ just an integer add,
    // rather than running through the inverse-matrix
    fFilterOneX = SkScalarFloorToInt(pt.fX);
    fFilterOneY = SkScalarFloorToInt(pt.fY);
    return true;
}

SkBitmapProcState::ShaderProc32 SkBitmapProcState::chooseShaderProc32() {

    if (SkBitmap::kARGB_8888_Config != fBitmap->config()) {
        return NULL;
    }

    static const unsigned kMask = SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask;

    if (1 == fBitmap->width() && 0 == (fInvType & ~kMask)) {
        if (SkPaint::kNone_FilterLevel == fFilterLevel &&
            fInvType <= SkMatrix::kTranslate_Mask &&
            !this->setupForTranslate()) {
            return DoNothing_shaderproc;
        }
        return S32_D32_constX_shaderproc;
    }

    if (fAlphaScale < 256) {
        return NULL;
    }
    if (fInvType > SkMatrix::kTranslate_Mask) {
        return NULL;
    }
    if (SkPaint::kNone_FilterLevel != fFilterLevel) {
        return NULL;
    }

    SkShader::TileMode tx = (SkShader::TileMode)fTileModeX;
    SkShader::TileMode ty = (SkShader::TileMode)fTileModeY;

    if (SkShader::kClamp_TileMode == tx && SkShader::kClamp_TileMode == ty) {
        if (this->setupForTranslate()) {
            return Clamp_S32_D32_nofilter_trans_shaderproc;
        }
        return DoNothing_shaderproc;
    }
    if (SkShader::kRepeat_TileMode == tx && SkShader::kRepeat_TileMode == ty) {
        if (this->setupForTranslate()) {
            return Repeat_S32_D32_nofilter_trans_shaderproc;
        }
        return DoNothing_shaderproc;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

static void check_scale_nofilter(uint32_t bitmapXY[], int count,
                                 unsigned mx, unsigned my) {
    unsigned y = *bitmapXY++;
    SkASSERT(y < my);

    const uint16_t* xptr = reinterpret_cast<const uint16_t*>(bitmapXY);
    for (int i = 0; i < count; ++i) {
        SkASSERT(xptr[i] < mx);
    }
}

static void check_scale_filter(uint32_t bitmapXY[], int count,
                                 unsigned mx, unsigned my) {
    uint32_t YY = *bitmapXY++;
    unsigned y0 = YY >> 18;
    unsigned y1 = YY & 0x3FFF;
    SkASSERT(y0 < my);
    SkASSERT(y1 < my);

    for (int i = 0; i < count; ++i) {
        uint32_t XX = bitmapXY[i];
        unsigned x0 = XX >> 18;
        unsigned x1 = XX & 0x3FFF;
        SkASSERT(x0 < mx);
        SkASSERT(x1 < mx);
    }
}

static void check_affine_nofilter(uint32_t bitmapXY[], int count,
                                 unsigned mx, unsigned my) {
    for (int i = 0; i < count; ++i) {
        uint32_t XY = bitmapXY[i];
        unsigned x = XY & 0xFFFF;
        unsigned y = XY >> 16;
        SkASSERT(x < mx);
        SkASSERT(y < my);
    }
}

static void check_affine_filter(uint32_t bitmapXY[], int count,
                                 unsigned mx, unsigned my) {
    for (int i = 0; i < count; ++i) {
        uint32_t YY = *bitmapXY++;
        unsigned y0 = YY >> 18;
        unsigned y1 = YY & 0x3FFF;
        SkASSERT(y0 < my);
        SkASSERT(y1 < my);

        uint32_t XX = *bitmapXY++;
        unsigned x0 = XX >> 18;
        unsigned x1 = XX & 0x3FFF;
        SkASSERT(x0 < mx);
        SkASSERT(x1 < mx);
    }
}

void SkBitmapProcState::DebugMatrixProc(const SkBitmapProcState& state,
                                        uint32_t bitmapXY[], int count,
                                        int x, int y) {
    SkASSERT(bitmapXY);
    SkASSERT(count > 0);

    state.fMatrixProc(state, bitmapXY, count, x, y);

    void (*proc)(uint32_t bitmapXY[], int count, unsigned mx, unsigned my);

    // There are four formats possible:
    //  scale -vs- affine
    //  filter -vs- nofilter
    if (state.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) {
        proc = state.fFilterLevel != SkPaint::kNone_FilterLevel ? check_scale_filter : check_scale_nofilter;
    } else {
        proc = state.fFilterLevel != SkPaint::kNone_FilterLevel ? check_affine_filter : check_affine_nofilter;
    }
    proc(bitmapXY, count, state.fBitmap->width(), state.fBitmap->height());
}

SkBitmapProcState::MatrixProc SkBitmapProcState::getMatrixProc() const {
    return DebugMatrixProc;
}

#endif

///////////////////////////////////////////////////////////////////////////////
/*
    The storage requirements for the different matrix procs are as follows,
    where each X or Y is 2 bytes, and N is the number of pixels/elements:

    scale/translate     nofilter      Y(4bytes) + N * X
    affine/perspective  nofilter      N * (X Y)
    scale/translate     filter        Y Y + N * (X X)
    affine/perspective  filter        N * (Y Y X X)
 */
int SkBitmapProcState::maxCountForBufferSize(size_t bufferSize) const {
    int32_t size = static_cast<int32_t>(bufferSize);

    size &= ~3; // only care about 4-byte aligned chunks
    if (fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask)) {
        size -= 4;   // the shared Y (or YY) coordinate
        if (size < 0) {
            size = 0;
        }
        size >>= 1;
    } else {
        size >>= 2;
    }

    if (fFilterLevel != SkPaint::kNone_FilterLevel) {
        size >>= 1;
    }

    return size;
}
