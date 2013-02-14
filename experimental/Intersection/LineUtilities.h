/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "DataTypes.h"

bool implicitLine(const _Line& line, double& slope, double& axisIntercept);
int reduceOrder(const _Line& line, _Line& reduced);
double is_left(const _Line& line, const _Point& pt);
void sub_divide(const _Line& src, double t1, double t2, _Line& dst);
double t_at(const _Line&, const _Point& );
void xy_at_t(const _Line& , double t, double& x, double& y);
_Point xy_at_t(const _Line& , double t);

enum x_at_flags {
    kFindTopMin = 1,
    kFindTopMax = 2,
    kFindBottomMin = 4,
    kFindBottomMax = 8
};

void x_at(const _Point& p1, const _Point& p2, double minY, double maxY,
        int flags, double& tMin, double& tMax);
