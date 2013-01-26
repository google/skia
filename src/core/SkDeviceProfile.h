/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDeviceProfile_DEFINED
#define SkDeviceProfile_DEFINED

#include "SkRefCnt.h"

class SkDeviceProfile : public SkRefCnt {
public:
    SK_DECLARE_INST_COUNT(SkDeviceProfile)

    enum LCDConfig {
        kNone_LCDConfig,   // disables LCD text rendering, uses A8 instead
        kRGB_Horizontal_LCDConfig,
        kBGR_Horizontal_LCDConfig,
        kRGB_Vertical_LCDConfig,
        kBGR_Vertical_LCDConfig
    };

    enum FontHintLevel {
        kNone_FontHintLevel,
        kSlight_FontHintLevel,
        kNormal_FontHintLevel,
        kFull_FontHintLevel,
        kAuto_FontHintLevel
    };

    /**
     *  gammaExp is typically between 1.0 and 2.2. For no gamma adjustment,
     *  specify 1.0
     *
     *  contrastScale will be pinned between 0.0 and 1.0. For no contrast
     *  adjustment, specify 0.0
     *
     *  @param config   Describes the LCD layout for this device. If this is set
     *                  to kNone, then all requests for LCD text will be
     *                  devolved to A8 antialiasing.
     *
     *  @param level    The hinting level to be used, IF the paint specifies
     *                  "default". Otherwise the paint's hinting level will be
     *                  respected.
     */
    static SkDeviceProfile* Create(float gammaExp,
                                   float contrastScale,
                                   LCDConfig,
                                   FontHintLevel);

    /**
     *  Returns the global default profile, that is used if no global profile is
     *  specified with SetGlobal(), or if NULL is specified to SetGlobal().
     *  The references count is *not* incremented, and the caller should not
     *  call unref().
     */
    static SkDeviceProfile* GetDefault();

    /**
     *  Return the current global profile (or the default if no global had yet
     *  been set) and increment its reference count. The call *must* call unref()
     *  when it is done using it.
     */
    static SkDeviceProfile* RefGlobal();

    /**
     *  Make the specified profile be the global value for all subsequently
     *  instantiated devices. Does not affect any existing devices.
     *  Increments the reference count on the profile.
     *  Specify NULL for the "identity" profile (where there is no gamma or
     *  contrast correction).
     */
    static void SetGlobal(SkDeviceProfile*);

    float getFontGammaExponent() const { return fGammaExponent; }
    float getFontContrastScale() const { return fContrastScale; }

    /**
     *  Given a luminance byte (0 for black, 0xFF for white), generate a table
     *  that applies the gamma/contrast settings to linear coverage values.
     */
    void generateTableForLuminanceByte(U8CPU lumByte, uint8_t table[256]) const;

private:
    SkDeviceProfile(float gammaExp, float contrastScale, LCDConfig,
                    FontHintLevel);

    float           fGammaExponent;
    float           fContrastScale;
    LCDConfig       fLCDConfig;
    FontHintLevel   fFontHintLevel;

    typedef SkRefCnt INHERITED;
};

#endif
