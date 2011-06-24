/*
 * Copyright (C) 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkAdvancedTypefaceMetrics.h"
#include "SkTypes.h"

#ifdef SK_BUILD_FOR_UNIX
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#ifdef SK_BUILD_FOR_MAC
#import <ApplicationServices/ApplicationServices.h>
#endif

#ifdef SK_BUILD_FOR_IOS
#include <CoreText/CoreText.h>
#include <CoreGraphics/CoreGraphics.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace skia_advanced_typeface_metrics_utils {

template <typename Data>
void resetRange(SkAdvancedTypefaceMetrics::AdvanceMetric<Data>* range,
                int startId) {
    range->fStartId = startId;
    range->fAdvance.setCount(0);
}

template <typename Data>
SkAdvancedTypefaceMetrics::AdvanceMetric<Data>* appendRange(
        SkTScopedPtr<SkAdvancedTypefaceMetrics::AdvanceMetric<Data> >* nextSlot,
        int startId) {
    nextSlot->reset(new SkAdvancedTypefaceMetrics::AdvanceMetric<Data>);
    resetRange(nextSlot->get(), startId);
    return nextSlot->get();
}

template <typename Data>
void finishRange(
        SkAdvancedTypefaceMetrics::AdvanceMetric<Data>* range,
        int endId,
        typename SkAdvancedTypefaceMetrics::AdvanceMetric<Data>::MetricType
                type) {
    range->fEndId = endId;
    range->fType = type;
    int newLength;
    if (type == SkAdvancedTypefaceMetrics::AdvanceMetric<Data>::kRange) {
        newLength = endId - range->fStartId + 1;
    } else {
        newLength = 1;
    }
    SkASSERT(range->fAdvance.count() >= newLength);
    range->fAdvance.setCount(newLength);
}

template <typename Data, typename FontHandle>
SkAdvancedTypefaceMetrics::AdvanceMetric<Data>* getAdvanceData(
        FontHandle fontHandle,
        int num_glyphs,
        bool (*getAdvance)(FontHandle fontHandle, int gId, Data* data)) {
    // Assuming that an ASCII representation of a width or a glyph id is,
    // on average, 3 characters long gives the following cut offs for
    // using different range types:
    // When currently in a range
    //  - Removing 4 0's is a win
    //  - Removing 5 repeats is a win
    // When not currently in a range
    //  - Removing 1 0 is a win
    //  - Removing 3 repeats is a win

    SkTScopedPtr<SkAdvancedTypefaceMetrics::AdvanceMetric<Data> > result;
    SkAdvancedTypefaceMetrics::AdvanceMetric<Data>* curRange;
    SkAdvancedTypefaceMetrics::AdvanceMetric<Data>* prevRange = NULL;
    curRange = appendRange(&result, 0);
    Data lastAdvance = SK_MinS16;
    int repeats = 0;
    for (int gId = 0; gId <= num_glyphs; gId++) {
        Data advance;
        if (gId < num_glyphs) {
            SkAssertResult(getAdvance(fontHandle, gId, &advance));
        } else {
            advance = SK_MinS16;
        }
        if (advance == lastAdvance) {
            repeats++;
        } else if (curRange->fAdvance.count() == repeats + 1) {
            if (lastAdvance == 0 && repeats >= 0) {
                resetRange(curRange, gId);
            } else if (repeats >= 2) {
                finishRange(curRange, gId - 1,
                            SkAdvancedTypefaceMetrics::WidthRange::kRun);
                prevRange = curRange;
                curRange = appendRange(&curRange->fNext, gId);
            }
            repeats = 0;
        } else {
            if (lastAdvance == 0 && repeats >= 3) {
                finishRange(curRange, gId - repeats - 2,
                            SkAdvancedTypefaceMetrics::WidthRange::kRange);
                prevRange = curRange;
                curRange = appendRange(&curRange->fNext, gId);
            } else if (repeats >= 4) {
                finishRange(curRange, gId - repeats - 2,
                            SkAdvancedTypefaceMetrics::WidthRange::kRange);
                curRange = appendRange(&curRange->fNext, gId - repeats - 1);
                curRange->fAdvance.append(1, &lastAdvance);
                finishRange(curRange, gId - 1,
                            SkAdvancedTypefaceMetrics::WidthRange::kRun);
                prevRange = curRange;
                curRange = appendRange(&curRange->fNext, gId);
            }
            repeats = 0;
        }
        curRange->fAdvance.append(1, &advance);
        lastAdvance = advance;
    }
    if (curRange->fStartId == num_glyphs) {
        SkASSERT(prevRange);
        SkASSERT(prevRange->fNext->fStartId == num_glyphs);
        prevRange->fNext.reset();
    } else {
        finishRange(curRange, num_glyphs - 1,
                    SkAdvancedTypefaceMetrics::WidthRange::kRange);
    }
    return result.release();
}

// Make AdvanceMetric template functions available for linking with typename
// WidthRange and VerticalAdvanceRange.
#if defined(SK_BUILD_FOR_WIN)
template SkAdvancedTypefaceMetrics::WidthRange* getAdvanceData(
        HDC hdc,
        int num_glyphs,
        bool (*getAdvance)(HDC hdc, int gId, int16_t* data));
#elif defined(SK_BUILD_FOR_UNIX)
template SkAdvancedTypefaceMetrics::WidthRange* getAdvanceData(
        FT_Face face,
        int num_glyphs,
        bool (*getAdvance)(FT_Face face, int gId, int16_t* data));
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_IOS)
template SkAdvancedTypefaceMetrics::WidthRange* getAdvanceData(
        CTFontRef ctFont,
        int num_glyphs,
        bool (*getAdvance)(CTFontRef ctFont, int gId, int16_t* data));
#endif
template void resetRange(
        SkAdvancedTypefaceMetrics::WidthRange* range,
        int startId);
template SkAdvancedTypefaceMetrics::WidthRange* appendRange(
        SkTScopedPtr<SkAdvancedTypefaceMetrics::WidthRange >* nextSlot,
        int startId);
template void finishRange<int16_t>(
        SkAdvancedTypefaceMetrics::WidthRange* range,
        int endId,
        SkAdvancedTypefaceMetrics::WidthRange::MetricType type);

template void resetRange(
        SkAdvancedTypefaceMetrics::VerticalAdvanceRange* range,
        int startId);
template SkAdvancedTypefaceMetrics::VerticalAdvanceRange* appendRange(
        SkTScopedPtr<SkAdvancedTypefaceMetrics::VerticalAdvanceRange >*
            nextSlot,
        int startId);
template void finishRange<SkAdvancedTypefaceMetrics::VerticalMetric>(
        SkAdvancedTypefaceMetrics::VerticalAdvanceRange* range,
        int endId,
        SkAdvancedTypefaceMetrics::VerticalAdvanceRange::MetricType type);

} // namespace skia_advanced_typeface_metrics_utils
