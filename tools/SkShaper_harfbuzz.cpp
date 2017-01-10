/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <hb-ot.h>

#include "SkShaper.h"
#include "SkStream.h"
#include "SkTextBlob.h"
#include "SkTypeface.h"

static const int FONT_SIZE_SCALE = 512;

namespace {
struct HBFBlobDel {
    void operator()(hb_blob_t* b) { hb_blob_destroy(b); }
};

std::unique_ptr<hb_blob_t, HBFBlobDel> stream_to_blob(std::unique_ptr<SkStreamAsset> asset) {
    size_t size = asset->getLength();
    std::unique_ptr<hb_blob_t, HBFBlobDel> blob;
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
    struct HBFontDel {
        void operator()(hb_font_t* f) { hb_font_destroy(f); }
    };
    std::unique_ptr<hb_font_t, HBFontDel> fHarfBuzzFont;
    struct HBBufDel {
        void operator()(hb_buffer_t* b) { hb_buffer_destroy(b); }
    };
    std::unique_ptr<hb_buffer_t, HBBufDel> fBuffer;
    sk_sp<SkTypeface> fTypeface;
};

SkShaper::SkShaper(sk_sp<SkTypeface> tf) : fImpl(new Impl) {
    fImpl->fTypeface = tf ? std::move(tf) : SkTypeface::MakeDefault();
    int index;
    std::unique_ptr<hb_blob_t, HBFBlobDel> blob(
            stream_to_blob(std::unique_ptr<SkStreamAsset>(
                                   fImpl->fTypeface->openStream(&index))));
    struct HBFaceDel {
        void operator()(hb_face_t* f) { hb_face_destroy(f); }
    };
    std::unique_ptr<hb_face_t, HBFaceDel> face(
            hb_face_create(blob.get(), (unsigned)index));
    SkASSERT(face);
    if (!face) {
        return;
    }
    hb_face_set_index(face.get(), (unsigned)index);
    hb_face_set_upem(face.get(), fImpl->fTypeface->getUnitsPerEm());

    fImpl->fHarfBuzzFont.reset(hb_font_create(face.get()));
    SkASSERT(fImpl->fHarfBuzzFont);
    hb_font_set_scale(fImpl->fHarfBuzzFont.get(), FONT_SIZE_SCALE, FONT_SIZE_SCALE);
    hb_ot_font_set_funcs(fImpl->fHarfBuzzFont.get());

    fImpl->fBuffer.reset(hb_buffer_create());
}

SkShaper::~SkShaper() {}

bool SkShaper::good() const { return fImpl->fHarfBuzzFont != nullptr; }

SkScalar SkShaper::shape(SkTextBlobBuilder* builder,
                         const SkPaint& srcPaint,
                         const char* utf8text,
                         size_t textBytes,
                         SkPoint point) const {
    SkPaint paint(srcPaint);
    paint.setTextEncoding(SkPaint::kGlyphID_TextEncoding);
    paint.setTypeface(fImpl->fTypeface);

    SkASSERT(builder);
    hb_buffer_t* buffer = fImpl->fBuffer.get();
    hb_buffer_add_utf8(buffer, utf8text, -1, 0, -1);
    hb_buffer_guess_segment_properties(buffer);
    hb_shape(fImpl->fHarfBuzzFont.get(), buffer, nullptr, 0);
    unsigned len = hb_buffer_get_length(buffer);
    if (len == 0) {
        hb_buffer_clear_contents(buffer);
        return 0;
    }

    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer, NULL);
    hb_glyph_position_t* pos =
            hb_buffer_get_glyph_positions(buffer, NULL);
    auto runBuffer = builder->allocRunTextPos(
            paint, SkToInt(len), SkToInt(textBytes), SkString());
    memcpy(runBuffer.utf8text, utf8text, textBytes);

    double x = point.x();
    double y = point.y();

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

    hb_buffer_clear_contents(buffer);
    return (SkScalar)x;
}
