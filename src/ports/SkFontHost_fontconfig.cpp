/*
 * Copyright 2008 Google Inc.
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

SkFontConfigInterface* SkFontConfigInterface::RefGlobal() {
    SkAutoMutexAcquire ac(gFontConfigInterfaceMutex);

    return SkSafeRef(gFontConfigInterface);
}

SkFontConfigInterface* SkFontConfigInterface::SetGlobal(SkFontConfigInterface* fc) {
    SkAutoMutexAcquire ac(gFontConfigInterfaceMutex);

    SkRefCnt_SafeAssign(gFontConfigInterface, fc);
    return fc;
}

///////////////////////////////////////////////////////////////////////////////

static SkFontConfigInterface* init_FCI() {
    SkAutoMutexAcquire ac(gFontConfigInterfaceMutex);

    if (gFontConfigInterface) {
        return SkRef(gFontConfigInterface);
    }
    gFontConfigInterface = SkRef(SkFontConfigInterface::GetSingletonDirectInterface());
    return gFontConfigInterface;
}

SkFontMgr* SkFontMgr::Factory() {
    SkFontConfigInterface* fci = init_FCI();
    return fci ? SkFontMgr_New_FCI(fci) : nullptr;
}
