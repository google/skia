/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <hb-ot.h>
#include <unicode/stringpiece.h>
#include <unicode/ubidi.h>
#include <unicode/unistr.h>

#include "SkFontMgr.h"
#include "SkShaper.h"
#include "SkStream.h"
#include "SkTemplates.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"
#include "SkUtils.h"

static const int FONT_SIZE_SCALE = 512;

namespace {
template <class T, void(*P)(T*)> using resource = std::unique_ptr<T, SkFunctionWrapper<void, T, P>>;
using HBBlob   = resource<hb_blob_t  , hb_blob_destroy  >;
using HBFace   = resource<hb_face_t  , hb_face_destroy  >;
using HBFont   = resource<hb_font_t  , hb_font_destroy  >;
using HBBuffer = resource<hb_buffer_t, hb_buffer_destroy>;
using ICUBiDi  = resource<UBiDi      , ubidi_close      >;

HBBlob stream_to_blob(std::unique_ptr<SkStreamAsset> asset) {
    size_t size = asset->getLength();
    HBBlob blob;
    if (const void* base = asset->getMemoryBase()) {
        blob.reset(hb_blob_create((char*)base, SkToUInt(size),
                                  HB_MEMORY_MODE_READONLY, asset.release(),
                                  [](void* p) { delete (SkStreamAsset*)p; }));
    } else {
        // SkDebugf("Extra SkStreamAsset copy\n");
        void* ptr = size ? sk_malloc_throw(size) : nullptr;
        asset->read(ptr, size);
        blob.reset(hb_blob_create((char*)ptr, SkToUInt(size),
                                  HB_MEMORY_MODE_READONLY, ptr, sk_free));
    }
    SkASSERT(blob);
    hb_blob_make_immutable(blob.get());
    return blob;
}
}  // namespace

struct SkShaper::Impl {
    HBFont fHarfBuzzFont;
    HBBuffer fBuffer;
    sk_sp<SkTypeface> fTypeface;
};

static HBFont create_hb_font(SkTypeface* tf) {
    int index;
    HBBlob blob(stream_to_blob(std::unique_ptr<SkStreamAsset>(tf->openStream(&index))));
    HBFace face(hb_face_create(blob.get(), (unsigned)index));
    SkASSERT(face);
    if (!face) {
        return nullptr;
    }
    hb_face_set_index(face.get(), (unsigned)index);
    hb_face_set_upem(face.get(), tf->getUnitsPerEm());

    HBFont font(hb_font_create(face.get()));
    SkASSERT(font);
    if (!font) {
        return nullptr;
    }
    hb_font_set_scale(font.get(), FONT_SIZE_SCALE, FONT_SIZE_SCALE);
    hb_ot_font_set_funcs(font.get());
    int axis_count = tf->getVariationDesignPosition(nullptr, 0);
    if (axis_count > 0) {
        SkAutoSTMalloc<4, SkFontArguments::VariationPosition::Coordinate> axis_values(axis_count);
        if (tf->getVariationDesignPosition(axis_values, axis_count) == axis_count) {
            hb_font_set_variations(font.get(),
                                   reinterpret_cast<hb_variation_t*>(axis_values.get()),
                                   axis_count);
        }
    }
    return font;
}

SkShaper::SkShaper(sk_sp<SkTypeface> tf) : fImpl(new Impl) {
    fImpl->fTypeface = tf ? std::move(tf) : SkTypeface::MakeDefault();
    fImpl->fHarfBuzzFont = create_hb_font(fImpl->fTypeface.get());
    SkASSERT(fImpl->fHarfBuzzFont);
    fImpl->fBuffer.reset(hb_buffer_create());
}

SkShaper::~SkShaper() {}

bool SkShaper::good() const { return fImpl->fHarfBuzzFont != nullptr; }

SkScalar SkShaper::shape(SkTextBlobBuilder* builder,
                         const SkPaint& srcPaint,
                         const char* utf8text,
                         size_t textBytes,
                         bool leftToRight,
                         SkPoint point) const {
    sk_sp<SkFontMgr> fontMgr = SkFontMgr::RefDefault();
    SkASSERT(builder);
    UBiDiLevel bidiLevel = leftToRight ? UBIDI_DEFAULT_LTR : UBIDI_DEFAULT_RTL;
    //hb_script_t script = ...
    UErrorCode status = U_ZERO_ERROR;
    double x = point.x();
    double y = point.y();

    // This function only accepts utf8.
    // ubidi only accepts utf16 (though internally it basically works on utf32 chars).
    // Internally, harfbuzz is all utf32, but always makes a copy.

    if (!SkTFitsIn<int32_t>(textBytes)) {
        SkDebugf("Bidi error: text too long");
        return (SkScalar)x;
    }
    icu::UnicodeString utf16 = icu::UnicodeString::fromUTF8(icu::StringPiece(utf8text, textBytes));

    ICUBiDi bidi(ubidi_openSized(utf16.length(), 0, &status));
    if (U_FAILURE(status)) {
        SkDebugf("Bidi error: %s", u_errorName(status));
        return (SkScalar)x;
    }
    SkASSERT(bidi);

    ubidi_setPara(bidi.get(), utf16.getBuffer(), utf16.length(), bidiLevel, nullptr, &status);
    if (U_FAILURE(status)) {
        SkDebugf("Bidi error: %s", u_errorName(status));
        return (SkScalar)x;
    }

    int32_t runCount = ubidi_countRuns(bidi.get(), &status);
    if (U_FAILURE(status)) {
        SkDebugf("Bidi error: %s", u_errorName(status));
        return (SkScalar)x;
    }

    const UChar* utf16End = utf16.getBuffer();
    const char* utf8End = utf8text;
    for (int32_t i = 0; i < runCount; ++i) {
        int32_t start;
        int32_t length;
        UBiDiDirection direction = ubidi_getVisualRun(bidi.get(), i, &start, &length);

        SkPaint paint(srcPaint);
        paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
        paint.setTypeface(fImpl->fTypeface);
        hb_font_t* hbfont = fImpl->fHarfBuzzFont.get();
        HBFont fallbackHBFont;
        sk_sp<SkTypeface> fallback;
        while (utf16End < utf16.getBuffer() + start + length) {
            hb_buffer_t* buffer = fImpl->fBuffer.get();
            SkAutoTCallVProc<hb_buffer_t, hb_buffer_clear_contents> autoClearBuffer(buffer);

            // The difficulty here is the cluster mapping.
            // If the hb_buffer is created with utf16, clusters will point to utf16 indexes,
            // but the SkTextBlob can only take utf8 and utf8 cluster indexes.
            // So populate the hb_buffer directly with utf32 and utf8 cluster indexes.
            // Since this steps through the visual runs in order, it is expected that each run will
            // start just after the previous one ended.
            const UChar* utf16Start = utf16.getBuffer() + start;
            const char* utf8Start;
            if (utf16End == utf16Start) {
                utf16Start = utf16End;
                utf8Start = utf8End;
            } else {
                SkDEBUGFAIL("Did not expect to ever get here.");
                utf16Start = utf16.getBuffer();
                utf8Start = utf8text;
                while (utf16Start < utf16.getBuffer() + start) {
                    SkUTF16_NextUnichar(&utf16Start);
                    SkUTF8_NextUnichar(&utf8Start);
                }
            }
            const char* utf8Current = utf8Start;
            const UChar* utf16Current = utf16Start;
            utf16End = utf16Current + length;
            while (utf16Current < utf16End) {
                const UChar* utf16Prev = utf16Current;
                hb_codepoint_t u = SkUTF16_NextUnichar(&utf16Current);
                bool doPartialRun = false;

                // If using a fallback and the initial typeface has this character, stop fallback.
                if (fallbackHBFont &&
                    fImpl->fTypeface->charsToGlyphs(&u, SkTypeface::kUTF32_Encoding, nullptr, 1))
                {
                    fallback.reset();
                    doPartialRun = true;

                // If the current typeface does not have this character, try a fallback.
                } else if (!paint.getTypeface()->charsToGlyphs(&u, SkTypeface::kUTF32_Encoding,
                                                               nullptr, 1))
                {
                    fallback.reset(fontMgr->matchFamilyStyleCharacter(nullptr,
                                                                      fImpl->fTypeface->fontStyle(),
                                                                      nullptr, 0,
                                                                      u));
                    if (fallback) {
                        doPartialRun = true;
                    }
                }
                if (doPartialRun) {
                    utf16End = utf16Prev;
                    int32_t oldStart = start;
                    start = utf16End - utf16.getBuffer();
                    length -= start - oldStart;
                    break;
                }
                hb_buffer_add(buffer, u, utf8Current - utf8Start);
                SkUTF8_NextUnichar(&utf8Current);
            }
            hb_buffer_set_content_type(buffer, HB_BUFFER_CONTENT_TYPE_UNICODE);
            utf8End = utf8Current;
            size_t utf8runLength = utf8End - utf8Start;
            if (!SkTFitsIn<int>(utf8runLength)) {
                SkDebugf("Shaping error: utf8 too long");
                return (SkScalar)x;
            }
            hb_buffer_guess_segment_properties(buffer);
            //hb_buffer_set_script(buffer, script);
            hb_buffer_set_direction(buffer, direction ? HB_DIRECTION_RTL : HB_DIRECTION_LTR);
            hb_shape(hbfont, buffer, nullptr, 0);
            unsigned len = hb_buffer_get_length(buffer);
            if (len > 0) {
                hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer, nullptr);
                hb_glyph_position_t* pos = hb_buffer_get_glyph_positions(buffer, nullptr);

                if (!SkTFitsIn<int>(len)) {
                    SkDebugf("Shaping error: too many glyphs");
                    return (SkScalar)x;
                }
                auto runBuffer = builder->allocRunTextPos(paint, len, utf8runLength, SkString());
                memcpy(runBuffer.utf8text, utf8Start, utf8runLength);

                double textSizeY = paint.getTextSize() / (double)FONT_SIZE_SCALE;
                double textSizeX = textSizeY * paint.getTextScaleX();

                for (unsigned i = 0; i < len; i++) {
                    runBuffer.glyphs[i] = info[i].codepoint;
                    runBuffer.clusters[i] = info[i].cluster;
                    reinterpret_cast<SkPoint*>(runBuffer.pos)[i] =
                            SkPoint::Make(SkDoubleToScalar(x + pos[i].x_offset * textSizeX),
                                          SkDoubleToScalar(y - pos[i].y_offset * textSizeY));
                    x += pos[i].x_advance * textSizeX;
                    y += pos[i].y_advance * textSizeY;
                }
            }

            if (fallback) {
                paint.setTypeface(std::move(fallback));
                fallbackHBFont = create_hb_font(paint.getTypeface());
                hbfont = fallbackHBFont.get();
            } else {
                paint.setTypeface(fImpl->fTypeface);
                fallbackHBFont = nullptr;
                hbfont = fImpl->fHarfBuzzFont.get();
            }
        }
    }
    return (SkScalar)x;
}
