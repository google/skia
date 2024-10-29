/*
* Copyright 2024 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SKFONTSCANNER_H_
#define SKFONTSCANNER_H_

#include "include/core/SkFontArguments.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"
class SkFontStyle;
class SkStreamAsset;
class SkString;

class SkFontScanner : public SkNoncopyable {
public:
    virtual ~SkFontScanner() = default;
    struct AxisDefinition {
        SkFourByteTag fTag;
        SkScalar fMinimum;
        SkScalar fDefault;
        SkScalar fMaximum;
    };
    typedef skia_private::STArray<4, AxisDefinition, true> AxisDefinitions;

    virtual bool scanFile(SkStreamAsset* stream, int* numFaces) const = 0;
    virtual bool scanFace(SkStreamAsset* stream, int faceIndex, int* numInstances) const = 0;
    /* instanceIndex 0 is the default instance, 1 to numInstances are the named instances. */
    virtual bool scanInstance(SkStreamAsset* stream,
                              int faceIndex,
                              int instanceIndex,
                              SkString* name,
                              SkFontStyle* style,
                              bool* isFixedPitch,
                              AxisDefinitions* axes) const = 0;
};

#endif // SKFONTSCANNER_H_
