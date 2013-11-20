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

// Severity of the issue, if it something interesting info, the result of an NYI feature,
// sme ignorable defect in pdf or a major issue.
enum SkPdfIssueSeverity {
    kInfo_SkPdfIssueSeverity,
    kCodeWarning_SkPdfIssueSeverity, // e.g. like NYI, PDF file is Ok.
    kWarning_SkPdfIssueSeverity,
    kIgnoreError_SkPdfIssueSeverity,
    kError_SkPdfIssueSeverity,
    kFatalError_SkPdfIssueSeverity,

    _kCount__SkPdfIssueSeverity
};

// The type of the issue.
enum SkPdfIssue {
    kNoIssue_SkPdfIssue,

    kNullObject_SkPdfIssue,
    kUnusedObject_SkPdfIssue,
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

// Calls SkPdfReport(...) if report is true.
void SkPdfReportIf(bool report,
                   SkPdfIssueSeverity sev, SkPdfIssue issue,
                   const char* context,
                   const SkPdfNativeObject* obj,
                   SkPdfContext* pdfContext);

// Reports an issue, along with information where it happened, for example obj can be used to report
// where exactly in th pdf there is a corruption
// TODO(edisonn): add ability to report the callstack
void SkPdfReport(SkPdfIssueSeverity sev, SkPdfIssue issue,
                 const char* context,
                 const SkPdfNativeObject* obj,
                 SkPdfContext* pdfContext);

// Reports that an object does not have the expected type
// TODO(edisonn): replace with SkPdfReportIfUnexpectedType() to simplify the callers?
// TODO(edisonn): pass the keyword/operator too which triggers the issue.
void SkPdfReportUnexpectedType(SkPdfIssueSeverity sev,
                               const char* context,
                               const SkPdfNativeObject* obj, int anyOfTypes,
                               SkPdfContext* pdfContext);

// Code only in builds with reporting turn on.
#define SkPdfREPORTCODE(code) code

#else  // !PDF_REPORT

#define SkPdfReportIf(a,b,c,d,e,f)
#define SkPdfReport(a,b,c,d,e)
#define SkPdfReportUnexpectedType(a,b,c,d,e)
#define SkPdfREPORTCODE(code)

#endif  // PDF_REPORT

#endif   // SkPdfReporter_DEFINED
