/*
* Copyright 2024 The Android Open Source Project
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SKFONTSCANNER_FONTATIONS_PRIV_H_
#define SKFONTSCANNER_FONTATIONS_PRIV_H_

#include "include/core/SkFontScanner.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"

struct SkAdvancedTypefaceMetrics;
class SkFontDescriptor;
class SkFontData;

class SkFontScanner_Fontations : public SkFontScanner {
public:
    SkFontScanner_Fontations();
    ~SkFontScanner_Fontations() override;

    bool scanFile(SkStreamAsset* stream, int* numFaces) const override;
    bool scanFace(SkStreamAsset* stream, int faceIndex, int* numInstances) const override;
    bool scanInstance(SkStreamAsset* stream,
                      int faceIndex,
                      int instanceIndex,
                      SkString* name,
                      SkFontStyle* style,
                      bool* isFixedPitch,
                      AxisDefinitions* axes) const override;
    sk_sp<SkTypeface> MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                     const SkFontArguments& args) const override;
    SkFourByteTag getFactoryId() const override;
private:
};

#endif // SKFONTSCANNER_FONTATIONS_PRIV_H_
