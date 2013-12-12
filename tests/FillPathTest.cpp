/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkRegion.h"
#include "SkPath.h"
#include "SkScan.h"
#include "SkBlitter.h"

struct FakeBlitter : public SkBlitter {
  FakeBlitter()
      : m_blitCount(0)
  {}

  virtual void blitH(int x, int y, int width) {
    m_blitCount++;
  }

  int m_blitCount;
};

// http://code.google.com/p/skia/issues/detail?id=87
// Lines which is not clipped by boundary based clipping,
// but skipped after tessellation, should be cleared by the blitter.
DEF_TEST(FillPathInverse, reporter) {
  FakeBlitter blitter;
  SkIRect clip;
  SkPath path;
  int height = 100;
  int width  = 200;
  int expected_lines = 5;
  clip.set(0, height - expected_lines, width, height);
  path.moveTo(0.0f, 0.0f);
  path.quadTo(SkIntToScalar(width/2), SkIntToScalar(height),
              SkIntToScalar(width), 0.0f);
  path.close();
  path.setFillType(SkPath::kInverseWinding_FillType);
  SkScan::FillPath(path, clip, &blitter);

  REPORTER_ASSERT(reporter, blitter.m_blitCount == expected_lines);
}
