/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/utils/SkLogHandler.h"

#include "include/core/SkRefCnt.h"
#include "include/private/SkMacros.h"
#include <atomic>

static std::atomic<SkLogHandler*> gUserLogHandler{nullptr};

bool SkLogHandler::SetInstance(sk_sp<SkLogHandler> handler) {
    SkLogHandler* h = handler.release();
    SkLogHandler* expected = nullptr;
    if (!gUserLogHandler.compare_exchange_strong(expected, h)) {
        SkSafeUnref(h);
        return false;
    }
    SK_INTENTIONALLY_LEAKED(h);
    return true;
}

sk_sp<SkLogHandler> SkLogHandler::GetInstance() {
    return sk_ref_sp(gUserLogHandler.load(std::memory_order_acquire));
}
