/*
 * Copyright 2009-2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* migrated from chrome/src/skia/ext/SkFontHost_fontconfig_direct.cpp */

#include "SkFontConfigInterface_direct.h"
#include "SkStream.h"
#include "SkTypes.h"

// Loads fonts using GoogleFt2ReadFontFromMemory.
class SkFontConfigInterfaceDirectGoogle3 : public SkFontConfigInterfaceDirect {
public:
    SkFontConfigInterfaceDirectGoogle3() {}
    ~SkFontConfigInterfaceDirectGoogle3() override {}

    SkStreamAsset* openStream(const FontIdentity&) override;
protected:
    // Override isAccessible to return true if the font is in the cache.
    bool isAccessible(const char* filename) override;
private:
    typedef SkFontConfigInterfaceDirect INHERITED;
};
