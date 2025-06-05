/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/encode/SkPngEncoderBase.h"

#include <utility>

#include "include/core/SkAlphaType.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkSpan.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkMSAN.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/encode/SkImageEncoderFns.h"

namespace {

SkEncodedInfo makeInfo(const SkImageInfo& srcInfo,
                       SkEncodedInfo::Color color,
                       int bitsPerComponent) {
    SkEncodedInfo::Alpha alpha =
            color == SkEncodedInfo::kGray_Color || color == SkEncodedInfo::kRGB_Color
                    ? SkEncodedInfo::kOpaque_Alpha
                    : SkEncodedInfo::kUnpremul_Alpha;

    return SkEncodedInfo::Make(srcInfo.width(), srcInfo.height(), color, alpha, bitsPerComponent);
}

SkEncodedInfo makeGray8Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kGray_Color, 8);
}

SkEncodedInfo makeGrayAlpha8Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kGrayAlpha_Color, 8);
}

SkEncodedInfo makeRgba8Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kRGBA_Color, 8);
}

SkEncodedInfo makeRgba16Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kRGBA_Color, 16);
}

SkPngEncoderBase::TargetInfo makeTargetInfo(SkEncodedInfo dstInfo, const SkImageInfo& srcImageInfo,
                                            SkColorType dstCT, SkAlphaType dstAT) {
    SkASSERT(dstCT != kAlpha_8_SkColorType);
    SkImageInfo dstRowInfo = SkImageInfo::Make(srcImageInfo.width(), 1, dstCT, dstAT);
    return SkPngEncoderBase::TargetInfo {srcImageInfo.makeWH(srcImageInfo.width(), 1),
                                         dstRowInfo,
                                         std::move(dstInfo),
                                         dstRowInfo.minRowBytes(),};
}

std::optional<SkPngEncoderBase::TargetInfo> makeAlpha8TargetInfo(SkEncodedInfo dstInfo) {
    // `static_cast<size_t>`(dstInfo.bitsPerPixel())` uses trustworthy, bounded
    // data as input - no need to use `SkSafeMath` for this part.
    SkASSERT(dstInfo.bitsPerComponent() == 8 || dstInfo.bitsPerComponent() == 16);
    SkASSERT(dstInfo.bitsPerPixel() <= (16 * 4));
    size_t bitsPerPixel = static_cast<size_t>(dstInfo.bitsPerPixel());
    SkASSERT((bitsPerPixel % 8) == 0);
    size_t bytesPerPixel = bitsPerPixel / 8;

    SkSafeMath safe;
    size_t dstRowSize = safe.mul(safe.castTo<size_t>(dstInfo.width()), bytesPerPixel);
    if (!safe.ok()) {
        return std::nullopt;
    }
    return SkPngEncoderBase::TargetInfo{std::nullopt, std::nullopt,
                                        std::move(dstInfo), dstRowSize};
}

}  // namespace

// static
std::optional<SkPngEncoderBase::TargetInfo> SkPngEncoderBase::getTargetInfo(
        const SkImageInfo& srcInfo) {

SkColorType srcCT = srcInfo.colorType();
SkAlphaType srcAT = srcInfo.alphaType();
int numChannels = SkColorTypeNumChannels(srcCT);

switch(numChannels) {
  case 1: {
      uint32_t srcChannelFlags = SkColorTypeChannelFlags(srcCT);
      if (srcChannelFlags == kGray_SkColorChannelFlag) {
          SkASSERT(srcInfo.isOpaque());
          return makeTargetInfo(makeGray8Info(srcInfo), srcInfo, srcCT, srcAT);
      }
      // We support encoding kAlpha_8_SkColorType to GrayAlpha images and just ignore gray.
      // Otherwise, there is no sensible way to encode alpha only images.
      if (srcCT == kAlpha_8_SkColorType) {
          return makeAlpha8TargetInfo(makeGrayAlpha8Info(srcInfo));
      }
      break;
  }
  case 3: {
      SkASSERT(srcInfo.isOpaque());
      if (srcAT == kUnknown_SkAlphaType) {
          SkDEBUGFAIL("unknown alpha type");
          return std::nullopt;
      }
      int maxBitsPerChannel = SkColorTypeMaxBitsPerChannel(srcCT);
      if (maxBitsPerChannel <= 8) {
          return makeTargetInfo(makeRgba8Info(srcInfo),
                                srcInfo,
                                kRGB_888x_SkColorType,
                                kOpaque_SkAlphaType);
      } else if (maxBitsPerChannel <= 32) {
          return makeTargetInfo(makeRgba16Info(srcInfo),
                                srcInfo,
                                kR16G16B16A16_unorm_SkColorType,
                                kOpaque_SkAlphaType);
      }
      break;
  }
  case 4: {
      if (srcAT == kUnknown_SkAlphaType) {
          SkDEBUGFAIL("unknown alpha type");
          return std::nullopt;
      }
      int maxBitsPerChannel = SkColorTypeMaxBitsPerChannel(srcCT);
      if (maxBitsPerChannel <= 8) {
          if (srcAT == kOpaque_SkAlphaType) {
              return makeTargetInfo(makeRgba8Info(srcInfo),
                                    srcInfo,
                                    kRGB_888x_SkColorType,
                                    kOpaque_SkAlphaType);
          }
          return makeTargetInfo(makeRgba8Info(srcInfo),
                                srcInfo,
                                kRGBA_8888_SkColorType,
                                kUnpremul_SkAlphaType);
      } else if (maxBitsPerChannel <= 32) {
          if (srcAT == kOpaque_SkAlphaType) {
              return makeTargetInfo(makeRgba16Info(srcInfo),
                                    srcInfo,
                                    kR16G16B16A16_unorm_SkColorType,
                                    kOpaque_SkAlphaType);
          }
          return makeTargetInfo(makeRgba16Info(srcInfo),
                                srcInfo,
                                kR16G16B16A16_unorm_SkColorType,
                                kUnpremul_SkAlphaType);
      }
  }
  break;
}
return std::nullopt;
}

SkPngEncoderBase::SkPngEncoderBase(TargetInfo targetInfo, const SkPixmap& src)
        : SkEncoder(src, targetInfo.fDstRowSize), fTargetInfo(std::move(targetInfo)) {
    SkASSERT(src.colorType() == kAlpha_8_SkColorType
             || (fTargetInfo.fSrcRowInfo && fTargetInfo.fDstRowInfo));
}

bool SkPngEncoderBase::onEncodeRows(int numRows) {
    // https://www.w3.org/TR/png-3/#11IHDR says that "zero is an invalid value"
    // for width and height.
    if (fSrc.width() == 0 || fSrc.height() == 0) {
        return false;
    }

    if (numRows < 0) {
        return false;
    }

    while (numRows > 0) {
        if (fCurrRow == fSrc.height()) {
            return false;
        }

        const void* srcRow = fSrc.addr(0, fCurrRow);
        sk_msan_assert_initialized(srcRow,
                                   (const uint8_t*)srcRow + (fSrc.width() << fSrc.shiftPerPixel()));

        if (fSrc.colorType() == kAlpha_8_SkColorType) {
          // This is a special case where we store kAlpha_8 images as GrayAlpha in png.
          transform_scanline_A8_to_GrayAlpha((char*)fStorage.get(),
                                            (const char*)srcRow,
                                            fSrc.width(),
                                            SkColorTypeBytesPerPixel(fSrc.colorType()));
        } else {
          SkASSERT(fSrc.width() == fTargetInfo.fSrcRowInfo->width());
          if (!SkConvertPixels(fTargetInfo.fDstRowInfo.value(),
                              (void*)fStorage.get(),
                              fTargetInfo.fDstRowSize,
                              fTargetInfo.fSrcRowInfo.value(),
                              srcRow,
                              fTargetInfo.fSrcRowInfo->minRowBytes()))
          {
              return false;
          }
          // We need to convert from little endian to big endian so we use skcms.
          if (fTargetInfo.fDstRowInfo.value().colorType() == kR16G16B16A16_unorm_SkColorType) {
              if (!skcms_Transform((char*)fStorage.get(), skcms_PixelFormat_RGBA_16161616LE,
                                  skcms_AlphaFormat_Unpremul, nullptr, fStorage.get(),
                                  skcms_PixelFormat_RGBA_16161616BE, skcms_AlphaFormat_Unpremul,
                                  nullptr, fSrc.width())) {
                  return false;
              }
          }
        }

        SkSpan<const uint8_t> rowToEncode(fStorage.get(), fTargetInfo.fDstRowSize);
        if (!this->onEncodeRow(rowToEncode)) {
            return false;
        }

        fCurrRow++;
        numRows--;
    }

    if (fCurrRow == fSrc.height() && !fFinishedEncoding) {
        fFinishedEncoding = true;
        return this->onFinishEncoding();
    }

    return true;
}
