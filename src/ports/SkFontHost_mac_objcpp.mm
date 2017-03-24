/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOnce.h"
#include "SkTypes.h"

#include <CoreGraphics/CoreGraphics.h>
#include <dlfcn.h>

#ifdef SK_BUILD_FOR_MAC
#include <AppKit/AppKit.h>
static constexpr struct {
    CGFloat defaultValue;
    const char* name;
} nsFontWeightLoaderInfos[] = {
    { -0.80f, "NSFontWeightUltraLight" },
    { -0.60f, "NSFontWeightThin" },
    { -0.40f, "NSFontWeightLight" },
    {  0.00f, "NSFontWeightRegular" },
    {  0.23f, "NSFontWeightMedium" },
    {  0.30f, "NSFontWeightSemibold" },
    {  0.40f, "NSFontWeightBold" },
    {  0.56f, "NSFontWeightHeavy" },
    {  0.62f, "NSFontWeightBlack" },
};
#endif

#ifdef SK_BUILD_FOR_IOS
#include <UIKit/UIKit.h>
static constexpr struct {
    CGFloat defaultValue;
    const char* name;
} nsFontWeightLoaderInfos[] = {
    { -0.80f, "UIFontWeightUltraLight" },
    { -0.60f, "UIFontWeightThin" },
    { -0.40f, "UIFontWeightLight" },
    {  0.00f, "UIFontWeightRegular" },
    {  0.23f, "UIFontWeightMedium" },
    {  0.30f, "UIFontWeightSemibold" },
    {  0.40f, "UIFontWeightBold" },
    {  0.56f, "UIFontWeightHeavy" },
    {  0.62f, "UIFontWeightBlack" },
};
#endif

/** Returns the [-1, 1] CTFontDescriptor weights for the
 *  [0, 100, 200, 300, 400, 500, 600, 700, 800, 900, 1000] CSS weights.
 *
 *  It is assumed that the values will be interpolated linearly between these points.
 *  NSFontWeightXXX were added in 10.11, appear in 10.10, but do not appear in 10.9.
 *  The actual values appear to be stable, but they may change in the future without notice.
 */
CGFloat(&SkGetNSFontWeightMapping())[11] {
    static_assert(SK_ARRAY_COUNT(nsFontWeightLoaderInfos) == 9, "");
    static CGFloat nsFontWeights[11];
    static SkOnce once;
    once([&] {
        size_t i = 0;
        nsFontWeights[i++] = -1.00;
        for (const auto& nsFontWeightLoaderInfo : nsFontWeightLoaderInfos) {
            void* nsFontWeightValuePtr = dlsym(RTLD_DEFAULT, nsFontWeightLoaderInfo.name);
            if (nsFontWeightValuePtr) {
                nsFontWeights[i++] = *(static_cast<CGFloat*>(nsFontWeightValuePtr));
            } else {
                nsFontWeights[i++] = nsFontWeightLoaderInfo.defaultValue;
            }
        }
        nsFontWeights[i++] = 1.00;
    });
    return nsFontWeights;
}
