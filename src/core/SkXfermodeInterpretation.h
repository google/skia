/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermodeInterpretation_DEFINED
#define SkXfermodeInterpretation_DEFINED

class SkPaint;

/** By analyzing the paint, we may decide we can take special
    action. This enum lists our possible actions. */
enum SkXfermodeInterpretation {
    kNormal_SkXfermodeInterpretation,      // draw normally
    kSrcOver_SkXfermodeInterpretation,     // draw as if in srcover mode
    kSkipDrawing_SkXfermodeInterpretation  // draw nothing
};
SkXfermodeInterpretation SkInterpretXfermode(const SkPaint&, bool dstIsOpaque);

#endif  // SkXfermodeInterpretation_DEFINED
