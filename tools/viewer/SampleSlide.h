/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#ifndef SampleSlide_DEFINED
#define SampleSlide_DEFINED

#include "tools/viewer/Slide.h"

class Sample;

sk_sp<Slide> MakeSampleSlide(Sample* (*factory)());

#endif
