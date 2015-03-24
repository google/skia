/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkDQuarticRoot_DEFINED
#define SkDQuarticRoot_DEFINED

int SkReducedQuarticRoots(const double t4, const double t3, const double t2, const double t1,
        const double t0, const bool oneHint, double s[4]);

int SkQuarticRootsReal(int firstCubicRoot, const double A, const double B, const double C,
        const double D, const double E, double s[4]);

#endif
