/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkAtlasTextContext_DEFINED
#define SkAtlasTextContext_DEFINED

#include "SkRefCnt.h"

class SkAtlasTextRenderer;
class SkInternalAtlasTextContext;

SkAtlasTextRenderer* SkGetAtlasTextRendererFromInternalContext(class SkInternalAtlasTextContext&);

/**
 * Class that Atlas Text client uses to register their SkAtlasTextRenderer implementation and
 * to create one or more SkAtlasTextTargets (destination surfaces for text rendering).
 */
class SK_API SkAtlasTextContext : public SkRefCnt {
public:
    static sk_sp<SkAtlasTextContext> Make(sk_sp<SkAtlasTextRenderer>);

    SkAtlasTextRenderer* renderer() const {
        return SkGetAtlasTextRendererFromInternalContext(*fInternalContext);
    }

    SkInternalAtlasTextContext& internal() { return *fInternalContext; }

private:
    SkAtlasTextContext() = delete;
    SkAtlasTextContext(const SkAtlasTextContext&) = delete;
    SkAtlasTextContext& operator=(const SkAtlasTextContext&) = delete;

    SkAtlasTextContext(sk_sp<SkAtlasTextRenderer>);

    std::unique_ptr<SkInternalAtlasTextContext> fInternalContext;
};

#endif
