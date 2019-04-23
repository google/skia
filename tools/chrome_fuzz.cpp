// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/core/SkCanvas.h"
#include "SkFlattenableSerialization.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkString.h"
#include "src/core/SkOSFile.h"

#include <stdio.h>

static const int kBitmapSize = 24;

static bool read_test_case(const char* filename, SkString* testdata) {
  FILE* file = sk_fopen(filename, kRead_SkFILE_Flag);
  if (!file) {
    SkDebugf("couldn't open file %s\n", filename);
    return false;
  }
  size_t len = sk_fgetsize(file);
  if (!len) {
    SkDebugf("couldn't read file %s\n", filename);
    return false;
  }
  testdata->resize(len);
  (void) fread(testdata->writable_str(), len, file);
  return true;
}

static void run_test_case(const SkString& testdata, const SkBitmap& bitmap,
                          SkCanvas* canvas) {
  // This call shouldn't crash or cause ASAN to flag any memory issues
  // If nothing bad happens within this call, everything is fine
  sk_sp<SkImageFilter> flattenable = SkValidatingDeserializeImageFilter(testdata.c_str(),
                                                                        testdata.size());

  // Adding some info, but the test passed if we got here without any trouble
  if (flattenable != nullptr) {
    SkDebugf("Valid stream detected.\n");
    // Let's see if using the filters can cause any trouble...
    SkPaint paint;
    paint.setImageFilter(flattenable);
    canvas->save();
    canvas->clipRect(SkRect::MakeXYWH(
        0, 0, SkIntToScalar(kBitmapSize), SkIntToScalar(kBitmapSize)));

    // This call shouldn't crash or cause ASAN to flag any memory issues
    // If nothing bad happens within this call, everything is fine
    canvas->drawBitmap(bitmap, 0, 0, &paint);

    SkDebugf("Filter DAG rendered successfully.\n");
    canvas->restore();
  } else {
    SkDebugf("Invalid stream detected.\n");
  }
}

static bool read_and_run_test_case(const char* filename, const SkBitmap& bitmap,
                        SkCanvas* canvas) {
  SkString testdata;
  SkDebugf("Test case: %s\n", filename);
  // read_test_case will print a useful error message if it fails.
  if (!read_test_case(filename, &testdata))
    return false;
  run_test_case(testdata, bitmap, canvas);
  return true;
}

int main(int argc, char** argv) {
  int ret = 0;
  SkBitmap bitmap;
  bitmap.allocN32Pixels(kBitmapSize, kBitmapSize);
  SkCanvas canvas(bitmap);
  canvas.clear(0x00000000);
  for (int i = 1; i < argc; i++)
    if (!read_and_run_test_case(argv[i], bitmap, &canvas))
      ret = 2;
  // Cluster-Fuzz likes "#EOF" as the last line of output to help distinguish
  // successful runs from crashes.
  SkDebugf("#EOF\n");
  return ret;
}
