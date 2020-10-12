/*
* Copyright 2020 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include <memory>
#include "modules/skshaper/src/SkUnicode.h"

std::unique_ptr<SkUnicode> SkUnicode::Make() {
    std::unique_ptr<SkUnicode> unicode = SkUnicode::MakeIcuUnicode();
    if (unicode) {
        // That is the only one implemented currently
        return unicode;
    }
    return nullptr;
}
