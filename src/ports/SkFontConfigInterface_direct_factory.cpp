/*
 * Copyright 2009-2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/ports/SkFontConfigInterface_direct.h"

SkFontConfigInterface* SkFontConfigInterface::GetSingletonDirectInterface() {
    static SkFontConfigInterface* singleton = new SkFontConfigInterfaceDirect(nullptr);
    return singleton;
}
