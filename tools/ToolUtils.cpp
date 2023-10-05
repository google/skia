/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/ToolUtils.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPicture.h"
#include "include/core/SkPixelRef.h"  // IWYU pragma: keep
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/SkColorData.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkFontPriv.h"
#include "tools/SkMetaData.h"

#include <cmath>
#include <cstring>

#ifdef SK_BUILD_FOR_WIN
#include "include/ports/SkTypeface_win.h"
#endif

using namespace skia_private;

namespace ToolUtils {

const char* alphatype_name(SkAlphaType at) {
    switch (at) {
        case kUnknown_SkAlphaType:  return "Unknown";
        case kOpaque_SkAlphaType:   return "Opaque";
        case kPremul_SkAlphaType:   return "Premul";
        case kUnpremul_SkAlphaType: return "Unpremul";
    }
    SkUNREACHABLE;
}

const char* colortype_name(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:            return "Unknown";
        case kAlpha_8_SkColorType:            return "Alpha_8";
        case kA16_unorm_SkColorType:          return "Alpha_16";
        case kA16_float_SkColorType:          return "A16_float";
        case kRGB_565_SkColorType:            return "RGB_565";
        case kARGB_4444_SkColorType:          return "ARGB_4444";
        case kRGBA_8888_SkColorType:          return "RGBA_8888";
        case kSRGBA_8888_SkColorType:         return "SRGBA_8888";
        case kRGB_888x_SkColorType:           return "RGB_888x";
        case kBGRA_8888_SkColorType:          return "BGRA_8888";
        case kRGBA_1010102_SkColorType:       return "RGBA_1010102";
        case kBGRA_1010102_SkColorType:       return "BGRA_1010102";
        case kRGB_101010x_SkColorType:        return "RGB_101010x";
        case kBGR_101010x_SkColorType:        return "BGR_101010x";
        case kBGR_101010x_XR_SkColorType:     return "BGR_101010x_XR";
        case kRGBA_10x6_SkColorType:          return "RGBA_10x6";
        case kGray_8_SkColorType:             return "Gray_8";
        case kRGBA_F16Norm_SkColorType:       return "RGBA_F16Norm";
        case kRGBA_F16_SkColorType:           return "RGBA_F16";
        case kRGBA_F32_SkColorType:           return "RGBA_F32";
        case kR8G8_unorm_SkColorType:         return "R8G8_unorm";
        case kR16G16_unorm_SkColorType:       return "R16G16_unorm";
        case kR16G16_float_SkColorType:       return "R16G16_float";
        case kR16G16B16A16_unorm_SkColorType: return "R16G16B16A16_unorm";
        case kR8_unorm_SkColorType:           return "R8_unorm";
    }
    SkUNREACHABLE;
}

const char* colortype_depth(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:            return "Unknown";
        case kAlpha_8_SkColorType:            return "A8";
        case kA16_unorm_SkColorType:          return "A16";
        case kA16_float_SkColorType:          return "AF16";
        case kRGB_565_SkColorType:            return "565";
        case kARGB_4444_SkColorType:          return "4444";
        case kRGBA_8888_SkColorType:          return "8888";
        case kSRGBA_8888_SkColorType:         return "8888";
        case kRGB_888x_SkColorType:           return "888";
        case kBGRA_8888_SkColorType:          return "8888";
        case kRGBA_1010102_SkColorType:       return "1010102";
        case kBGRA_1010102_SkColorType:       return "1010102";
        case kRGB_101010x_SkColorType:        return "101010";
        case kBGR_101010x_SkColorType:        return "101010";
        case kBGR_101010x_XR_SkColorType:     return "101010";
        case kRGBA_10x6_SkColorType:          return "10101010";
        case kGray_8_SkColorType:             return "G8";
        case kRGBA_F16Norm_SkColorType:       return "F16Norm";
        case kRGBA_F16_SkColorType:           return "F16";
        case kRGBA_F32_SkColorType:           return "F32";
        case kR8G8_unorm_SkColorType:         return "88";
        case kR16G16_unorm_SkColorType:       return "1616";
        case kR16G16_float_SkColorType:       return "F16F16";
        case kR16G16B16A16_unorm_SkColorType: return "16161616";
        case kR8_unorm_SkColorType:           return "R8";
    }
    SkUNREACHABLE;
}

const char* tilemode_name(SkTileMode mode) {
    switch (mode) {
        case SkTileMode::kClamp:  return "clamp";
        case SkTileMode::kRepeat: return "repeat";
        case SkTileMode::kMirror: return "mirror";
        case SkTileMode::kDecal:  return "decal";
    }
    SkUNREACHABLE;
}

SkColor color_to_565(SkColor color) {
    // Not a good idea to use this function for greyscale colors...
    // it will add an obvious purple or green tint.
    SkASSERT(SkColorGetR(color) != SkColorGetG(color) || SkColorGetR(color) != SkColorGetB(color) ||
             SkColorGetG(color) != SkColorGetB(color));

    SkPMColor pmColor = SkPreMultiplyColor(color);
    U16CPU    color16 = SkPixel32ToPixel16(pmColor);
    return SkPixel16ToColor(color16);
}

sk_sp<SkShader> create_checkerboard_shader(SkColor c1, SkColor c2, int size) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::MakeS32(2 * size, 2 * size, kPremul_SkAlphaType));
    bm.eraseColor(c1);
    bm.eraseArea(SkIRect::MakeLTRB(0, 0, size, size), c2);
    bm.eraseArea(SkIRect::MakeLTRB(size, size, 2 * size, 2 * size), c2);
    return bm.makeShader(SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions());
}

SkBitmap create_checkerboard_bitmap(int w, int h, SkColor c1, SkColor c2, int checkSize) {
    SkBitmap bitmap;
    bitmap.allocPixels(SkImageInfo::MakeS32(w, h, kPremul_SkAlphaType));
    SkCanvas canvas(bitmap);

    ToolUtils::draw_checkerboard(&canvas, c1, c2, checkSize);
    return bitmap;
}

sk_sp<SkImage> create_checkerboard_image(int w, int h, SkColor c1, SkColor c2, int checkSize) {
    auto surf = SkSurfaces::Raster(SkImageInfo::MakeN32Premul(w, h));
    ToolUtils::draw_checkerboard(surf->getCanvas(), c1, c2, checkSize);
    return surf->makeImageSnapshot();
}

void draw_checkerboard(SkCanvas* canvas, SkColor c1, SkColor c2, int size) {
    SkPaint paint;
    paint.setShader(create_checkerboard_shader(c1, c2, size));
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawPaint(paint);
}

int make_pixmaps(SkColorType ct,
                 SkAlphaType at,
                 bool withMips,
                 const SkColor4f colors[6],
                 SkPixmap pixmaps[6],
                 std::unique_ptr<char[]>* mem) {

    int levelSize = 32;
    int numMipLevels = withMips ? 6 : 1;
    size_t size = 0;
    SkImageInfo ii[6];
    size_t rowBytes[6];
    for (int level = 0; level < numMipLevels; ++level) {
        ii[level] = SkImageInfo::Make(levelSize, levelSize, ct, at);
        rowBytes[level] = ii[level].minRowBytes();
        // Make sure we test row bytes that aren't tight.
        if (!(level % 2)) {
            rowBytes[level] += (level + 1)*SkColorTypeBytesPerPixel(ii[level].colorType());
        }
        size += rowBytes[level]*ii[level].height();
        levelSize /= 2;
    }
    mem->reset(new char[size]);
    char* addr = mem->get();
    for (int level = 0; level < numMipLevels; ++level) {
        pixmaps[level].reset(ii[level], addr, rowBytes[level]);
        addr += rowBytes[level]*ii[level].height();
        pixmaps[level].erase(colors[level]);
    }
    return numMipLevels;
}

SkBitmap create_string_bitmap(int w, int h, SkColor c, int x, int y, int textSize,
                              const char* str) {
    SkBitmap bitmap;
    bitmap.allocN32Pixels(w, h);
    SkCanvas canvas(bitmap);

    SkPaint paint;
    paint.setColor(c);

    SkFont font(ToolUtils::create_portable_typeface(), textSize);

    canvas.clear(0x00000000);
    canvas.drawSimpleText(str,
                          strlen(str),
                          SkTextEncoding::kUTF8,
                          SkIntToScalar(x),
                          SkIntToScalar(y),
                          font,
                          paint);

    // Tag data as sRGB (without doing any color space conversion). Color-space aware configs
    // will process this correctly but legacy configs will render as if this returned N32.
    SkBitmap result;
    result.setInfo(SkImageInfo::MakeS32(w, h, kPremul_SkAlphaType));
    result.setPixelRef(sk_ref_sp(bitmap.pixelRef()), 0, 0);
    return result;
}

sk_sp<SkImage> create_string_image(int w, int h, SkColor c, int x, int y, int textSize,
                                   const char* str) {
    return create_string_bitmap(w, h, c, x, y, textSize, str).asImage();
}

void add_to_text_blob_w_len(SkTextBlobBuilder* builder,
                            const char*        text,
                            size_t             len,
                            SkTextEncoding     encoding,
                            const SkFont&      font,
                            SkScalar           x,
                            SkScalar           y) {
    int  count = font.countText(text, len, encoding);
    if (count < 1) {
        return;
    }
    auto run   = builder->allocRun(font, count, x, y);
    font.textToGlyphs(text, len, encoding, run.glyphs, count);
}

void add_to_text_blob(SkTextBlobBuilder* builder,
                      const char*        text,
                      const SkFont&      font,
                      SkScalar           x,
                      SkScalar           y) {
    add_to_text_blob_w_len(builder, text, strlen(text), SkTextEncoding::kUTF8, font, x, y);
}

void get_text_path(const SkFont&  font,
                   const void*    text,
                   size_t         length,
                   SkTextEncoding encoding,
                   SkPath*        dst,
                   const SkPoint  pos[]) {
    SkAutoToGlyphs        atg(font, text, length, encoding);
    const int             count = atg.count();
    AutoTArray<SkPoint> computedPos;
    if (pos == nullptr) {
        computedPos.reset(count);
        font.getPos(atg.glyphs(), count, &computedPos[0]);
        pos = computedPos.get();
    }

    struct Rec {
        SkPath*        fDst;
        const SkPoint* fPos;
    } rec = {dst, pos};
    font.getPaths(atg.glyphs(),
                  atg.count(),
                  [](const SkPath* src, const SkMatrix& mx, void* ctx) {
                      Rec* rec = (Rec*)ctx;
                      if (src) {
                          SkMatrix tmp(mx);
                          tmp.postTranslate(rec->fPos->fX, rec->fPos->fY);
                          rec->fDst->addPath(*src, tmp);
                      }
                      rec->fPos += 1;
                  },
                  &rec);
}

SkPath make_star(const SkRect& bounds, int numPts, int step) {
    SkASSERT(numPts != step);
    SkPathBuilder builder;
    builder.setFillType(SkPathFillType::kEvenOdd);
    builder.moveTo(0, -1);
    for (int i = 1; i < numPts; ++i) {
        int      idx   = i * step % numPts;
        SkScalar theta = idx * 2 * SK_ScalarPI / numPts + SK_ScalarPI / 2;
        SkScalar x     = SkScalarCos(theta);
        SkScalar y     = -SkScalarSin(theta);
        builder.lineTo(x, y);
    }
    SkPath path = builder.detach();
    path.transform(SkMatrix::RectToRect(path.getBounds(), bounds));
    return path;
}

static inline void norm_to_rgb(SkBitmap* bm, int x, int y, const SkVector3& norm) {
    SkASSERT(SkScalarNearlyEqual(norm.length(), 1.0f));
    unsigned char r      = static_cast<unsigned char>((0.5f * norm.fX + 0.5f) * 255);
    unsigned char g      = static_cast<unsigned char>((-0.5f * norm.fY + 0.5f) * 255);
    unsigned char b      = static_cast<unsigned char>((0.5f * norm.fZ + 0.5f) * 255);
    *bm->getAddr32(x, y) = SkPackARGB32(0xFF, r, g, b);
}

void create_hemi_normal_map(SkBitmap* bm, const SkIRect& dst) {
    const SkPoint center =
            SkPoint::Make(dst.fLeft + (dst.width() / 2.0f), dst.fTop + (dst.height() / 2.0f));
    const SkPoint halfSize = SkPoint::Make(dst.width() / 2.0f, dst.height() / 2.0f);

    SkVector3 norm;

    for (int y = dst.fTop; y < dst.fBottom; ++y) {
        for (int x = dst.fLeft; x < dst.fRight; ++x) {
            norm.fX = (x + 0.5f - center.fX) / halfSize.fX;
            norm.fY = (y + 0.5f - center.fY) / halfSize.fY;

            SkScalar tmp = norm.fX * norm.fX + norm.fY * norm.fY;
            if (tmp >= 1.0f) {
                norm.set(0.0f, 0.0f, 1.0f);
            } else {
                norm.fZ = sqrtf(1.0f - tmp);
            }

            norm_to_rgb(bm, x, y, norm);
        }
    }
}

void create_frustum_normal_map(SkBitmap* bm, const SkIRect& dst) {
    const SkPoint center =
            SkPoint::Make(dst.fLeft + (dst.width() / 2.0f), dst.fTop + (dst.height() / 2.0f));

    SkIRect inner = dst;
    inner.inset(dst.width() / 4, dst.height() / 4);

    SkPoint3       norm;
    const SkPoint3 left  = SkPoint3::Make(-SK_ScalarRoot2Over2, 0.0f, SK_ScalarRoot2Over2);
    const SkPoint3 up    = SkPoint3::Make(0.0f, -SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);
    const SkPoint3 right = SkPoint3::Make(SK_ScalarRoot2Over2, 0.0f, SK_ScalarRoot2Over2);
    const SkPoint3 down  = SkPoint3::Make(0.0f, SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);

    for (int y = dst.fTop; y < dst.fBottom; ++y) {
        for (int x = dst.fLeft; x < dst.fRight; ++x) {
            if (inner.contains(x, y)) {
                norm.set(0.0f, 0.0f, 1.0f);
            } else {
                SkScalar locX = x + 0.5f - center.fX;
                SkScalar locY = y + 0.5f - center.fY;

                if (locX >= 0.0f) {
                    if (locY > 0.0f) {
                        norm = locX >= locY ? right : down;  // LR corner
                    } else {
                        norm = locX > -locY ? right : up;  // UR corner
                    }
                } else {
                    if (locY > 0.0f) {
                        norm = -locX > locY ? left : down;  // LL corner
                    } else {
                        norm = locX > locY ? up : left;  // UL corner
                    }
                }
            }

            norm_to_rgb(bm, x, y, norm);
        }
    }
}

void create_tetra_normal_map(SkBitmap* bm, const SkIRect& dst) {
    const SkPoint center =
            SkPoint::Make(dst.fLeft + (dst.width() / 2.0f), dst.fTop + (dst.height() / 2.0f));

    static const SkScalar k1OverRoot3 = 0.5773502692f;

    SkPoint3       norm;
    const SkPoint3 leftUp  = SkPoint3::Make(-k1OverRoot3, -k1OverRoot3, k1OverRoot3);
    const SkPoint3 rightUp = SkPoint3::Make(k1OverRoot3, -k1OverRoot3, k1OverRoot3);
    const SkPoint3 down    = SkPoint3::Make(0.0f, SK_ScalarRoot2Over2, SK_ScalarRoot2Over2);

    for (int y = dst.fTop; y < dst.fBottom; ++y) {
        for (int x = dst.fLeft; x < dst.fRight; ++x) {
            SkScalar locX = x + 0.5f - center.fX;
            SkScalar locY = y + 0.5f - center.fY;

            if (locX >= 0.0f) {
                if (locY > 0.0f) {
                    norm = locX >= locY ? rightUp : down;  // LR corner
                } else {
                    norm = rightUp;
                }
            } else {
                if (locY > 0.0f) {
                    norm = -locX > locY ? leftUp : down;  // LL corner
                } else {
                    norm = leftUp;
                }
            }

            norm_to_rgb(bm, x, y, norm);
        }
    }
}

bool copy_to(SkBitmap* dst, SkColorType dstColorType, const SkBitmap& src) {
    SkPixmap srcPM;
    if (!src.peekPixels(&srcPM)) {
        return false;
    }

    SkBitmap    tmpDst;
    SkImageInfo dstInfo = srcPM.info().makeColorType(dstColorType);
    if (!tmpDst.setInfo(dstInfo)) {
        return false;
    }

    if (!tmpDst.tryAllocPixels()) {
        return false;
    }

    SkPixmap dstPM;
    if (!tmpDst.peekPixels(&dstPM)) {
        return false;
    }

    if (!srcPM.readPixels(dstPM)) {
        return false;
    }

    dst->swap(tmpDst);
    return true;
}

void copy_to_g8(SkBitmap* dst, const SkBitmap& src) {
    SkASSERT(kBGRA_8888_SkColorType == src.colorType() ||
             kRGBA_8888_SkColorType == src.colorType());

    SkImageInfo grayInfo = src.info().makeColorType(kGray_8_SkColorType);
    dst->allocPixels(grayInfo);
    uint8_t*        dst8  = (uint8_t*)dst->getPixels();
    const uint32_t* src32 = (const uint32_t*)src.getPixels();

    const int  w      = src.width();
    const int  h      = src.height();
    const bool isBGRA = (kBGRA_8888_SkColorType == src.colorType());
    for (int y = 0; y < h; ++y) {
        if (isBGRA) {
            // BGRA
            for (int x = 0; x < w; ++x) {
                uint32_t s = src32[x];
                dst8[x]    = SkComputeLuminance((s >> 16) & 0xFF, (s >> 8) & 0xFF, s & 0xFF);
            }
        } else {
            // RGBA
            for (int x = 0; x < w; ++x) {
                uint32_t s = src32[x];
                dst8[x]    = SkComputeLuminance(s & 0xFF, (s >> 8) & 0xFF, (s >> 16) & 0xFF);
            }
        }
        src32 = (const uint32_t*)((const char*)src32 + src.rowBytes());
        dst8 += dst->rowBytes();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////

bool equal_pixels(const SkPixmap& a, const SkPixmap& b) {
    if (a.width() != b.width() || a.height() != b.height() || a.colorType() != b.colorType()) {
        return false;
    }

    for (int y = 0; y < a.height(); ++y) {
        const char* aptr = (const char*)a.addr(0, y);
        const char* bptr = (const char*)b.addr(0, y);
        if (0 != memcmp(aptr, bptr, a.width() * a.info().bytesPerPixel())) {
            return false;
        }
    }
    return true;
}

bool equal_pixels(const SkBitmap& bm0, const SkBitmap& bm1) {
    SkPixmap pm0, pm1;
    return bm0.peekPixels(&pm0) && bm1.peekPixels(&pm1) && equal_pixels(pm0, pm1);
}

bool equal_pixels(const SkImage* a, const SkImage* b) {
    // ensure that peekPixels will succeed
    auto imga = a->makeRasterImage();
    auto imgb = b->makeRasterImage();

    SkPixmap pm0, pm1;
    return imga->peekPixels(&pm0) && imgb->peekPixels(&pm1) && equal_pixels(pm0, pm1);
}

sk_sp<SkSurface> makeSurface(SkCanvas*             canvas,
                             const SkImageInfo&    info,
                             const SkSurfaceProps* props) {
    auto surf = canvas->makeSurface(info, props);
    if (!surf) {
        surf = SkSurfaces::Raster(info, props);
    }
    return surf;
}

VariationSliders::VariationSliders(SkTypeface* typeface,
                                   SkFontArguments::VariationPosition variationPosition) {
    if (!typeface) {
        return;
    }

    int numAxes = typeface->getVariationDesignParameters(nullptr, 0);
    if (numAxes < 0) {
        return;
    }

    std::unique_ptr<SkFontParameters::Variation::Axis[]> copiedAxes =
            std::make_unique<SkFontParameters::Variation::Axis[]>(numAxes);

    numAxes = typeface->getVariationDesignParameters(copiedAxes.get(), numAxes);
    if (numAxes < 0) {
        return;
    }

    auto argVariationPositionOrDefault = [&variationPosition](SkFourByteTag tag,
                                                              SkScalar defaultValue) -> SkScalar {
        for (int i = 0; i < variationPosition.coordinateCount; ++i) {
            if (variationPosition.coordinates[i].axis == tag) {
                return variationPosition.coordinates[i].value;
            }
        }
        return defaultValue;
    };

    fAxisSliders.resize(numAxes);
    fCoords = std::make_unique<SkFontArguments::VariationPosition::Coordinate[]>(numAxes);
    for (int i = 0; i < numAxes; ++i) {
        fAxisSliders[i].axis = copiedAxes[i];
        fAxisSliders[i].current =
                argVariationPositionOrDefault(copiedAxes[i].tag, copiedAxes[i].def);
        fAxisSliders[i].name = tagToString(fAxisSliders[i].axis.tag);
        fCoords[i] = { fAxisSliders[i].axis.tag, fAxisSliders[i].current };
    }
}

/* static */
SkString VariationSliders::tagToString(SkFourByteTag tag) {
    char tagAsString[5];
    tagAsString[4] = 0;
    tagAsString[0] = (char)(uint8_t)(tag >> 24);
    tagAsString[1] = (char)(uint8_t)(tag >> 16);
    tagAsString[2] = (char)(uint8_t)(tag >> 8);
    tagAsString[3] = (char)(uint8_t)(tag >> 0);
    return SkString(tagAsString);
}

bool VariationSliders::writeControls(SkMetaData* controls) {
    for (size_t i = 0; i < fAxisSliders.size(); ++i) {
        SkScalar axisVars[kAxisVarsSize];

        axisVars[0] = fAxisSliders[i].current;
        axisVars[1] = fAxisSliders[i].axis.min;
        axisVars[2] = fAxisSliders[i].axis.max;
        controls->setScalars(fAxisSliders[i].name.c_str(), kAxisVarsSize, axisVars);
    }
    return true;
}

void VariationSliders::readControls(const SkMetaData& controls, bool* changed) {
    for (size_t i = 0; i < fAxisSliders.size(); ++i) {
        SkScalar axisVars[kAxisVarsSize] = {0};
        int resultAxisVarsSize = 0;
        SkASSERT_RELEASE(controls.findScalars(
                tagToString(fAxisSliders[i].axis.tag).c_str(), &resultAxisVarsSize, axisVars));
        SkASSERT_RELEASE(resultAxisVarsSize == kAxisVarsSize);
        if (changed) {
            *changed |= fAxisSliders[i].current != axisVars[0];
        }
        fAxisSliders[i].current = axisVars[0];
        fCoords[i] = { fAxisSliders[i].axis.tag, fAxisSliders[i].current };
    }
}

SkSpan<const SkFontArguments::VariationPosition::Coordinate> VariationSliders::getCoordinates() {
    return SkSpan<const SkFontArguments::VariationPosition::Coordinate>{fCoords.get(),
                                                                        fAxisSliders.size()};
}

////////////////////////////////////////////////////////////////////////////////////////////////////
HilbertGenerator::HilbertGenerator(float desiredSize, float desiredLineWidth, int desiredDepth)
        : fDesiredSize(desiredSize)
        , fDesiredDepth(desiredDepth)
        , fSegmentLength(fDesiredSize / ((0x1 << fDesiredDepth) - 1.0f))
        , fDesiredLineWidth(desiredLineWidth)
        , fActualBounds(SkRect::MakeEmpty())
        , fCurPos(SkPoint::Make(0.0f, 0.0f))
        , fCurDir(0)
        , fExpectedLen(fSegmentLength * ((0x1 << (2*fDesiredDepth)) - 1.0f))
        , fCurLen(0.0f) {
}

void HilbertGenerator::draw(SkCanvas* canvas) {
    this->recursiveDraw(canvas, /* curDepth= */ 0, /* turnLeft= */ true);

    SkScalarNearlyEqual(fExpectedLen, fCurLen, 0.01f);
    SkScalarNearlyEqual(fDesiredSize, fActualBounds.width(), 0.01f);
    SkScalarNearlyEqual(fDesiredSize, fActualBounds.height(), 0.01f);
}

void HilbertGenerator::turn90(bool turnLeft) {
    fCurDir += turnLeft ? 90 : -90;
    if (fCurDir >= 360) {
        fCurDir = 0;
    } else if (fCurDir < 0) {
        fCurDir = 270;
    }

    SkASSERT(fCurDir == 0 || fCurDir == 90 || fCurDir == 180 || fCurDir == 270);
}

void HilbertGenerator::line(SkCanvas* canvas) {

    SkPoint before = fCurPos;

    SkRect r;
    switch (fCurDir) {
        case 0:
            r.fLeft = fCurPos.fX;
            r.fTop = fCurPos.fY - fDesiredLineWidth / 2.0f;
            r.fRight = fCurPos.fX + fSegmentLength;
            r.fBottom = fCurPos.fY + fDesiredLineWidth / 2.0f;
            fCurPos.fX += fSegmentLength;
            break;
        case 90:
            r.fLeft = fCurPos.fX - fDesiredLineWidth / 2.0f;
            r.fTop = fCurPos.fY - fSegmentLength;
            r.fRight = fCurPos.fX + fDesiredLineWidth / 2.0f;
            r.fBottom = fCurPos.fY;
            fCurPos.fY -= fSegmentLength;
            break;
        case 180:
            r.fLeft = fCurPos.fX - fSegmentLength;
            r.fTop = fCurPos.fY - fDesiredLineWidth / 2.0f;
            r.fRight = fCurPos.fX;
            r.fBottom = fCurPos.fY + fDesiredLineWidth / 2.0f;
            fCurPos.fX -= fSegmentLength;
            break;
        case 270:
            r.fLeft = fCurPos.fX - fDesiredLineWidth / 2.0f;
            r.fTop = fCurPos.fY;
            r.fRight = fCurPos.fX + fDesiredLineWidth / 2.0f;
            r.fBottom = fCurPos.fY + fSegmentLength;
            fCurPos.fY += fSegmentLength;
            break;
        default:
            return;
    }

    SkPoint pts[2] = { before, fCurPos };

    SkColor4f colors[2] = {
            this->getColor(fCurLen),
            this->getColor(fCurLen + fSegmentLength),
    };

    fCurLen += fSegmentLength;
    if (fActualBounds.isEmpty()) {
        fActualBounds = r;
    } else {
        fActualBounds.join(r);
    }

    SkPaint paint;
    paint.setShader(SkGradientShader::MakeLinear(pts, colors, /* colorSpace= */ nullptr,
                                                 /* pos= */ nullptr, 2, SkTileMode::kClamp));
    canvas->drawRect(r, paint);
}

void HilbertGenerator::recursiveDraw(SkCanvas* canvas, int curDepth, bool turnLeft) {
    if (curDepth >= fDesiredDepth) {
        return;
    }

    this->turn90(turnLeft);
    this->recursiveDraw(canvas, curDepth + 1, !turnLeft);
    this->line(canvas);
    this->turn90(!turnLeft);
    this->recursiveDraw(canvas, curDepth + 1, turnLeft);
    this->line(canvas);
    this->recursiveDraw(canvas, curDepth + 1, turnLeft);
    this->turn90(!turnLeft);
    this->line(canvas);
    this->recursiveDraw(canvas, curDepth + 1, !turnLeft);
    this->turn90(turnLeft);
}

SkColor4f HilbertGenerator::getColor(float curLen) {
    static const SkColor4f kColors[] = {
            SkColors::kBlack,
            SkColors::kBlue,
            SkColors::kCyan,
            SkColors::kGreen,
            SkColors::kYellow,
            SkColors::kRed,
            SkColors::kWhite,
    };

    static const float kStops[] = {
            0.0f,
            1.0f/6.0f,
            2.0f/6.0f,
            0.5f,
            4.0f/6.0f,
            5.0f/6.0f,
            1.0f,
    };
    static_assert(std::size(kColors) == std::size(kStops));

    float t = curLen / fExpectedLen;
    if (t <= 0.0f) {
        return kColors[0];
    } else if (t >= 1.0f) {
        return kColors[std::size(kColors)-1];
    }

    for (unsigned int i = 0; i < std::size(kColors)-1; ++i) {
        if (kStops[i] <= t && t <= kStops[i+1]) {
            t = (t - kStops[i]) / (kStops[i+1] - kStops[i]);
            SkASSERT(0.0f <= t && t <= 1.0f);
            return { kColors[i].fR * (1 - t) + kColors[i+1].fR * t,
                     kColors[i].fG * (1 - t) + kColors[i+1].fG * t,
                     kColors[i].fB * (1 - t) + kColors[i+1].fB * t,
                     kColors[i].fA * (1 - t) + kColors[i+1].fA * t };

        }
    }

    return SkColors::kBlack;
}

void ExtractPathsFromSKP(const char filepath[], std::function<PathSniffCallback> callback) {
    SkFILEStream stream(filepath);
    if (!stream.isValid()) {
        SkDebugf("ExtractPaths: invalid input file at \"%s\"\n", filepath);
        return;
    }

    class PathSniffer : public SkCanvas {
    public:
        PathSniffer(std::function<PathSniffCallback> callback)
                : SkCanvas(4096, 4096, nullptr)
                , fPathSniffCallback(callback) {}
    private:
        void onDrawPath(const SkPath& path, const SkPaint& paint) override {
            fPathSniffCallback(this->getTotalMatrix(), path, paint);
        }
        std::function<PathSniffCallback> fPathSniffCallback;
    };

    sk_sp<SkPicture> skp = SkPicture::MakeFromStream(&stream);
    if (!skp) {
        SkDebugf("ExtractPaths: couldn't load skp at \"%s\"\n", filepath);
        return;
    }
    PathSniffer pathSniffer(callback);
    skp->playback(&pathSniffer);
}

}  // namespace ToolUtils
