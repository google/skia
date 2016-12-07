/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkICC_DEFINED
#define SkICC_DEFINED

#include "SkTypes.h"

struct SkColorSpaceTransferFn;
class SkData;
class SkMatrix;

class SK_API SkICC : public SkNoncopyable {
public:

    /**
     *  Parse an ICC profile.
     */
    static std::unique_ptr<SkICC> Make(const void*, size_t);

    /**
     *  If the gamut can be represented as transformation into XYZ D50, returns
     *  true and sets the proper values in |matrix|.
     *
     *  If not, returns false.  This indicates that the ICC data is too complex
     *  to isolate a simple gamut transformation.
     */
    bool toXYZD50(SkMatrix* toXYZD50) const;

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
     *  If the transfer function can be approximated as coefficients to the standard
     *  equation, returns true and sets |fn| to the proper values.
     *
     *  If not, returns false.  This indicates one of the following:
     *  (1) The R, G, and B transfer functions are not the same.
     *  (2) The transfer function is represented as a table that is not increasing with
     *      end points at zero and one.
     *  (3) The ICC data is too complex to isolate a single transfer function.
     */
    bool approximateNumericalTransferFn(SkColorSpaceTransferFn* fn) const;

    /**
     *  Write an ICC profile with transfer function |fn| and gamut |toXYZD50|.
     */
    static sk_sp<SkData> WriteToICC(const SkColorSpaceTransferFn& fn, const SkMatrix& toXYZD50);

private:
    SkICC() {}
};

#endif
