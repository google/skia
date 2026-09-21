/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkContextPriv_DEFINED
#define SkContextPriv_DEFINED

#include "include/core/SkContext.h"
#include "include/core/SkRefCnt.h"
#include "src/core/SkSharedContext.h"

#include <memory>

class SkContext;

/** Class that adds const methods to SkContext that are only intended for use internal to Skia.
    This class is purely a privileged window into SkContext. It should never have additional
    data members or virtual methods.*/
class SkContextPrivConst {
public:

protected:
    explicit SkContextPrivConst(const SkContext* context) : fConstContext(context) {}

    size_t fontCacheLimit() const { return fConstContext->fontCacheLimit(); }

private:
    // No taking addresses of this type.
    const SkContextPrivConst* operator&() const = delete;
    SkContextPrivConst& operator=(const SkContextPrivConst&) = delete;

    friend class SkContext;

    const SkContext* fConstContext;
};

/** Class that adds both const and mutable methods to SkContext that are only intended for use
   internal to Skia. This class is purely a privileged window into SkContext. It should never have
   additional data members or virtual methods.*/
class SkContextPriv : public SkContextPrivConst {
public:
    SkSharedContext* sharedContext() { return fContext->fSharedContext.get(); }

private:
    friend class SkContext;

    explicit SkContextPriv(SkContext* context) : SkContextPrivConst(context), fContext(context) {}

    // No taking addresses of this type.
    SkContextPriv* operator&() = delete;
    SkContextPriv& operator=(const SkContextPriv&) = delete;

    SkContext* fContext;
};

#endif  // SkContextPriv_DEFINED
