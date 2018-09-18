/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkFontConfigInterface.h"
#include "SkFontMgr.h"
#include "SkMutex.h"
#include "SkRefCnt.h"

SK_DECLARE_STATIC_MUTEX(gFontConfigInterfaceMutex);
static SkFontConfigInterface* gFontConfigInterface;

sk_sp<SkFontConfigInterface> SkFontConfigInterface::RefGlobal() {
    SkAutoMutexAcquire ac(gFontConfigInterfaceMutex);

    if (gFontConfigInterface) {
        return sk_ref_sp(gFontConfigInterface);
    }
    return sk_ref_sp(SkFontConfigInterface::GetSingletonDirectInterface());
}

void SkFontConfigInterface::SetGlobal(sk_sp<SkFontConfigInterface> fc) {
    SkAutoMutexAcquire ac(gFontConfigInterfaceMutex);

    SkSafeUnref(gFontConfigInterface);
    gFontConfigInterface = fc.release();
}
