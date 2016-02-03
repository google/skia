/*
 * Copyright 2009-2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* migrated from chrome/src/skia/ext/SkFontHost_fontconfig_direct.cpp */

#include "SkFontConfigInterface_direct.h"
#include "SkMutex.h"

SkFontConfigInterface* SkFontConfigInterface::GetSingletonDirectInterface(SkBaseMutex* mutex) {
    SkAutoMutexAcquire ac(mutex);
    static SkFontConfigInterfaceDirect* singleton = nullptr;
    if (singleton == nullptr) {
        singleton = new SkFontConfigInterfaceDirect;
    }
    return singleton;
}
