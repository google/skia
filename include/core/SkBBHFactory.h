/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBBHFactory_DEFINED
#define SkBBHFactory_DEFINED

#include "include/core/SkTypes.h"
class SkBBoxHierarchy;
struct SkRect;

class SK_API SkBBHFactory {
public:
    /**
     *  Allocate a new SkBBoxHierarchy. Return NULL on failure.
     */
    virtual SkBBoxHierarchy* operator()(const SkRect& bounds) const = 0;
    virtual ~SkBBHFactory() {}
};

class SK_API SkRTreeFactory : public SkBBHFactory {
public:
    SkBBoxHierarchy* operator()(const SkRect& bounds) const override;
private:
    typedef SkBBHFactory INHERITED;
};

#endif
