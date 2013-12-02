/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfDiffEncoder_DEFINED
#define SkPdfDiffEncoder_DEFINED

struct PdfToken;

namespace SkPdfDiffEncoder {
    /**
     * If PDF_TRACE_DIFF_IN_PNG is defined, the PDF commands so far are written
     * to a file with the difference created by using this token highlighted.
     * The file is named "/tmp/log_step_by_step/step-%i-%s.png", where %i is
     * the number of the command and %s is the name of the command. If
     * PDF_TRACE_DIFF_IN_PNG is not defined this function does nothing.
     * TODO(scroggo): Pass SkPdfContext and SkCanvas for info.
     */
    void WriteToFile(PdfToken*);
};

#endif // SkPdfDiffEncoder_DEFINED
