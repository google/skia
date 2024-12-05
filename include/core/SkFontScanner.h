/*
* Copyright 2024 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SKFONTSCANNER_H_
#define SKFONTSCANNER_H_

#include "include/core/SkFontArguments.h"
#include "include/core/SkFontParameters.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"
class SkFontStyle;
class SkStreamAsset;
class SkString;
class SkTypeface;

class SkFontScanner : public SkNoncopyable {
public:
    virtual ~SkFontScanner() = default;
    using AxisDefinitions = skia_private::STArray<4, SkFontParameters::Variation::Axis, true>;
    using VariationPosition = skia_private::STArray<4, SkFontArguments::VariationPosition::Coordinate, true>;

    virtual bool scanFile(SkStreamAsset* stream, int* numFaces) const = 0;
    virtual bool scanFace(SkStreamAsset* stream, int faceIndex, int* numInstances) const = 0;
    /* instanceIndex 0 is the default instance, 1 to numInstances are the named instances. */
    virtual bool scanInstance(SkStreamAsset* stream,
                              int faceIndex,
                              int instanceIndex,
                              SkString* name,
                              SkFontStyle* style,
                              bool* isFixedPitch,
                              AxisDefinitions* axes,
                              VariationPosition* position) const = 0;
    virtual sk_sp<SkTypeface> MakeFromStream(std::unique_ptr<SkStreamAsset> stream,
                                             const SkFontArguments& args) const = 0;
    virtual SkFourByteTag getFactoryId() const = 0;
};

#endif // SKFONTSCANNER_H_
