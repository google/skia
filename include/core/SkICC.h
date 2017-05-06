/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkICC_DEFINED
#define SkICC_DEFINED

#include "SkData.h"
#include "SkRefCnt.h"

struct SkColorSpaceTransferFn;
class SkColorSpace;
class SkData;
class SkMatrix44;

class SK_API SkICC : public SkRefCnt {
public:

    /**
     *  Parse an ICC profile.
     *
     *  Returns nullptr if the data is not a valid ICC profile or if the profile
     *  input space is not RGB.
     */
    static sk_sp<SkICC> Make(const void*, size_t);

    /**
     *  If the gamut can be represented as transformation into XYZ D50, returns
     *  true and sets the proper values in |toXYZD50|.
     *
     *  If not, returns false.  This indicates that the ICC data is too complex
     *  to isolate a simple gamut transformation.
     */
    bool toXYZD50(SkMatrix44* toXYZD50) const;

    /**
     *  If the transfer function can be represented as coefficients to the standard
     *  equation, returns true and sets |fn| to the proper values.
     *
     *  If not, returns false.  This indicates one of the following:
     *  (1) The R, G, and B transfer functions are not the same.
     *  (2) The transfer function is represented as a table that we have not managed
     *      to match to a standard curve.
     *  (3) The ICC data is too complex to isolate a single transfer function.
     */
    bool isNumericalTransferFn(SkColorSpaceTransferFn* fn) const;

    /**
     *  Please do not call this unless isNumericalTransferFn() has been called and it
     *  fails.  SkColorSpaceTransferFn is the preferred representation.
     *
     *  If it is not possible to represent the R, G, and B transfer functions numerically
     *  and it is still necessary to get the transfer function, this will return the
     *  transfer functions as three tables (R, G, and B).
     *
     *  If possible, this will return tables of the same length as they were specified in
     *  the ICC profile.  This means that the lengths of the three tables are not
     *  guaranteed to be the same.  If the ICC representation was not a table, the length
     *  will be chosen arbitrarily.
     *
     *  The lengths of the tables are all guaranteed to be at least 2.  Entries in the
     *  tables are guaranteed to be in [0, 1].
     *
     *  This API may be deleted in favor of a numerical approximation of the raw data.
     *
     *  This function may fail, indicating that the ICC profile does not have transfer
     *  functions.
     */
    struct Channel {
        // Byte offset of the start of the table in |fStorage|
        size_t fOffset;
        int    fCount;
    };
    struct Tables {
        Channel fRed;
        Channel fGreen;
        Channel fBlue;

        const float* red() {
            return (const float*) (fStorage->bytes() + fRed.fOffset);
        }
        const float* green() {
            return (const float*) (fStorage->bytes() + fGreen.fOffset);
        }
        const float* blue() {
            return (const float*) (fStorage->bytes() + fBlue.fOffset);
        }

        sk_sp<SkData> fStorage;
    };
    bool rawTransferFnData(Tables* tables) const;

    /**
     *  Write an ICC profile with transfer function |fn| and gamut |toXYZD50|.
     */
    static sk_sp<SkData> WriteToICC(const SkColorSpaceTransferFn& fn, const SkMatrix44& toXYZD50);

private:
    SkICC(sk_sp<SkColorSpace> colorSpace);

    sk_sp<SkColorSpace> fColorSpace;

    friend class ICCTest;
};

#endif
