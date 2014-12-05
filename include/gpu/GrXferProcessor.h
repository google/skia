/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrXferProcessor_DEFINED
#define GrXferProcessor_DEFINED

#include "GrColor.h"
#include "GrFragmentProcessor.h"
#include "GrTypes.h"
#include "SkXfermode.h"

/**
 * GrXferProcessor is responsible for implementing the xfer mode that blends the src color and dst
 * color. It does this by emitting fragment shader code and controlling the fixed-function blend
 * state. The inputs to its shader code are the final computed src color and fractional pixel
 * coverage. The GrXferProcessor's shader code writes the fragment shader output color that goes
 * into the fixed-function blend. When dual-source blending is available, it may also write a
 * seconday fragment shader output color. When allowed by the backend API, the GrXferProcessor may
 * read the destination color. The GrXferProcessor is responsible for setting the blend coefficients
 * and blend constant color.
 *
 * A GrXferProcessor is never installed directly into our draw state, but instead is created from a
 * GrXPFactory once we have finalized the state of our draw.
 */
class GrXferProcessor : public GrFragmentProcessor {
private:

    typedef GrFragmentProcessor INHERITED;
};

/**
 * We install a GrXPFactory (XPF) early on in the pipeline before all the final draw information is
 * known (e.g. whether there is fractional pixel coverage, will coverage be 1 or 4 channel, is the
 * draw opaque, etc.). Once the state of the draw is finalized, we use the XPF along with all the
 * draw information to create a GrXferProcessor (XP) which can implement the desired blending for
 * the draw.
 *
 * Before the XP is created, the XPF is able to answer queries about what functionality the XPs it
 * creates will have. For example, can it create an XP that supports RGB coverage or will the XP
 * blend with the destination color.
 */
class GrXPFactory : public SkRefCnt {
public:
    virtual const GrXferProcessor* createXferProcessor() const = 0;

    /**
     * This function returns true if the GrXferProcessor generated from this factory will be able to
     * correctly blend when using RGB coverage. The knownColor and knownColorFlags represent the
     * final computed color from the color stages.
     */
    virtual bool supportsRGBCoverage(GrColor knownColor, uint32_t knownColorFlags) const = 0;

    bool isEqual(const GrXPFactory& that) const {
        if (this->classID() != that.classID()) {
            return false;
        }
        return this->onIsEqual(that);
    }

    /**
      * Helper for down-casting to a GrXPFactory subclass
      */
    template <typename T> const T& cast() const { return *static_cast<const T*>(this); }

    uint32_t classID() const { SkASSERT(kIllegalXPFClassID != fClassID); return fClassID; }

protected:
    GrXPFactory() : fClassID(kIllegalXPFClassID) {}

    template <typename XPF_SUBCLASS> void initClassID() {
         static uint32_t kClassID = GenClassID();
         fClassID = kClassID;
    }

    uint32_t fClassID;

private:
    virtual bool onIsEqual(const GrXPFactory&) const = 0;

    static uint32_t GenClassID() {
        // fCurrXPFactoryID has been initialized to kIllegalXPFactoryID. The
        // atomic inc returns the old value not the incremented value. So we add
        // 1 to the returned value.
        uint32_t id = static_cast<uint32_t>(sk_atomic_inc(&gCurrXPFClassID)) + 1;
        if (!id) {
            SkFAIL("This should never wrap as it should only be called once for each GrXPFactory "
                   "subclass.");
        }
        return id;
    }

    enum {
        kIllegalXPFClassID = 0,
    };
    static int32_t gCurrXPFClassID;

    typedef GrProgramElement INHERITED;
};

#endif

