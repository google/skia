
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ole2.h>
#include "SkAutoCoInitialize.h"

AutoCoInitialize::AutoCoInitialize() :
    fHR(
        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE)
    )
{ }

AutoCoInitialize::~AutoCoInitialize() {
    if (SUCCEEDED(this->fHR)) {
        CoUninitialize();
    }
}

HRESULT AutoCoInitialize::getHR() { return this->fHR; }
