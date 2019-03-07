//// Copyright 2019 Google LLC.
//// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
//#include "fiddle/examples.h"
//REG_FIDDLE(Canvas_007, 256, 256, true, 0) {
//// HASH=1598396056045e8d0c583b748293d652
//void draw(SkCanvas* ) {
//    const char* kHelloMetaData = "HelloMetaData";
//    SkCanvas canvas;
//    SkMetaData& metaData = canvas.getMetaData();
//    SkDebugf("before: %s\n", metaData.findString(kHelloMetaData));
//    metaData.setString(kHelloMetaData, "Hello!");
//    SkDebugf("during: %s\n", metaData.findString(kHelloMetaData));
//    metaData.removeString(kHelloMetaData);
//    SkDebugf("after: %s\n", metaData.findString(kHelloMetaData));
//}
//}
