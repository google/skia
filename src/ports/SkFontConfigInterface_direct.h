/*
 * Copyright 2009-2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* migrated from chrome/src/skia/ext/SkFontHost_fontconfig_direct.cpp */
#ifndef SKFONTCONFIGINTERFACE_DIRECT_H_
#define SKFONTCONFIGINTERFACE_DIRECT_H_

#include "include/ports/SkFontConfigInterface.h"

#include <fontconfig/fontconfig.h>

class SkFontConfigInterfaceDirect : public SkFontConfigInterface {
public:
    SkFontConfigInterfaceDirect();
    ~SkFontConfigInterfaceDirect() override;

    bool matchFamilyName(const char familyName[],
                         SkFontStyle requested,
                         FontIdentity* outFontIdentifier,
                         SkString* outFamilyName,
                         SkFontStyle* outStyle) override;

    SkStreamAsset* openStream(const FontIdentity&) override;

protected:
    virtual bool isAccessible(const char* filename);

private:
    std::unique_ptr<FcConfig,SkFunctionWrapper<decltype(FcConfigDestroy),FcConfigDestroy>> fcConfig;
    bool isValidPattern(FcPattern* pattern);
    FcPattern* MatchFont(FcFontSet* font_set, const char* post_config_family,
                         const SkString& family);
    typedef SkFontConfigInterface INHERITED;
};

#endif
