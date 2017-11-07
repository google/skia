/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkAtlasTextContext.h"
#include "SkInternalAtlasTextContext.h"
#include "SkAtlasTextRenderer.h"

sk_sp<SkAtlasTextContext> SkAtlasTextContext::Make(std::unique_ptr<SkAtlasTextRenderer> renderer) {
    return sk_sp<SkAtlasTextContext>(new SkAtlasTextContext(std::move(renderer)));
}

SkAtlasTextContext::SkAtlasTextContext(std::unique_ptr<SkAtlasTextRenderer> renderer)
        : fInternalContext(SkInternalAtlasTextContext::Make(std::move(renderer))) {}
