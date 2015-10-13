//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/Cache.h"
#include "compiler/translator/InitializeDll.h"
#include "compiler/translator/InitializeGlobals.h"
#include "compiler/translator/InitializeParseContext.h"

#include "common/platform.h"

#include <assert.h>

bool InitProcess()
{
    if (!InitializePoolIndex()) {
        assert(0 && "InitProcess(): Failed to initalize global pool");
        return false;
    }

    if (!InitializeParseContextIndex()) {
        assert(0 && "InitProcess(): Failed to initalize parse context");
        return false;
    }

    TCache::initialize();

    return true;
}

void DetachProcess()
{
    FreeParseContextIndex();
    FreePoolIndex();
    TCache::destroy();
}
