/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRunnable_DEFINED
#define SkRunnable_DEFINED

struct SkRunnable {
    virtual ~SkRunnable() {};
    virtual void run() = 0;
};

#endif
