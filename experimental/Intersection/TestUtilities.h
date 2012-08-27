/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "DataTypes.h"

bool controls_inside(const Cubic& );
void find_tight_bounds(const Cubic& , _Rect& );
void quad_to_cubic(const Quadratic& , Cubic& );
