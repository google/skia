/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextAdapter_DEFINED
#define SkottieTextAdapter_DEFINED

#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/text/TextValue.h"

namespace sksg {
class Color;
class Draw;
class Group;
class TextBlob;
} // namespace sksg

namespace skottie {

class TextAdapter final : public SkNVRefCnt<TextAdapter> {
public:
    explicit TextAdapter(sk_sp<sksg::Group> root);
    ~TextAdapter();

    ADAPTER_PROPERTY(Text, TextValue, TextValue())

    const sk_sp<sksg::Group>& root() const { return fRoot; }

private:
    void apply();

    sk_sp<sksg::Group>     fRoot;
    sk_sp<sksg::TextBlob>  fTextNode;
    sk_sp<sksg::Color>     fFillColor,
                           fStrokeColor;
    sk_sp<sksg::Draw>      fFillNode,
                           fStrokeNode;

    bool                   fHadFill   : 1, //  - state cached from the prev apply()
                           fHadStroke : 1; //  /
};

} // namespace skottie

#endif // SkottieTextAdapter_DEFINED
