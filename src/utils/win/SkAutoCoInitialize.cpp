
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#if defined(SK_BUILD_FOR_WIN32)


#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <ole2.h>
#include "SkAutoCoInitialize.h"

SkAutoCoInitialize::SkAutoCoInitialize() :
    fHR(
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)
    )
{ }

SkAutoCoInitialize::~SkAutoCoInitialize() {
    if (SUCCEEDED(this->fHR)) {
        CoUninitialize();
    }
}

bool SkAutoCoInitialize::succeeded() {
    return SUCCEEDED(this->fHR) || RPC_E_CHANGED_MODE == this->fHR;
}

#endif//defined(SK_BUILD_FOR_WIN32)
