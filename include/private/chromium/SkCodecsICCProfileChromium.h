/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkCodecsICCProfileChromium_DEFINED
#define SkCodecsICCProfileChromium_DEFINED

#include <memory>

#include "include/core/SkData.h"
#include "include/core/SkRefCnt.h"
#include "include/private/SkAPI.h"
#include "modules/skcms/skcms.h"

namespace SkCodecs {

// Allows Chromium code that does not use SkCodec to parse ICC profiles
// using the same parser that SkCodec uses (skcms or Rust).
class SK_API ICCProfileChromium {
public:
    virtual ~ICCProfileChromium() = default;

    // Returns nullptr if parsing fails. May retain `data`. Uses the
    // build-default parser.
    static std::unique_ptr<ICCProfileChromium> Make(sk_sp<SkData> data);

    // Return the parsed profile. The pointers in the structure are guaranteed
    // to be valid until `this` is destroyed.
    virtual const skcms_ICCProfile& GetProfile() const = 0;

protected:
    ICCProfileChromium() = default;

private:
    ICCProfileChromium(const ICCProfileChromium&) = delete;
    ICCProfileChromium& operator=(const ICCProfileChromium&) = delete;
};

}  // namespace SkCodecs

#endif  // SkCodecsICCProfileChromium_DEFINED
