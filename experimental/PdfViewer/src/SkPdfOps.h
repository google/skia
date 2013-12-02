/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfOps_DEFINED
#define SkPdfOps_DEFINED

// Signature for all the operations available in pdf.
typedef SkPdfResult (*PdfOperatorRenderer)(SkPdfContext*, SkCanvas*, SkPdfTokenLooper*);

// FIXME (scroggo): Make a cleaner interface for this, and avoid statics and globals.
// Map of string to function pointer for all known draw operations.
extern SkTDict<PdfOperatorRenderer> gPdfOps;

#endif // SkPdfOps_DEFINED
