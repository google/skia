/*
 * Copyright 2009-2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface_direct_google3.h"
#include "SkOnce.h"

SkFontConfigInterface* SkFontConfigInterface::GetSingletonDirectInterface() {
    static SkFontConfigInterface* singleton;
    static SkOnce once;
    once([]{ singleton = new SkFontConfigInterfaceDirectGoogle3(); });
    return singleton;
}
