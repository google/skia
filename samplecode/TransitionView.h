/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SAMPLECODE_TRANSITIONVIEW_H_
#define SAMPLECODE_TRANSITIONVIEW_H_

class SkView;

SkView* create_transition(SkView* prev, SkView* next, int direction);

bool is_transition(SkView* view);

#endif  // SAMPLECODE_TRANSITIONVIEW_H_
