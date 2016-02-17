/*
 * Copyright 2009-2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* migrated from chrome/src/skia/ext/SkFontHost_fontconfig_direct.cpp */

#include "SkFontConfigInterface.h"
#include "SkMutex.h"

#include <fontconfig/fontconfig.h>

class SkFontConfigInterfaceDirect : public SkFontConfigInterface {
public:
    SkFontConfigInterfaceDirect();
    ~SkFontConfigInterfaceDirect() override;

    bool matchFamilyName(const char familyName[],
                         SkTypeface::Style requested,
                         FontIdentity* outFontIdentifier,
                         SkString* outFamilyName,
                         SkTypeface::Style* outStyle) override;
    SkStreamAsset* openStream(const FontIdentity&) override;

    // new APIs
    SkDataTable* getFamilyNames() override;

protected:
    virtual bool isAccessible(const char* filename);

private:
    SkMutex mutex_;

    bool isValidPattern(FcPattern* pattern);
    FcPattern* MatchFont(FcFontSet* font_set, const char* post_config_family,
                         const SkString& family);
    typedef SkFontConfigInterface INHERITED;
};
