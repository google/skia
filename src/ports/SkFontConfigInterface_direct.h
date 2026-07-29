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
    /** Create around a FontConfig instance.
     *  If 'fc' is nullptr, each method call will use the current config.
     *  Takes ownership of 'fc' and will call FcConfigDestroy on it.
     */
    explicit SkFontConfigInterfaceDirect(FcConfig* fc);
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
    FcConfig * const fFC;
    bool isValidPattern(FcPattern* pattern);

    /**
     * Determines whether the match returned by FcFontMatch is acceptable, meaning
     * * It checks whether isValidPattern(match) is true
     * * Whether the match is an acceptable substitute if IsFallbackFontAllowed(family) is false
     *
     * @param match the FcPattern* returned by FcFontMatch
     * @param post_config_family the post config family to match
     * @param family the font family to match
     */
    bool isAcceptableMatch(FcPattern* match,
                           const char* post_config_family,
                           const SkString& family);
    FcPattern* MatchFont(FcFontSet* font_set,
                         const char* post_config_family,
                         const SkString& family);
    using INHERITED = SkFontConfigInterface;
};

#endif
