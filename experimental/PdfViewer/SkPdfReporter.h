/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkPdfReporter_DEFINED
#define SkPdfReporter_DEFINED

#include "SkPdfConfig.h"

class SkPdfNativeObject;
class SkPdfContext;

// TODO(edisonn): ability to turn on asserts for known good files

enum SkPdfIssueSeverity {
    kInfo_SkPdfIssueSeverity,
    kCodeWarning_SkPdfIssueSeverity, // e.g. like NYI, PDF file is Ok.
    kWarning_SkPdfIssueSeverity,
    kIgnoreError_SkPdfIssueSeverity,
    kError_SkPdfIssueSeverity,
    kFatalError_SkPdfIssueSeverity,

    _kCount__SkPdfIssueSeverity
};

enum SkPdfIssue {
    kNoIssue_SkPdfIssue,

    kNullObject_SkPdfIssue,
    kUnexpectedArraySize_SkPdfIssue,
    kMissingEncoding_SkPdfIssue,
    kNYI_SkPdfIssue,
    kIncostistentSizes_SkPdfIssue,
    kMissingRequiredKey_SkPdfIssue,
    kRecursiveReferencing_SkPdfIssue,
    kStackNestingOverflow_SkPdfIssue,
    kStackOverflow_SkPdfIssue,
    kIncositentSyntax_SkPdfIssue,
    kMissingFont_SkPdfIssue,
    kInvalidFont_SkPdfIssue,
    kMissingBT_SkPdfIssue,
    kOutOfRange_SkPdfIssue,
    kUnknownBlendMode_SkPdfIssue,
    kMissingExtGState_SkPdfIssue,
    kMissingXObject_SkPdfIssue,
    kReadStreamError_SkPdfIssue,
    kMissingToken_SkPdfIssue,
    kBadReference_SkPdfIssue,
    kNoFlateLibrary_SkPdfIssue,
    kBadStream_SkPdfIssue,

    _kCount__SkPdfIssue
};

#ifdef PDF_REPORT

void SkPdfReportIf(bool report,
                   SkPdfIssueSeverity sev, SkPdfIssue issue,
                   const char* context,
                   const SkPdfNativeObject* obj,
                   SkPdfContext* pdfContext);
void SkPdfReport(SkPdfIssueSeverity sev, SkPdfIssue issue,
                 const char* context,
                 const SkPdfNativeObject* obj,
                 SkPdfContext* pdfContext);
void SkPdfReportUnexpectedType(SkPdfIssueSeverity sev,
                               const char* context,
                               const SkPdfNativeObject* obj, int anyOfTypes,
                               SkPdfContext* pdfContext);
#define SkPdfREPORTCODE(code) code

#else  // !PDF_REPORT

#define SkPdfReportIf(a,b,c,d,e,f)
#define SkPdfReport(a,b,c,d,e)
#define SkPdfReportUnexpectedType(a,b,c,d,e)
#define SkPdfREPORTCODE(code)

#endif  // PDF_REPORT

#endif   // SkPdfReporter_DEFINED
