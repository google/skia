
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkAutoCo_DEFINED
#define SkAutoCo_DEFINED

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "SkTemplates.h"

/**
 * An instance of this class initializes COM on creation
 * and closes the COM library on destruction.
 */
class SkAutoCoInitialize : SkNoncopyable {
private:
    HRESULT fHR;
public:
    SkAutoCoInitialize();
    ~SkAutoCoInitialize();
    bool succeeded();
};

#endif
