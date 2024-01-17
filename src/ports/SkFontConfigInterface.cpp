/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/ports/SkFontConfigInterface.h"

#include "include/core/SkFontMgr.h"
#include "include/core/SkRefCnt.h"
#include "include/private/base/SkMutex.h"

static SkMutex& font_config_interface_mutex() {
    static SkMutex& mutex = *(new SkMutex);
    return mutex;
}
static SkFontConfigInterface* gFontConfigInterface;

sk_sp<SkFontConfigInterface> SkFontConfigInterface::RefGlobal() {
    SkAutoMutexExclusive ac(font_config_interface_mutex());

    if (gFontConfigInterface) {
        return sk_ref_sp(gFontConfigInterface);
    }
    return sk_ref_sp(SkFontConfigInterface::GetSingletonDirectInterface());
}

void SkFontConfigInterface::SetGlobal(sk_sp<SkFontConfigInterface> fc) {
    SkAutoMutexExclusive ac(font_config_interface_mutex());

    SkSafeUnref(gFontConfigInterface);
    gFontConfigInterface = fc.release();
}

sk_sp<SkTypeface> SkFontConfigInterface::makeTypeface(const FontIdentity& identity,
                                                      sk_sp<SkFontMgr> mgr) {
    return mgr->makeFromStream(std::unique_ptr<SkStreamAsset>(this->openStream(identity)),
                               identity.fTTCIndex);
}
