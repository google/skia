/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBBHFactory_DEFINED
#define SkBBHFactory_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkTypes.h"

class SkBBoxHierarchy : public SkRefCnt {
public:
    SkBBoxHierarchy() {}
    virtual ~SkBBoxHierarchy() {}

    // Future public APIs may go here.
};

class SK_API SkBBHFactory {
public:
    /**
     *  Allocate a new SkBBoxHierarchy. Return NULL on failure.
     */
    virtual sk_sp<SkBBoxHierarchy> operator()() const = 0;
    virtual ~SkBBHFactory() {}
};

class SK_API SkRTreeFactory : public SkBBHFactory {
public:
    sk_sp<SkBBoxHierarchy> operator()() const override;
};

#endif
