/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFontMgr_DEFINED
#define SkFontMgr_DEFINED

#include "SkRefCnt.h"

class SkData;
class SkStream;
class SkString;

class SkFontStyle {
public:
    enum Weight {
        kThin_Weight        = 100,
        kExtraLight_Weight  = 200,
        kLight_Weight       = 300,
        kNormal_Weight      = 400,
        kMedium_Weight      = 500,
        kSemiBold_Weight    = 600,
        kBold_Weight        = 700,
        kExtraBold_Weight   = 800,
        kBlack_Weight       = 900
    };
    
    enum Width {
        kUltraCondensed_Width   = 1,
        kExtraCondensed_Width   = 2,
        kCondensed_Width        = 3,
        kSemiCondensed_Width    = 4,
        kNormal_Width           = 5,
        kSemiExpanded_Width     = 6,
        kExpanded_Width         = 7,
        kExtraExpanded_Width    = 8,
        kUltaExpanded_Width     = 9
    };

    enum Flags {
        kItalic_Flag    = 1 << 0,
    };
    
    SkFontStyle();
    SkFontStyle(int weight, int width, unsigned flags);

    bool operator==(const SkFontStyle&) const {
        return fUnion.fU32 == other.fUnion.fU32;
    }

    int weight() const { return fUnion.fR.fWeight; }
    int width() const { return fUnion.fR.fWidth; }
    unsigned flags() const { return fUnion.fR.fFlags; }

    bool isItalic() const {
        return SkToBool(fUnion.fR.fFlags & kItalic_Flag);
    }

private:
    union {
        struct {
            uint16_t fWeight;   // 100 ... 900
            uint8_t  fWidth;    // 1 .. 9
            uint8_t  fFlags;
        } fR;
        uint32_t    fU32;
    } fUnion;
};

class SkFontMgr : public SkRefCnt {
public:
    /**
     *  SkData contains an array of [const char*]
     */
    SkData* refFamilyNames();
    
    /**
     *  Given a familyName, if a corresponding family is found, return
     *  the array of available styles in SkData (as [SkFontStyle]).
     *
     *  If foundFamilyName is not null, set it to the actual familyName for the
     *  found family.
     */
    SkData* refFamilyStyles(const char familyName[], SkString* foundFamilyName);

    /**
     *  Find the closest matching typeface to the specified familyName and style
     *  and return a ref to it. The caller must call unref() on the returned
     *  object. Will never return NULL.
     *
     *  If foundFamilyName is not null, set it to the actual familyName for the
     *  returned typeface.
     */
    SkTypeface* matchFamilyName(const char familyName[], const Desc&,
                                SkString* foundFamilyName);

    /**
     *  Create a typeface for the specified data and TTC index (pass 0 for none)
     *  or NULL if the data is not recognized. The caller must call unref() on
     *  the returned object if it is not null.
     */
    SkTypeface* createFromData(SkData*, int ttcIndex = 0);
    
    /**
     *  Create a typeface for the specified stream and TTC index
     *  (pass 0 for none) or NULL if the stream is not recognized. The caller
     *  must call unref() on the returned object if it is not null.
     */
    SkTypeface* createFromStream(SkStream*, int ttcIndex = 0);
    
    /**
     *  Create a typeface for the specified fileName and TTC index
     *  (pass 0 for none) or NULL if the file is not found, or its contents are
     *  not recognized. The caller must call unref() on the returned object
     *  if it is not null.
     */
    SkTypeface* createFromFile(const char path[], int ttcIndex = 0);
    
private:
    typedef SkRefCnt INHERITED;
};

#endif
