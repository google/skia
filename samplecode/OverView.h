/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SAMPLECODE_OVERVIEW_H_
#define SAMPLECODE_OVERVIEW_H_

class SkView;
class SkViewFactory;

SkView* create_overview(int, const SkViewFactory*[]);

bool is_overview(SkView* view);

#endif  // SAMPLECODE_OVERVIEW_H_
