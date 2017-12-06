/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkColorSpace_New_DEFINED
#define SkColorSpace_New_DEFINED

#include "SkColorSpace.h"

class SkRasterPipeline;

// TODO: for this to make sense we essentially have to fold a lot of
// SkColorSpace_Base into SkColorSpace.  SkColorSpace doesn't have any virtual methods...
class SkColorSpace_New : public SkColorSpace {
public:

    class ICCProfile;   // TODO: == SkICC?

    struct TransferFn {
        virtual ~TransferFn() = default;

        // May return false even when this is equivalent to TranferFn,
        // but must always be equivalent when this returns true.  (No false positives.)
        virtual bool equals(const TransferFn&) const = 0;

        // Append stages to use this transfer function with SkRasterPipeline-based rendering.
        virtual void    encodeSrc(SkRasterPipeline*) const = 0;
        virtual void linearizeSrc(SkRasterPipeline*) const = 0;
        virtual void linearizeDst(SkRasterPipeline*) const = 0;

        // TODO: Ganesh hooks.

        // Must return a pointer unique to each subclass, to allow safe downcasts in equals().
        virtual void* typeID() const = 0;

        // TODO: ???
        virtual void updateICCProfile(ICCProfile*) const = 0;
    };

    enum class Blending { Linear, AsEncoded };  // TODO: move to SkColorSpace?

    SkColorSpace_New(SkMatrix44 toXYZD50, std::unique_ptr<TransferFn>, Blending);

    const SkMatrix44&   toXYZD50() const { return fToXYZD50;    }
    const SkMatrix44& fromXYZD50() const { return fFromXYZD50;  }
    const TransferFn& transferFn() const { return *fTransferFn; }
    Blending            blending() const { return fBlending;    }

    // TODO: here is where I would put my SkColorSpace overrides, _if it had any virtual methods_.

private:
    SkMatrix44                  fToXYZD50,
                                fFromXYZD50;
    std::unique_ptr<TransferFn> fTransferFn;
    Blending                    fBlending;
};

#endif//SkColorSpace_New_DEFINED
