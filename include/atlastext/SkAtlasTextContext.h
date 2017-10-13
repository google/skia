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

class SkAtlasTextContext : public SkRefCnt {
public:
     static sk_sp<SkAtlasTextContext> Make(std::unique_ptr<SkAtlasTextRenderer> renderer) {
         return sk_make_sp<SkAtlasTextContext>(std::move(renderer));
     }

private:
    SkAtlasTextContext() = delete;
    SkAtlasTextContext(const SkAtlasTextContext&) = delete;
    SkAtlasTextContext& operator=(const SkAtlasTextContext&) = delete;

    SkAtlasTextContext(std::unique_ptr<SkAtlasTextRenderer> renderer)
            : fRenderer(std::move(renderer)) {}

    std::unique_ptr<SkAtlasTextRenderer> fRenderer;
};

#endif
