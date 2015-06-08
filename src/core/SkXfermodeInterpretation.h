/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkXfermodeInterpretation_DEFINED
#define SkXfermodeInterpretation_DEFINED

class SkPaint;

/**
 *  By analyzing the paint, we may decide we can take special
 *  action. This enum lists our possible actions.
 */
enum SkXfermodeInterpretation {
    kNormal_SkXfermodeInterpretation,      //< draw normally
    kSrcOver_SkXfermodeInterpretation,     //< draw as if in srcover mode
    kSkipDrawing_SkXfermodeInterpretation  //< draw nothing
};

/**
 *  Given a paint, determine whether the paint's transfer mode can be
 *  replaced with kSrcOver_Mode or not drawn at all.  This is used by
 *  SkBlitter and SkPDFDevice.
 */
SkXfermodeInterpretation SkInterpretXfermode(const SkPaint&, bool dstIsOpaque);

#endif  // SkXfermodeInterpretation_DEFINED
