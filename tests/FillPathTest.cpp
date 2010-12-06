/*
 * Copyright (C) 2010 The Chromium Authors. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Test.h"
#include "SkRegion.h"
#include "SkPath.h"
#include "SkScan.h"
#include "SkBlitter.h"

namespace {

struct FakeBlitter : public SkBlitter {
  FakeBlitter()
      : m_blitCount(0)
  {}

  virtual void blitH(int x, int y, int width) {
    m_blitCount++;
  }

  int m_blitCount;
};

}

// http://code.google.com/p/skia/issues/detail?id=87
// Lines which is not clipped by boundary based clipping, 
// but skipped after tessellation, should be cleared by the blitter.
static void TestFillPathInverse(skiatest::Reporter* reporter) {
  FakeBlitter blitter;
  SkRegion clip;
  SkPath path;
  int height = 100;
  int width  = 200;
  int expected_lines = 5;
  clip.setRect(0, height - expected_lines, width, height);
  path.moveTo(0.0, 0.0);
  path.quadTo(width/2, height, width, 0.0);
  path.close();
  path.setFillType(SkPath::kInverseWinding_FillType);
  SkScan::FillPath(path, clip, &blitter);

  REPORTER_ASSERT(reporter, blitter.m_blitCount == expected_lines);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("FillPath", FillPathTestClass, TestFillPathInverse)
