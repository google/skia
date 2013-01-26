
/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */



#ifndef GrNoncopyable_DEFINED
#define GrNoncopyable_DEFINED

#include "GrTypes.h"

/**
 *  Base for classes that want to disallow copying themselves. It makes its
 *  copy-constructor and assignment operators private (and unimplemented).
 */
class GR_API GrNoncopyable {
public:
    GrNoncopyable() {}

private:
    // illegal
    GrNoncopyable(const GrNoncopyable&);
    GrNoncopyable& operator=(const GrNoncopyable&);
};

#endif
