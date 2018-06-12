/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkClusterator.h"

#include "SkTo.h"
#include "SkUtils.h"

static bool is_reversed(const uint32_t* clusters, uint32_t count) {
    // "ReversedChars" is how PDF deals with RTL text.
    // return true if more than one cluster and monotonicly decreasing to zero.
    if (count < 2 || clusters[0] == 0 || clusters[count - 1] != 0) {
        return false;
    }
    for (uint32_t i = 0; i + 1 < count; ++i) {
        if (clusters[i + 1] > clusters[i]) {
            return false;
        }
    }
    return true;
}

SkClusterator::SkClusterator(const void* sourceText,
                             size_t sourceByteCount,
                             const SkPaint& paint,
                             const uint32_t* clusters,
                             uint32_t utf8TextByteLength,
                             const char* utf8Text) {
    if (SkPaint::kGlyphID_TextEncoding == paint.getTextEncoding()) {
        fGlyphs = reinterpret_cast<const SkGlyphID*>(sourceText);
        fClusters = clusters;
        fUtf8Text = utf8Text;
        fGlyphCount = sourceByteCount / sizeof(SkGlyphID);
        fTextByteLength = utf8TextByteLength;
        if (fClusters) {
            SkASSERT(fUtf8Text && fTextByteLength > 0 && fGlyphCount > 0);
            fReversedChars = is_reversed(fClusters, fGlyphCount);
        } else {
            SkASSERT(!fUtf8Text && fTextByteLength == 0);
        }
        return;
    }

    // If Skia is given text (not glyphs), then our fallback primitive shaping will
    // produce a simple 1-1 cluster mapping.
    fGlyphCount = SkToU32(paint.textToGlyphs(sourceText, sourceByteCount, nullptr));
    fGlyphStorage.resize(fGlyphCount);
    (void)paint.textToGlyphs(sourceText, sourceByteCount, fGlyphStorage.data());
    fGlyphs = fGlyphStorage.data();
    fClusterStorage.resize(fGlyphCount);
    fClusters = fClusterStorage.data();

    switch (paint.getTextEncoding()) {
        case SkPaint::kUTF8_TextEncoding:
        {
            fUtf8Text = reinterpret_cast<const char*>(sourceText);
            fTextByteLength = SkToU32(sourceByteCount);
            const char* txtPtr = fUtf8Text;
            for (uint32_t i = 0; i < fGlyphCount; ++i) {
                fClusterStorage[i] = SkToU32(txtPtr - fUtf8Text);
                txtPtr += SkUTF8_LeadByteToCount(*(const unsigned char*)txtPtr);
                SkASSERT(txtPtr <= fUtf8Text + sourceByteCount);
            }
            SkASSERT(txtPtr == fUtf8Text + sourceByteCount);
            return;
        }
        case SkPaint::kUTF16_TextEncoding:
        {
            const uint16_t* utf16ptr = reinterpret_cast<const uint16_t*>(sourceText);
            int utf16count = SkToInt(sourceByteCount / sizeof(uint16_t));
            fTextByteLength = SkToU32(SkUTF16_ToUTF8(utf16ptr, utf16count));
            fUtf8textStorage.resize(fTextByteLength);
            fUtf8Text = fUtf8textStorage.data();
            char* txtPtr = fUtf8textStorage.data();
            uint32_t clusterIndex = 0;
            while (utf16ptr < (const uint16_t*)sourceText + utf16count) {
                fClusterStorage[clusterIndex++] = SkToU32(txtPtr - fUtf8Text);
                SkUnichar uni = SkUTF16_NextUnichar(&utf16ptr);
                txtPtr += SkUTF8_FromUnichar(uni, txtPtr);
            }
            SkASSERT(clusterIndex == fGlyphCount);
            SkASSERT(txtPtr == fUtf8textStorage.data() + fTextByteLength);
            SkASSERT(utf16ptr == (const uint16_t*)sourceText + utf16count);
            return;
        }
        case SkPaint::kUTF32_TextEncoding:
        {
            const SkUnichar* utf32 = reinterpret_cast<const SkUnichar*>(sourceText);
            uint32_t utf32count = SkToU32(sourceByteCount / sizeof(SkUnichar));
            SkASSERT(fGlyphCount == utf32count);
            fTextByteLength = 0;
            for (uint32_t i = 0; i < utf32count; ++i) {
                fTextByteLength += SkToU32(SkUTF8_FromUnichar(utf32[i]));
            }
            fUtf8textStorage.resize(SkToSizeT(fTextByteLength));
            fUtf8Text = fUtf8textStorage.data();
            char* txtPtr = fUtf8textStorage.data();
            for (uint32_t i = 0; i < utf32count; ++i) {
                fClusterStorage[i] = SkToU32(txtPtr - fUtf8Text);
                txtPtr += SkUTF8_FromUnichar(utf32[i], txtPtr);
            }
            return;
        }
        default:
            SkDEBUGFAIL("");
            break;
    }
}

SkClusterator::Cluster SkClusterator::next() {
    if (fCurrentGlyphIndex >= fGlyphCount) {
        return Cluster{nullptr, 0, 0, 0};
    }
    if (!fClusters || !fUtf8Text) {
        return Cluster{nullptr, 0, fCurrentGlyphIndex++, 1};
    }
    uint32_t clusterGlyphIndex = fCurrentGlyphIndex;
    uint32_t cluster = fClusters[clusterGlyphIndex];
    do {
        ++fCurrentGlyphIndex;
    } while (fCurrentGlyphIndex < fGlyphCount && cluster == fClusters[fCurrentGlyphIndex]);
    uint32_t clusterGlyphCount = fCurrentGlyphIndex - clusterGlyphIndex;
    uint32_t clusterEnd = fTextByteLength;
    for (unsigned i = 0; i < fGlyphCount; ++i) {
       uint32_t c = fClusters[i];
       if (c > cluster && c < clusterEnd) {
           clusterEnd = c;
       }
    }
    uint32_t clusterLen = clusterEnd - cluster;
    return Cluster{fUtf8Text + cluster, clusterLen, clusterGlyphIndex, clusterGlyphCount};
}

